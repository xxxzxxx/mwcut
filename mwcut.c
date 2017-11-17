#include "mwcut.h"

int MWCUT_Init(void) {
    MWCaptureInitInstance();
    MWRefreshDevice();
    BYTE nMajor;
    BYTE nMinor;
    WORD nBuild;
    MWGetVersion(&nMajor, &nMinor, &nBuild);
    printf("MWCapture SDK for Linux V%d.%d.%d\n", nMajor, nMinor, nBuild);
    return nChannelCount;
}

int MWCUT_GetChannelCount(void) {
    return MWGetChannelCount();
}

static void capture_thread_callback(void* data)
{
    MWCUT_Context *context = (MWCUT_Context *)data;
    MW_RESULT mr;
    DWORD dwRet;
    LONGLONG llStartTime = 0;
    LONGLONG llEndTime = 0;
    MWCAP_VIDEO_CAPTURE_STATUS capStatus;
    MWCAP_VIDEO_BUFFER_INFO bufferInfo;
    MWCAP_VIDEO_FRAME_INFO videoFrameInfo;
    MWCAP_PTR hEvents[] = {context->event.hExit, context->event.hNotify};
    while(true)
    {
        dwRet = MWMultiWaitEvent(hEvents, 2, 500);
        if (dwRet == 0)
        {
            mr = MWCaptureVideoFrameToVirtualAddressEx(
                context->channel, 
                MWCAP_VIDEO_FRAME_ID_NEWEST_BUFFERED, 
                context->frame.buffer, 
                context->frame.imageSize, 
                context->frame.minStride,
                FOURCC_IsRGB(context->setting.fourcc),
                NULL, 
                context->setting.fourcc, context->setting.cx, context->setting.cy,
                0, 0, 0, 0,
                0, 100, 0, 100, 0, 
                MWCAP_VIDEO_DEINTERLACE_BLEND, 
                MWCAP_VIDEO_ASPECT_RATIO_CROPPING, 
                NULL, NULL, 0, 0,
                MWCAP_VIDEO_COLOR_FORMAT_UNKNOWN, 
                MWCAP_VIDEO_QUANTIZATION_UNKNOWN,
                MWCAP_VIDEO_SATURATION_UNKNOWN
            );
            if (mr != MW_SUCCEEDED)
            {
                fprintf(stderr, "");
                break;
            }
            do
            {
                MWWaitEvent(m_hCaptureEvent, -1);
                mr = MWGetVideoCaptureStatus(m_hChannel, &capStatus);
            } while(mr == MW_SUCCEEDED && capStatus.bFrameCompleted == FALSE);
            if (context->captureCB != NULL)
            {
                context->captureCB(context->frame.buffer, m_dwImageSize, NULL);
            }
        }

        if (dwRet & 0x01)
        {
            break;
        }

        if (dwRet & (0x01 << 1))
        {
            ULONGLONG llStatus;
            mr = MWGetNotifyStatus(m_hChannel, m_hNotify, &llStatus);
            if (mr != MW_SUCCEEDED)
            {
                break;
            }
            if (llStatus & MWCAP_NOTIFY_VIDEO_FRAME_BUFFERED)
            {
                mr = MWGetVideoBufferInfo(context->channel, &bufferInfo);
                if (mr != MW_SUCCEEDED)
                {
                    break;
                }
                mr = MWGetVideoFrameInfo(context->channel, bufferInfo.iNewestBuffered,&videoFrameInfo);
                if (mr != MW_SUCCEEDED)
                {
                    break;
                }
                mr = MWCaptureVideoFrameToVirtualAddressEx(
                    context->channel, 
                    bufferInfo.iNewestBuffered, 
                    context->frame.buffer, 
                    context->frame.imageSize, 
                    context->frame.minStride,
                    FOURCC_IsRGB(context->setting.fourcc),
                    NULL, 
                    context->setting.fourcc, context->setting.cx, context->setting.cy,
                    0, 0, 0, 0, 0, 
                    100, 0, 100, 0,
                    MWCAP_VIDEO_DEINTERLACE_BLEND, MWCAP_VIDEO_ASPECT_RATIO_CROPPING, 
                    NULL, NULL, 0, 0,
                    MWCAP_VIDEO_COLOR_FORMAT_UNKNOWN, 
                    MWCAP_VIDEO_QUANTIZATION_UNKNOWN, 
                    MWCAP_VIDEO_SATURATION_UNKNOWN
                );
                if (mr != MW_SUCCEEDED)
                {
                    break;
                }
                do
                {
                    MWWaitEvent(m_hCaptureEvent, -1);
                    mr = MWGetVideoCaptureStatus(m_hChannel, &capStatus);
                } while(mr == MW_SUCCEEDED && capStatus.bFrameCompleted == FALSE);
                if (context->captureCB != NULL)
                {
                    context->captureCB(m_pbImage, m_dwImageSize, NULL);
                }
            }
            else if (llStatus & MWCAP_NOTIFY_VIDEO_FRAME_BUFFERING)
            {
                mr = MWGetVideoBufferInfo(context->channel, &bufferInfo);
                if (mr != MW_SUCCEEDED)
                {
                    break;
                }
                mr = MWGetVideoFrameInfo(context->channel, bufferInfo.iNewestBuffering,&videoFrameInfo);
                if (mr != MW_SUCCEEDED)
                {
                    break;
                }
                mr = MWCaptureVideoFrameToVirtualAddressEx(
                    context->channel, 
                    bufferInfo.iNewestBuffered, 
                    context->frame.buffer, 
                    context->frame.imageSize, 
                    context->frame.minStride,
                    FOURCC_IsRGB(context->setting.fourcc),
                    NULL, 
                    context->setting.fourcc, context->setting.cx, context->setting.cy,
                    0, 0, 0, 0, 0, 100, 0, 100, 0,
                    MWCAP_VIDEO_DEINTERLACE_BLEND, MWCAP_VIDEO_ASPECT_RATIO_CROPPING,
                    NULL, NULL, 0, 0,
                    MWCAP_VIDEO_COLOR_FORMAT_UNKNOWN, 
                    MWCAP_VIDEO_QUANTIZATION_UNKNOWN, 
                    MWCAP_VIDEO_SATURATION_UNKNOWN
                );
                if (mr != MW_SUCCEEDED)
                {
                    break;
                }
                do
                {
                    MWWaitEvent(m_hCaptureEvent, -1);
                    mr = MWGetVideoCaptureStatus(m_hChannel, &capStatus);
                } while(mr == MW_SUCCEEDED && capStatus.bFrameCompleted == FALSE);
                if (context->captureCB != NULL)
                {
                    context->captureCB(m_pbImage, m_dwImageSize, NULL);
                }
            }
            else if (llStatus & MWCAP_NOTIFY_VIDEO_SIGNAL_CHANGE)
            {
                if (context->videoSignalChangeCB != NULL)
                {
                    context->videoSignalChangeCB(context);
                }
            }
        }
    }
}
int MWCUT_StartCapture(MWCUT_Context* context)
{
    MW_RESULT mr;
    if (context->channel == -1)
    {
        return -1;
    }
    context->event.hCapture = MWCreateEvent();
    if(context->event.hCapture == 0)
    {
        fprintf(stderr, "Error : MWCreateEvent event.capture failed");
        return -2;
    }
    mr = MWStartVideoCapture(context->channel, context->event.hCapture);
    if (mr != MW_SUCCEEDED)
    {
        fprintf(stderr, "Error : MWStartVideoCapture");
        return -3;
    }
    context->event.hNotify = MWCreateEvent();
    if(context->event.hNotify == 0)
    {
        fprintf(stderr, "Error : MWCreateEvent context->event.hNotify failed");
        return -2;
    }
    context->event.hExit = MWCreateEvent();
    if(context->event.hExit == 0)
    {
        fprintf(stderr, "Error : MWCreateEvent context->event.hExit failed");
        return -2;
    }

    DWORD option = (!context->setting.lowLatency)
    ? MWCAP_NOTIFY_VIDEO_FRAME_BUFFERED | MWCAP_NOTIFY_VIDEO_SIGNAL_CHANGE
    : MWCAP_NOTIFY_VIDEO_FRAME_BUFFERING | MWCAP_NOTIFY_VIDEO_SIGNAL_CHANGE
    ;
    context->notify = MWRegisterNotify(context->channel
        , context->event.hNotify, option);
    if (context->notify == 0)
    {
        fprintf(stderr, "Error : MWRegisterNotify context->notify failed");
        return -4;
    }
    if (context->frame.buffer)
    {
        free((void *)(unsigned long)context->frame.buffer);
        context->frame.buffer = 0;
        context->frame.minStride = 0;
        context->frame.imageSize = 0;
    }
    context->frame.minStride = FOURCC_CalcMinStride(
        context->setting.fourcc,
        context->setting.cx,
        4
    );
    context->frame.imageSize = FOURCC_CalcImageSize(
        context->setting.fourcc, 
        context->setting.cx,
        context->setting.cy, 
        context->frame.minStride
    );
    context->frame.buffer = (MWCAP_PTR)malloc(context->frame.imageSize);
    if (!context->frame.buffer) {
        fprintf(stderr, "Error : context->frame.buffer malloc failed. [%d]", context->setting.imageSize);
        context->frame.buffer = 0;
        context->frame.minStride = 0;
        context->frame.imageSize = 0;
        return -4;
    }
    if (pthread_create(&context->threadID, NULL, VideoCaptureProc, context) != 0)
    {
        fprintf(stderr, "Error : pthread_create failed.");
        return -5;
    }
    return 0;
}

