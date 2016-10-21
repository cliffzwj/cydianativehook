#include "substrate.h"
#include "test.cy.h"
#include <android/log.h>
#include <fcntl.h>
#include <jni.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define TAG "HOOKTEST"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

//MSConfig(MSFilterExecutable, "/system/bin/app_process")
MSConfig(MSFilterLibrary,"/system/lib/libc.so")


#define GETLR(store_lr) __asm__ __volatile__("mov %0, lr\n\t" : "=r"(store_lr))

//head prifix
#define PERSIST "persist."
#define REDIRECT "/mnt/sdcard/zapp/"

#define SLASH "/"
#define DOT "."



// exclude app
const char* ex0 = "com.cyanogenmod.filemanager";
const char* ex1 = "com.randyswallow.swallowlauncher";
const char* ex2 = "biz.bokhorst.xprivacy";
const char* ex3 = "de.robv.android.xposed.installer";
const char* ex4 = "com.weisheng.xready";
const char* ex5 = "com.cyjh.mobileanjian";
const char* ex6 = "com.saurik.substrate";
const char* ex7 = "com.n0n3m4.gltools";
/*system exclude*/
const char* ex8 = "app_process";
const char* ex9 = "/system/bin/debuggerd";
const char* ex10 = "system_server";
const char* ex11 = "zygote";
const char* ex12 = "/system/bin/mediaserver";
const char* ex13 = "/system/bin/surfaceflinger";
const char* ex14 = "/system/bin/bootanimation";
const char* ex15 = "/system/bin/keystore";
const char* ex16 = "/system/bin/vold";



//need redirect file element, file0 > file1 >.... order by  occurrence probability
const char* file0 = "proc/cpuinfo";
const char* file1 = "proc/version";
const char* file2 = "system/build.prop";
const char* file3 = "sys/block/";
const char* file4 = "sys/class/";
const char* file5 = "/cid";
const char* file6 = "/idProduct";
const char* file7 = "/idVendor";
const char* file8 = "/iManufacturer";
const char* file9 = "/iProduct";
const char* file10 = "/iSerial";
const char* file11 = "/address";
const char* file12 = "/csd";
const char* file13 = "/serial";
const char* file14 = "/name";

//need redirect system property, must use strcmp
const char* prop0 = "ro.build.id";
const char* prop1 = "ro.build.display.id";
const char* prop2 = "ro.product.name";
const char* prop3 = "ro.product.device";
const char* prop4 = "ro.product.board";
const char* prop5 = "ro.product.manufacturer";
const char* prop6 = "ro.product.brand";
const char* prop7 = "ro.product.model";
const char* prop8 = "ro.bootloader";
const char* prop9 = "gsm.version.baseband";
const char* prop10 = "ro.hardware";
const char* prop11 = "ro.serialno";
const char* prop12 = "ro.build.version.incremental";
const char* prop13 = "ro.build.version.release";
const char* prop14 = "ro.build.version.codename";
const char* prop15 = "ro.build.type";
const char* prop16 = "ro.build.tags";
const char* prop17 = "ro.build.fingerprint";
const char* prop18 = "ro.build.date.utc"; // public static final long TIME = getLong("ro.build.date.utc") * 1000;
const char* prop19 = "ro.build.user";
const char* prop20 = "ro.build.host";
//others need to fake, oders in getprop
const char* prop21 = "gsm.apn.sim.operator.numeric"; //[gsm.apn.sim.operator.numeric]: [46002]
const char* prop22 = "gsm.network.type";//[gsm.network.type]: [UMTS]
const char* prop23 = "gsm.operator.numeric";//[gsm.operator.numeric]: [46001]
const char* prop24 = "gsm.sim.operator.alpha";//[gsm.sim.operator.alpha]: [CMCC]
const char* prop25 = "gsm.sim.operator.numeric";//[gsm.sim.operator.numeric]: [46002]
const char* prop26 = "gsm.sim.state";//[gsm.sim.state]: [READY]
const char* prop27 = "net.hostname";//[net.hostname]: [android-bc036bd6f73da1e5]
const char* prop28 = "ril.model_id";//[ril.model_id]: [I9250]
const char* prop29 = "ril.serialnumber";//[ril.serialnumber]: [00000000000] [ril.serialnumber]: [RF2C202WYME]
const char* prop30 = "ril.sw_ver";//[ril.sw_ver]: [I9250XXLJ1]
const char* prop31 = "ro.baseband";//[ro.baseband]: [I9250XXLJ1]
//GPU can not fake in all system
//const char* prop32 = "ro.board.platform";//[ro.board.platform]: [omap4] [ro.board.platform]: [exynos4]  GPU infomation
const char* prop33 = "ro.boot.baseband";//[ro.boot.baseband]: [I9250XXLJ1]
const char* prop34 = "ro.boot.bootloader";//[ro.boot.bootloader]: [PRIMEMD04]
const char* prop35 = "ro.boot.serialno";//[ro.boot.serialno]: [0149AE7C07017008]
const char* prop36 = "ro.build.description";//[ro.build.description]: [yakju-user 4.3 JWR66Y 776638 release-keys]
const char* prop37 = "ro.build.product";//[ro.build.product]: [maguro]
const char* prop38 = "ro.cm.device";//[ro.cm.device]: [maguro]
const char* prop39 = "ro.cm.display.version";//[ro.cm.display.version]: [11-20141008-SNAPSHOT-M11-maguro]
const char* prop40 = "ro.cm.releasetype";//[ro.cm.releasetype]: [SNAPSHOT]
const char* prop41 = "ro.cm.version";//[ro.cm.version]: [11-20141008-SNAPSHOT-M11-maguro]
const char* prop42 = "ro.modversion"; //[ro.modversion]: [11-20141008-SNAPSHOT-M11-maguro]
const char* prop43 = "ro.rommanager.developerid"; //[ro.rommanager.developerid]: [cyanogenmod]









