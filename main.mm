#include <iosteram>
#include "mwcut.h>"
#include <vector>

int main() 
{
    MWCUT_Init();
    const int channelCount =  MWCUT_GetChannelCount();
    const std::vector<MWCUT_Context> channels(channelCount);
    const int cx = 1920;
    const int cy = 1080;
    const DWORD dwFourCC = MWFOURCC_BGR24;
    const BOOL lowLatency = TRUE;
    for (int i = 0; i < channels.size(); i++) {
        MWCUT_OpenChanel(&channels[i]
            ,i
            ,cx
            ,cy
            ,fourcc
            ,lowLatency
        );
    }
    do {

    } while(0);
    MWCaptureExitInstance();
    return 0;
}