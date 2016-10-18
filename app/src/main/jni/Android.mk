LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := substrate
LOCAL_SRC_FILES := libsubstrate.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := substrate-dvm
LOCAL_SRC_FILES := libsubstrate-dvm.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
# 注意：此处substrate规定模块名必须以.cy结尾
LOCAL_MODULE    := test.cy 
LOCAL_SRC_FILES := test.cy.cpp
LOCAL_LDLIBS := -llog
LOCAL_ARM_MODE := arm
LOCAL_LDLIBS += -L$(LOCAL_PATH) -lsubstrate-dvm -lsubstrate
include $(BUILD_SHARED_LIBRARY)