//return 1, contain the s
int exclude(const char *s)
{
        int i = 0;
        i = strstr(s, ex0) != NULL || strstr(s, ex1) != NULL || strstr(s, ex2) != NULL ||
            strstr(s, ex3) != NULL || strstr(s, ex4) != NULL || strstr(s, ex5) != NULL ||
            strstr(s, ex6) != NULL || strstr(s, ex7) != NULL || strstr(s, ex8) != NULL ||
            strstr(s, ex9) != NULL || strstr(s, ex10) != NULL || strstr(s, ex11) !=NULL ||
            strstr(s, ex12) != NULL || strstr(s, ex13) != NULL || strstr(s, ex14) != NULL ||
            strstr(s, ex15) != NULL || strstr(s, ex16) != NULL;
        return i;
}


//return 1 : reprsent need redirect the file
int NeedRedirect(const char *file)
{
        int i = 0;

        i =  strstr(file, file0) != NULL || strstr(file, file1) != NULL || strstr(file, file2) != NULL ||
             //sys/block/.....
             (strstr(file, file3) != NULL && (strstr(file, file5) != NULL || strstr(file, file6) != NULL ||
                                              strstr(file, file7) != NULL||strstr(file, file8) != NULL||strstr(file, file9) != NULL ||
                                              strstr(file, file10) != NULL ||strstr(file, file11) != NULL||strstr(file, file12) != NULL ||
                                              strstr(file, file13) != NULL||strstr(file, file14) != NULL)) ||
             //sys/class/.....
             (strstr(file, file4) != NULL && (strstr(file, file5) != NULL || strstr(file, file6) != NULL ||
                                              strstr(file, file7) != NULL||strstr(file, file8) != NULL||strstr(file, file9) != NULL ||
                                              strstr(file, file10) != NULL ||strstr(file, file11) != NULL||strstr(file, file12) != NULL ||
                                              strstr(file, file13) != NULL||strstr(file, file14) != NULL));

        return i;

}

