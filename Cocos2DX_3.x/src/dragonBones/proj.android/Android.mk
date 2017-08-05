LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := dragonbones_static

LOCAL_MODULE_FILENAME := libdragonbones

FILE_LIST := animation/Animation.cpp   
FILE_LIST +=  $(wildcard $(LOCAL_PATH)/animation/*.cpp) 
FILE_LIST +=  $(wildcard $(LOCAL_PATH)/armature/*.cpp) 
FILE_LIST += $(wildcard $(LOCAL_PATH)/cocos2dx/*.cpp)  
FILE_LIST += $(wildcard $(LOCAL_PATH)/core/*.cpp)  
FILE_LIST += $(wildcard $(LOCAL_PATH)/events/*.cpp)  
FILE_LIST += $(wildcard $(LOCAL_PATH)/factories/*.cpp)  
FILE_LIST += $(wildcard $(LOCAL_PATH)/geom/*.cpp)  
FILE_LIST += $(wildcard $(LOCAL_PATH)/model/*.cpp)  
FILE_LIST += $(wildcard $(LOCAL_PATH)/parsers/*.cpp)  
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%) 


LOCAL_C_INCLUDES :=     $(LOCAL_PATH)/ \
                        $(LOCAL_PATH)/animation \
                        $(LOCAL_PATH)/armature \
                        $(LOCAL_PATH)/core \
                        $(LOCAL_PATH)/events \
                        $(LOCAL_PATH)/factories \
                        $(LOCAL_PATH)/geom \
                        $(LOCAL_PATH)/model \
                        $(LOCAL_PATH)/parsers \
                        $(LOCAL_PATH)/cocos2dx \
                        $(LOCAL_PATH)/textures \
                        $(LOCAL_PATH)/dragonBones \


LOCAL_EXPORT_C_INCLUDES :=     $(LOCAL_PATH)/ \
                        $(LOCAL_PATH)/animation \
                        $(LOCAL_PATH)/armature \
                        $(LOCAL_PATH)/core \
                        $(LOCAL_PATH)/events \
                        $(LOCAL_PATH)/factories \
                        $(LOCAL_PATH)/geom \
                        $(LOCAL_PATH)/model \
                        $(LOCAL_PATH)/parsers \
                        $(LOCAL_PATH)/cocos2dx \
                        $(LOCAL_PATH)/textures \


LOCAL_STATIC_LIBRARIES := cocos2dx_internal_static

include $(BUILD_STATIC_LIBRARY)
