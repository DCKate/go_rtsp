#ifndef _WRAPPER_LIVE_STREAM_H
#define _WRAPPER_LIVE_STREAM_H

#ifdef __cplusplus
extern "C" {
#endif
typedef void* LiveQueue;
typedef struct RTSPServerInfo {
    int port;
    void* env;
    void* rtspServer;
}RTSPServerInfo;
// static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
// 			   char const* streamName, char const* inputFileName); // fwd
int Say();
LiveQueue LiveQueueInit();
void SaveFrameDataToQueue(LiveQueue que,char* data, int size,int fnum,int isI);
int ClearQueueData(LiveQueue que);

int ServeRTSPServiseInit(RTSPServerInfo *info);
void StartServeRTSPServise(RTSPServerInfo *info);
void AddNewStream(RTSPServerInfo *info, const char* streamName,const char* descriptionString,LiveQueue que);
int TogatherRTSPServise(LiveQueue que);
void SelfStart(LiveQueue lqu,int usp);
#ifdef __cplusplus
}
#endif
#endif