include ../Makefile.param


###########AMP SRCS & INC ############
AMP_CLIENT_SRCS := $(wildcard $(PWD)/$(ARM_ARCH)/client/*.c)
AMP_CLIENT_SRCS += $(wildcard $(PWD)/*.c)
AMP_SERVER_SRCS := $(wildcard $(PWD)/$(ARM_ARCH)/server/*.c)
AMP_INC += -I$(PWD)/$(ARM_ARCH)/include

###########SMP SRCS & INC ############
SMP_SRCS := $(wildcard $(PWD)/$(ARM_ARCH)/*.c)
SMP_SRCS += $(wildcard $(PWD)/*.c)
SMP_SRCS += $(wildcard $(PWD)/$(ARM_ARCH)/*.cpp)


SMP_INC := -I$(PWD)/
SMP_INC += -I/home/ren/DV300/opencv-4.2.0/hisi-install/include/opencv4

C_SRCS := $(filter %.c, $(AMP_CLIENT_SRCS) $(AMP_SERVER_SRCS) $(SMP_SRCS))

CPP_SRCS := $(filter %.cpp, $(SMP_SRCS))



# 生成目标文件
C_OBJS := $(sort $(C_SRCS:.c=.o))

CPP_OBJS := $(CPP_SRCS:.cpp=.o)

# 编译选项
CFLAGS += $(SMP_INC) $(AMP_INC)
CXXFLAGS += $(SMP_INC) $(AMP_INC) -std=c++11

#LDFLAGS := -L/home/ren/DV300/opencv-4.2.0/hisi-install/lib -L/home/ren/DV300/opencv-4.2.0/lib
#LDFLAGS += -L/home/ren/DV300/Hi3516CV500_SDK_V2.0.2.0/smp/a7_linux/mpp/lib -Wl,-rpath-link=/home/ren/DV300/opencv-4.2.0/lib:/home/ren/DV300/opencv-4.2.0/hisi-install/lib
#LDFLAGS += -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs -lopencv_videoio
#LDFLAGS += -L/opt/hisi-linux/x86-arm/arm-himix200-linux/arm-himix200-linux/lib/a7
#LDFLAGS += -static-libstdc++
#LIBS += -lopencv_core -Slopencv_imgproc -lopencv_highgui -lopencv_imgcodecs

LDFLAGS := -L/home/ren/DV300/opencv-4.2.0/hisi-install/lib -L/home/ren/DV300/opencv-4.2.0/lib
#LDFLAGS += -L/opt/hisi-linux/x86-arm/arm-himix200-linux/arm-himix200-linux/lib/a7
LDFLAGS += -L/home/ren/DV300/Hi3516CV500_SDK_V2.0.2.0/smp/a7_linux/mpp/lib -Wl,-rpath-link=/home/ren/DV300/opencv-4.2.0/lib:/home/ren/DV300/opencv-4.2.0/hisi-install/lib
LDFLAGS += -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs -lopencv_videoio

#LDFLAGS += -static-libstdc++

ifeq ($(ARM_ARCH), amp)

ifeq ($(OSTYPE), liteos)
TARGET := sample_vio_server
else
TARGET := sample_vio
endif

else
TARGET := sample_vio
endif

TARGET_PATH := $(PWD)/$(ARM_ARCH)




include $(PWD)/../$(ARM_ARCH)_$(OSTYPE).mak