//return 1, represent need fake the key name
int NeedFakeProperty(const char *name)
{
        int i = 0;

        i = strcmp(name, prop0) ==0 || strcmp(name, prop1) ==0 || strcmp(name, prop2) ==0 || strcmp(name, prop3) ==0 || strcmp(name, prop4) ==0 ||
            strcmp(name, prop5) ==0 || strcmp(name, prop6) ==0 || strcmp(name, prop7) ==0 || strcmp(name, prop8) ==0 ||
            strcmp(name, prop9) ==0 || strcmp(name, prop10) ==0 || strcmp(name, prop11) ==0 || strcmp(name, prop12) ==0 ||
            strcmp(name, prop13) ==0 || strcmp(name, prop14) ==0 || strcmp(name, prop15) ==0 || strcmp(name, prop16) ==0 ||
            strcmp(name, prop17) ==0 || strcmp(name, prop18) ==0 || strcmp(name, prop19) ==0 || strcmp(name, prop20) ==0 ||
            strcmp(name, prop21) ==0 || strcmp(name, prop22) ==0 || strcmp(name, prop23) ==0 || strcmp(name, prop24) ==0 ||
            strcmp(name, prop25) ==0 || strcmp(name, prop26) ==0 || strcmp(name, prop27) ==0 || strcmp(name, prop28) ==0 ||
            strcmp(name, prop29) ==0 || strcmp(name, prop30) ==0 || strcmp(name, prop31) ==0 || /*strcmp(name, prop32) ==0 ||*/
            strcmp(name, prop33) ==0 || strcmp(name, prop34) ==0 || strcmp(name, prop35) ==0 ||strcmp(name, prop36) ==0 ||
            strcmp(name, prop37) ==0 || strcmp(name, prop38) ==0 || strcmp(name, prop39) ==0 ||strcmp(name, prop40) ==0 ||
            strcmp(name, prop41) ==0 || strcmp(name, prop42) ==0 || strcmp(name, prop43) ==0;

        return i;
}





// public function
int ReadConfigFile(const char *path, char *str)
{
        FILE *fp = fopen(path, "r");
        if (fp == NULL)
        {
                LOGE("fopen file %s faild,%d\n", path, getpid());
                return -1;
        }

        fread(str, 1, 200, fp);
        LOGI("read file %s is %s, %d\n", path, str, getpid());
        fclose(fp);
        return 0;
}

//在原属性名前面加"persist."用来配置重定向的属性,且重启还会保留
char *AddPreFix(const char *prefix, const char *oril)
{
        char head[128] = {0};
        strcpy(head, prefix);
        strcat(head, oril);
        return head;
}

// get packagename from pid
int getProcessName(char *buffer)
{
        char path_t[256] = {0};
        pid_t pid = getpid();
        char str[15];
        sprintf(str, "%d", pid);
        memset(path_t, 0, sizeof(path_t));
        strcat(path_t, "/proc/");
        strcat(path_t, str);
        strcat(path_t, "/cmdline");
        int fd_t = open(path_t, O_RDONLY);
        if (fd_t > 0)
        {
                int read_count = read(fd_t, buffer, 1024);

                if (read_count > 0)
                {
                        int processIndex = 0;
                        for (processIndex = 0; processIndex < strlen(buffer); processIndex++)
                        {
                                if (buffer[processIndex] == ':')
                                {
                                        buffer[processIndex] = '_';
                                }
                        }
                        return 1;
                }
        }
        return 0;
}

int GetNameFormPid(int pid, char *buffer)
{
        char path_t[256] = {0};
        // pid_t pid = getpid();
        char str[15];
        sprintf(str, "%d", pid);
        memset(path_t, 0, sizeof(path_t));
        strcat(path_t, "/proc/");
        strcat(path_t, str);
        strcat(path_t, "/cmdline");
        int fd_t = open(path_t, O_RDONLY);
        if (fd_t > 0)
        {
                int read_count = read(fd_t, buffer, 1024);

                if (read_count > 0)
                {
                        int processIndex = 0;
                        for (processIndex = 0; processIndex < strlen(buffer); processIndex++)
                        {
                                if (buffer[processIndex] == ':')
                                {
                                        buffer[processIndex] = '_';
                                }
                        }
                        return 1;
                }
        }
        return 0;
}

