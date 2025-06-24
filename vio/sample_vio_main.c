#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "hi_common.h"
#include "sample_vio.h"
#include "mpi_sys.h"

//#include "serial.h"



/******************************************************************************
* function    : main()
* Description : main
******************************************************************************/
#ifdef __HuaweiLite__
int app_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    HI_S32 s32Ret = HI_FAILURE;
    HI_U32 u32VoIntfType = 0;
    HI_U32  u32ChipId;


#ifdef __HuaweiLite__
#else
    signal(SIGINT, SAMPLE_VIO_HandleSig);
    signal(SIGTERM, SAMPLE_VIO_HandleSig);
#endif

    HI_MPI_SYS_GetChipId(&u32ChipId);

    if (HI3516C_V500 == u32ChipId)
    {
        u32VoIntfType = 1;
    }
    else
    {
        u32VoIntfType = 0;
    }

    SAMPLE_VIO_MsgInit();

 
    s32Ret = SAMPLE_VIO_ViOnlineVpssOnlineRoute(u32VoIntfType);


    if (HI_SUCCESS == s32Ret)
    {
        SAMPLE_PRT("sample_vio exit success!\n");
    }
    else
    {
        SAMPLE_PRT("sample_vio exit abnormally!\n");
    }

    SAMPLE_VIO_MsgExit();

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

// void main()
// {
//     saveContourImage(const char* referPath, const char* inputPath, const char* outputPath,const char* outputPath_refer);
// }