void MWCUT_StopCapture(MWCUT_Context* context)
{
    if (context->event.hExit == 0 || context->channel == 0)
        return;

    MWSetEvent(context->event.hExit);

    pthread_join(context->threadID, NULL);

    if(context->event.hExit != 0)
    {
        MWCloseEvent(context->event.hExit);
        context->event.hExit = 0;
    }

    if (context->hNotify != 0)
    {
        MWUnregisterNotify(context->channel, context->hNotify);
        context->hNotify = 0;
    }

    if(context->event.hNotify != 0)
    {
        MWCloseEvent(context->event.hNotify);
        context->event.hNotify = 0;
    }

    MWStopVideoCapture(context->channel);

    if(context->event.hCapture != 0)
    {
        MWCloseEvent(context->event.hCapture);
        context->event.hCapture = 0;
    }

    if (context->frame.buffer != 0)
    {
        free((void *)(unsigned long)context->frame.buffer);
        context->frame.buffer = 0;
    }
}

int MWCUT_CloseChanel(MWCUT_Context* context)
{
    MWCUT_StopCapture(context);
    if (context->channel)
    {
        MWCloseChannel(context->channel);
        context->channel = 0;
        context->setting.index = 0;
    }
}

int MWCUT_OpenChanel(MWCUT_Context* context,
    int channelIndex,
    int cx,
    int cy,
    DWORD fourcc,
    BOOL lowLatency)
{
    MW_RESULT mr;
    HCHANNEL channel;
    MWCAP_CHANNEL_INFO channelInfo;
    if (channelIndex >= MWGetChannelCount() || channelIndex < 0)
    {
        fprintf(stderr, "Error: MWCUT_OpenChanel specified channel is invalid [%d]", index);
        return -1;
    }
    if (context->channel != 0 && channelIndex == context->setting.channelIndex)
    {
        fprintf(stderr, "Error: MWCUT_OpenChanel pecified channel is already open [%d]", index);
        return -2;
    }
    if (context->channel != 0 && channelIndex != context->setting.channelIndex)
    {
        MWCUT_CloseChanel(context)
    }
    mr = MWGetChannelInfoByIndex(channelIndex, &channelInfo)
    if (MW_SUCCEEDED != mr)
    {
        fprintf(stderr, "Error: MWCUT_OpenChanel MWGetChannelInfoByIndex failed. [%d]", mr);
        return -3;
    }
    channel = MWOpenChannel(channelInfo.byBoardIndex, channelInfo.byChannelIndex);
    if (channel == -1)
    {
        fprintf(stderr, "Error: MWCUT_OpenChanel MWOpenChannel failed. [%d:%d]", 
            channelInfo.byBoardIndex, channelInfo.byChannelIndex
        );
        return -4;
    }
    context->channel = channel;
    context->setting.channelIndex = channelIndex;
    context->setting.cx = cx;
    context->setting.cy = cy;
    context->setting.fourcc = fourcc;
    context->setting.lowLatency = lowLatency;
    return 0;
}