int FindLastName(char const *path, char const *split, char *last)
{
        //char *const delim = "/";
        char *str = (char*)malloc((strlen(path)+1)*sizeof(char));
        char *result;

        strcpy(str, path);
        //printf("%s\n", str);
        char *token, *cur = str;
        while (token = strsep(&cur, split))
        {
                result = token;
        }
        strcpy(last, result);
        free(str);
        return 0;
}





//通过传进的proc路径返回pid的值
int OpenProcToPid(char const *path)
{
        char *const delim = "/";
        char str[128];

        memset(str,0, 128*sizeof(char));
        memcpy(str, path, 128*sizeof(char));

        char *token, *cur = str;
        while (token = strsep(&cur, delim))
        {
                if (atoi(token) != 0)
                {
                        // printf("%s", token);
                        //printf("%d\n", atoi(token));
                        return atoi(token);
                }
        }
        return 0;
}

// dest_string should be freeed by the user
void array_to_string(char *dest_string, char *const src_array[])
{
        int count = 0;
        int str_len = 0;

        if (src_array == NULL)
                return;
        for (count = 0; src_array[count]; count++)
        {
                str_len += strlen(src_array[count]);
        }

        for (count = 0; src_array[count]; count++)
        {
                strcat(dest_string, "#");
                strcat(dest_string, src_array[count]);
                // printf("sigal: %s\n", src_array[count]);
        }
        // printf("after :%s\n", dest_string);
}


// MSConfig(MSFilterLibrary, "libdvm.so");
//
// bool (*_dvmLoadNativeCode)(char* pathName, void* classLoader, char** detail);
//
// bool fake_dvmLoadNativeCode(char* soPath, void* classLoader, char** detail)
//{
//      LOGD("fake_dvmLoadNativeCode soPath:%s", soPath);
//      return _dvmLoadNativeCode(soPath,classLoader,detail);
//}
//
////Substrate entry point
// MSInitialize{
//    LOGD("Substrate initialized.");
//    MSImageRef image;
//    image = MSGetImageByName("/system/lib/libdvm.so"); // ����·��
//    if (image != NULL)
//    {
//      LOGD("dvm image: 0x%08X", (void*)image);
//
//        void * dvmload = MSFindSymbol(image,
//        "_Z17dvmLoadNativeCodePKcP6ObjectPPc");
//        if(dvmload == NULL)
//        {
//            LOGD("error find dvmLoadNativeCode.");
//        }
//        else
//        {
//              MSHookFunction(dvmload,(void*)&fake_dvmLoadNativeCode,(void
//        **)&_dvmLoadNativeCode);
//                      LOGD("hook dvmLoadNativeCode sucess.");
//        }
//    }
//    else{
//        LOGD("can not find libdvm.");
//    }
//}

