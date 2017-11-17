#ifndef __MWCAPTURE_C__
#define __MWCAPTURE_C__

#include <stdio.h>
#include <stdlib.h>
#include "ProductVer.h"
#include "lib-mw-capture.h"
#include "mw-fourcc.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef void (*CaptureFuncPtr)(void* context);
typedef void (*ErrorFuncPtr)(int error);
typedef void (*VideoSignalChangeFuncPtr)(void* context);
typedef struct {
    HCHANNEL channel;
    HNOTIFY notify;
    struct {
        int channelIndex;
        int cx;
        int cy;
        DWORD fourcc;
        BOOL lowLatency;
    }setting;
    struct {
        MWCAP_PTR hCapture;
        MWCAP_PTR hNotify;
        MWCAP_PTR hExit;
    }event;
    struct {
        void* buffer;
        DWORD minStride;
        DWORD imageSize;
    }frame;
    CaptureFuncPtr captureCB;
    VideoSignalChangeFuncPtr videoSignalChangeCB;
    ErrorFuncPtr errorCB;
    pthread_t threadID;
}MWCUT_Context;

int  MWCUT_Init(void);
int  MWCUT_GetChannelCount();
int  MWCUT_OpenChanel(MWCUT_Context* context,
    int channelIndex,
    int cx,
    int cy,
    DWORD fourcc,
    BOOL lowLatency);
int  MWCUT_CloseChanel(MWCUT_Context* context);
int  MWCUT_StartCapture(MWCUT_Context* context);
void MWCUT_StopCapture(MWCUT_Context* context);

#ifdef __cplusplus
}
#endif
#endif