//ָ��Ҫhook��lib
// MSConfig(MSFilterLibrary, "/system/lib/libc.so")
//
// int (* oldfopen)(const char* path, const char* mode);
//
// int newfopen(const char* path, const char* mode) {
//
////    LOGI("call my fopen!!:%d",getpid());
//      unsigned lr;
//      GETLR(lr);
//
//      if (strstr(path, "status") != NULL) {
//              LOGI("[*] Traced-fopen Call function: 0x%x\n", lr);
//              if (strstr(path, "task") != NULL) {
//                      LOGI("[*] Traced-anti-task/status");
//              } else
//                      LOGI("[*] Traced-anti-status");
//      } else if (strstr(path, "wchan") != NULL) {
//              LOGI("[*] Traced-fopen Call function: 0x%x\n", lr);
//              LOGI("[*] Traced-anti-wchan");
//      }else if (strstr(path, "hello") != NULL){
//              LOGI("[*] Traced-fopen file path : %s, pid:%d\n", path,
// getpid());
//              path = "/sdcard/fake.txt";
//              LOGI("[*] Traced-fopen file faked to path : %s, pid:%d\n", path,
// getpid());
//      }else if (strstr(path, "cpuinfo") != NULL){
//              LOGI("[*] Traced-fopen file path : %s, pid:%d\n", path,
// getpid());
//      }else if (strstr(path, "version") != NULL){
//              LOGI("[*] Traced-fopen file path : %s, pid:%d\n", path,
// getpid());
//      }else if (strstr(path, "build.prop") != NULL){
//              LOGI("[*] Traced-fopen file path : %s, pid:%d\n", path,
// getpid());
//      }else if (strstr(path, "cid") != NULL){
//              LOGI("[*] Traced-fopen file path : %s, pid:%d\n", path,
// getpid());
//      }else if (strstr(path, "idProduct") != NULL){
//              LOGI("[*] Traced-fopen file path : %s, pid:%d\n", path,
// getpid());
//      }else if (strstr(path, "idVendor") != NULL){
//              LOGI("[*] Traced-fopen file path : %s, pid:%d\n", path,
// getpid());
//      }else if (strstr(path, "iManufacturer") != NULL){
//              LOGI("[*] Traced-fopen file path : %s, pid:%d\n", path,
// getpid());
//      }else if (strstr(path, "iProduct") != NULL){
//              LOGI("[*] Traced-fopen file path : %s, pid:%d\n", path,
// getpid());
//      }else if (strstr(path, "iSerial") != NULL){
//              LOGI("[*] Traced-fopen file path : %s, pid:%d\n", path,
// getpid());
//      }else if (strstr(path, "address") != NULL){
//              LOGI("[*] Traced-fopen file path : %s, pid:%d\n", path,
// getpid());
//      }else if (strstr(path, "csd") != NULL){
//              LOGI("[*] Traced-fopen file path : %s, pid:%d\n", path,
// getpid());
//      }else if (strstr(path, "serial") != NULL){
//              LOGI("[*] Traced-fopen file path : %s, pid:%d\n", path,
// getpid());
//      }else if (strstr(path, "name") != NULL){
//              LOGI("[*] Traced-fopen file path : %s, pid:%d\n", path,
// getpid());
//      }
//      return oldfopen(path, mode);
//}





int (*old_property_get)(const char *name, char *value);
int new_property_get(const char *name, char *value)
{
        if(NeedFakeProperty(name))
        {
                char *bufferProcess = (char *)calloc(256, sizeof(char));
                int processStatus = getProcessName(bufferProcess);
                if (exclude(bufferProcess))
                {
                        free(bufferProcess);
                        return old_property_get(name, value);
                }
                LOGI("[property_get] key name:%s, pid:%s\n", name, bufferProcess);
                char lenstr[8]= {0};
                char lastpropname[32]= {0};
                char result[64] = {0};
                sprintf(lenstr, "%d", strlen(name));
                FindLastName(name, DOT, lastpropname);
                strcat(result, PERSIST);
                strcat(result, lenstr);
                strcat(result, DOT);
                strcat(result, lastpropname);
                name = result;
                LOGD("[property_get] fake newname:%s, pid:%s\n", name, bufferProcess);
                free(bufferProcess);
        }

        return old_property_get(name, value);
}

//hook fopen
FILE *(*oldfopen)(const char *path, const char *mode);
FILE *newfopen(const char *path, const char *mode)
{

        //    LOGI("call my fopen!!:%d",getpid());
        //    unsigned lr;
        //    GETLR(lr);
        if(NeedRedirect(path))
        {
                char *bufferProcess = (char *)calloc(256, sizeof(char));
                int processStatus = getProcessName(bufferProcess);

                if (exclude(bufferProcess))
                {
                        free(bufferProcess);
                        FILE *file = oldfopen(path, mode);
                        return file;
                }


                LOGI("[fffopen] file path:%s, pid:%s\n", path, bufferProcess);
                char lastname[64] = {0};
                char *newpath;
                FindLastName(path, SLASH, lastname);
                newpath = AddPreFix(REDIRECT, lastname);
                if (!access(newpath, 0))
                {
                        path = newpath;
                        LOGD("[fffopen] file faked to path:%s, pid:%s\n", path, bufferProcess);
                }

                free(bufferProcess);
        }

        FILE *file = oldfopen(path, mode);
        return file;
}

// hook open
int (*oldopen)(char *path, int acc, int permission);
int newopen(char *path, int acc, int permission)
{

        //    LOGI("call my fopen!!:%d",getpid());
        //    unsigned lr;
        //    GETLR(lr);
//        if (strstr(path, "hello") != NULL)
//        {
//                char *bufferProcess = (char *)calloc(256, sizeof(char));
//                int processStatus = getProcessName(bufferProcess);
//
//                LOGI("[open] file path:%s, pid:%s\n", path, bufferProcess);
//                path = "/sdcard/fake.txt";
//                LOGD("[open] file faked to path:%s, pid:%s\n", path, bufferProcess);
//                free(bufferProcess);
//        }

        if(NeedRedirect(path))
        {
                char *bufferProcess = (char *)calloc(256, sizeof(char));
                int processStatus = getProcessName(bufferProcess);

                if (exclude(bufferProcess))
                {
                        free(bufferProcess);
                        return oldopen(path, acc, permission);
                }


                LOGI("[open] file path:%s, pid:%s\n", path, bufferProcess);
                char lastname[64] = {0};
                char *newpath;
                FindLastName(path, SLASH, lastname);
                newpath = AddPreFix(REDIRECT, lastname);
                if (!access(newpath, 0))
                {
                        path = newpath;
                        LOGD("[open] file faked to path:%s, pid:%s\n", path, bufferProcess);
                }

                free(bufferProcess);
        }

        //if(strstr(path, "dev/graphics")!=NULL)
        //LOGD("[open]else  path:%s, pid:%s\n", path, getpid());

        return oldopen(path, acc, permission);
}

// hook execv
//http://blog.csdn.net/guoping16/article/details/6583383
//int (*old_execve)(const char *filename, char *const argv[]);
//int new_execve(const char *filename, char *const argv[])
int (*old_execve)(const char *filename, char *const argv[], char *const envp[]);
int new_execve(const char *filename, char *const argv[], char *const envp[])
{
        char *bufferProcess = (char *)calloc(256, sizeof(char));
        int processStatus = getProcessName(bufferProcess);
        if (exclude(bufferProcess))
        {
                free(bufferProcess);
                return old_execve(filename, argv, envp);
        }

        // Log message for argv
        char argv_string[1024];
        memset(argv_string, 0, 1024 * sizeof(char));
        array_to_string(argv_string, argv);

        // Log message for envp
        //char envp_string[1024];
        //memset(envp_string, 0, 1024 * sizeof(char));
        //array_to_string(envp_string, envp);
        LOGD("[execve] %s | %s , pid:%s\n", filename, argv_string, /*envp_string,*/ bufferProcess);
        free(bufferProcess);

        return old_execve(filename, argv, envp);
}

//hook system
int (*old_system)(const char * string);
int new_system(const char * string)
{

        char *bufferProcess = (char *)calloc(256, sizeof(char));
        int processStatus = getProcessName(bufferProcess);
        if (exclude(bufferProcess))
        {
                free(bufferProcess);
                return old_system(string);
        }
        LOGI("[system] %s, pid:%s\n", string, bufferProcess);
        free(bufferProcess);

        return old_system(string);
}


//hook popen
FILE *(*old_popen)(const char *command, const char *type);
FILE *new_popen(const char *command, const char *type)
{
        char *bufferProcess = (char *)calloc(256, sizeof(char));
        int processStatus = getProcessName(bufferProcess);
        if (exclude(bufferProcess))
        {
                free(bufferProcess);
                FILE* file = old_popen(command, type);
                return file;
        }
        LOGI("[popen] %s, pid:%s\n", command, bufferProcess);
        free(bufferProcess);

        FILE* file = old_popen(command, type);
        return file;
}



/*======================================================GPU========================================================*/
//hook glGetString
//const char *(*old_glGetString)(unsigned int name);
//const char *new_glGetString(unsigned int name)
//{
//
//
//        LOGI("[glGetString]name is  %d, pid:%s\n", name, getpid());
//
//        return old_glGetString(name);
//}





/*
 * Substrate entry point
 */
//hook
MSInitialize
{

        // Let the user know that the extension has been
        // extension has been registered
        // LOGI( "Substrate initialized.");
        MSImageRef image;

        image = MSGetImageByName("/system/lib/libc.so");

        if (image != NULL)
        {
                // hook system_property_get
                void *hook_property_get = MSFindSymbol(image, "__system_property_get");
                if (hook_property_get == NULL)
                {
                        LOGI("error find property_get ");
                }
                else
                {
                        MSHookFunction(hook_property_get, (void *)&new_property_get,
                                       (void **)&old_property_get);
                }

                //hook open
                void *hookopen = MSFindSymbol(image, "open");
                if (hookopen == NULL)
                {
                        LOGI("error find open ");
                }
                else
                {
                        MSHookFunction(hookopen, (void *)&newopen, (void **)&oldopen);
                }

                // hook fopen
                void *hookfopen = MSFindSymbol(image, "fopen");
                if (hookfopen == NULL)
                {
                        LOGI("error find fopen ");
                }
                else
                {
                        MSHookFunction(hookfopen, (void *)&newfopen, (void **)&oldfopen);
                }

                // hook execv
                void *hookexecve = MSFindSymbol(image, "execve");
                if (hookexecve == NULL)
                {
                        LOGI("error find execve ");
                }
                else
                {
                        MSHookFunction(hookexecve, (void *)&new_execve, (void **)&old_execve);
                }

                //hook system
                //void *hooksystem = MSFindSymbol(image, "system");
                //if (hooksystem == NULL)
                //{
                //        LOGI("error find system ");
                //}
                //else
                //{
                //        MSHookFunction(hooksystem, (void *)&new_system, (void **)&old_system);
                //}

                //hook popen
                //void *hookpopen = MSFindSymbol(image, "popen");
                //if (hookpopen == NULL)
                //{
                //        LOGI("error find popen ");
                //}
                //else
                //{
                //        MSHookFunction(hookpopen, (void *)&new_popen, (void **)&old_popen);
                //}


        }
        else {
                LOGI("ERROR FIND LIBC");
        }


        //GPU
        //MSImageRef image_gpu = MSGetImageByName("/system/lib/libGLESv2.so");
        //if (image_gpu != NULL)
        //{
        //        // hook glGetString
        //        void *hook_glGetString = MSFindSymbol(image_gpu, "glGetString");
        //        if (hook_glGetString == NULL)
        //        {
        //                LOGI("error find glGetString");
        //        }
        //        else
        //        {
        //                MSHookFunction(hook_glGetString, (void *)&new_glGetString,
        //                               (void **)&old_glGetString);
        //        }
		//
        //}
        //else{
		//
        //        LOGI("ERROR FIND GPU so");
        //}




}

// hook libreference-ril.so
// MSConfig(MSFilterLibrary, "/system/lib/libreference-ril.so")
//
// int (*old_at_send_command_numeric)(const char *command, ATResponse
// **pp_outResponse);
//
// int new_at_send_command_numeric(const char *command, ATResponse
// **pp_outResponse) {
//
////    LOGI("call my fopen!!:%d",getpid());
////    unsigned lr;
////    GETLR(lr);
//
//      LOGI("call my new_at_send_command_numeric, the command is
//%s!!:%d",command, getpid());
//
//      return old_at_send_command_numeric(command, pp_outResponse);
//}
///*
// * Substrate entry point
// */
////hook
// MSInitialize
//{
//      // Let the user know that the extension has been
//      // extension has been registered
//      LOGI( "Substrate initialized.");
//      MSImageRef image;
//
//      image = MSGetImageByName("/system/lib/libreference-ril.so");
//
//      if (image != NULL)
//      {
//              void * hookfopen=MSFindSymbol(image,"at_send_command_numeric");
//              if(hookfopen==NULL)
//              {
//                      LOGI("error find at_send_command_numeric ");
//              }
//              else {
//                      MSHookFunction(hookfopen,(void*)&new_at_send_command_numeric,(void
//**)&old_at_send_command_numeric);
//              }
//      }
//      else {
//              LOGI("ERROR FIND libreference-ril.so");
//      }
//
//}
