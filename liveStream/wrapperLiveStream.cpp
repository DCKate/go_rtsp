/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2018, Live Networks, Inc.  All rights reserved
// A test program that demonstrates how to stream - via unicast RTP
// - various kinds of file on demand, using a built-in RTSP server.
// main program
#include <thread>
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "wrapperLiveStream.h"
UsageEnvironment* env;

// To make the second and subsequent client for each stream reuse the same
// input stream as the first client (rather than playing the file from the
// start for each client), change the following "False" to "True":
Boolean reuseFirstSource = False;

// To stream *only* MPEG-1 or 2 video "I" frames
// (e.g., to reduce network bandwidth),
// change the following "False" to "True":
Boolean iFramesOnly = False;

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
			   char const* streamName, char const* inputFileName); // fwd

// LiveFrameQueue* gQueue = new LiveFrameQueue();

int Say(){
    printf("Hello Live555");
    int sum = 0; 
    for(int ii=1;ii!=10;ii++){
        sum+=ii;
    }
    return sum;
}

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
			   char const* streamName, char const* inputFileName) {
  char* url = rtspServer->rtspURL(sms);
  UsageEnvironment& env = rtspServer->envir();
  env << "\n\"" << streamName << "\" stream, from the file \""
      << inputFileName << "\"\n";
  env << "Play this stream using the URL \"" << url << "\"\n";
  delete[] url;
}

LiveQueue LiveQueueInit(){
    LiveFrameQueue* que = new LiveFrameQueue();
    return (void*) que;
}

void SaveFrameDataToQueue(LiveQueue que,char* data, int size,int fnum,int isI){
    LiveFrameQueue* lqu = static_cast<LiveFrameQueue*>(que);
    lqu->saveFrameData(data,size,isI,fnum,size);
}

int ClearQueueData(LiveQueue que){
    LiveFrameQueue* lqu = static_cast<LiveFrameQueue*>(que);
    return lqu->clearAllFrames();
}

int ServeRTSPServiseInit(RTSPServerInfo *info) {
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  UsageEnvironment *benv= BasicUsageEnvironment::createNew(*scheduler);
  info->env = (void*)benv;

  UserAuthenticationDatabase* authDB = NULL;
// #ifdef ACCESS_CONTROL
//   // To implement client access control to the RTSP server, do the following:
//   authDB = new UserAuthenticationDatabase;
//   authDB->addUserRecord("username1", "password1"); // replace these with real strings
//   // Repeat the above with each <username>, <password> that you wish to allow
//   // access to the server.
// #endif
//    std::thread mThread( ReadFrameData );
  // Create the RTSP server:
  info->rtspServer = RTSPServer::createNew(*benv, info->port, authDB);
  if (info->rtspServer == NULL) {
    *benv << "Failed to create RTSP server: " << benv->getResultMsg() << "\n";
    return 1;
  }

//   benv->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}

void StartServeRTSPServise(RTSPServerInfo *info) {
  UsageEnvironment *benv = static_cast<UsageEnvironment*>(info->env);
  benv->taskScheduler().doEventLoop(); // does not return
}

void AddNewStream(RTSPServerInfo *info, const char* streamName,const char* descriptionString,LiveQueue que){
    LiveFrameQueue* lqu = static_cast<LiveFrameQueue*>(que);
    UsageEnvironment *benv = static_cast<UsageEnvironment*>(info->env);
    RTSPServer* svr = static_cast<RTSPServer*>(info->rtspServer);
    // char const* descriptionString
    // = "Session streamed by \"testOnDemandRTSPServer\"";

  // Set up each of the possible streams that can be served by the
  // RTSP server.  Each such stream is implemented using a
  // "ServerMediaSession" object, plus one or more
  // "ServerMediaSubsession" objects for each audio/video substream.
    // char const* streamName = "ASDFGHJKL";
    // char const* inputFileName = "BackhandShotsAllEnglandOpen2018.aac";
    ServerMediaSession* sms
      = ServerMediaSession::createNew(*benv, streamName, streamName,
				      descriptionString);
    OutPacketBuffer::maxSize = 800000; // allow for some possibly large H.264 frames
    sms->addSubsession(LiveStreamServerMediaSubsession
		       ::createNew(*benv, streamName, reuseFirstSource,lqu));
    // sms->addSubsession(ADTSAudioFileServerMediaSubsession
	// 	       ::createNew(*info->env, inputFileName, reuseFirstSource));
    svr->addServerMediaSession(sms);
    announceStream(svr, sms, streamName, streamName);
  
}

int TogatherRTSPServise(LiveQueue que) {
    LiveFrameQueue* lqu = static_cast<LiveFrameQueue*>(que);
    // Begin by setting up our usage environment:
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();
    UsageEnvironment *benv= BasicUsageEnvironment::createNew(*scheduler);
    char const* descriptionString
        = "Session streamed by \"testOnDemandRTSPServer\"";
    UserAuthenticationDatabase* authDB = NULL;
    // #ifdef ACCESS_CONTROL
    //   // To implement client access control to the RTSP server, do the following:
    //   authDB = new UserAuthenticationDatabase;
    //   authDB->addUserRecord("username1", "password1"); // replace these with real strings
    //   // Repeat the above with each <username>, <password> that you wish to allow
    //   // access to the server.
    // #endif
    //    std::thread mThread( ReadFrameData );
    // Create the RTSP server:
    RTSPServer* rtspServer = RTSPServer::createNew(*benv, 8554, authDB);
    if (rtspServer == NULL) {
        *benv << "Failed to create RTSP server: " << benv->getResultMsg() << "\n";
        return 1;
    }

    ServerMediaSession* sms
      = ServerMediaSession::createNew(*benv, "QWERTYUIOP", "QWERTYUIOP",
				      descriptionString);
    OutPacketBuffer::maxSize = 800000; // allow for some possibly large H.264 frames
    sms->addSubsession(LiveStreamServerMediaSubsession
		       ::createNew(*benv, "QWERTYUIOP", reuseFirstSource,lqu));
    // sms->addSubsession(ADTSAudioFileServerMediaSubsession
	// 	       ::createNew(*info->env, inputFileName, reuseFirstSource));
    rtspServer->addServerMediaSession(sms);
    announceStream(rtspServer, sms, "QWERTYUIOP", "QWERTYUIOP");
    benv->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}

// LiveQueue lqu = LiveQueueInit();

void ReadFrameData(LiveQueue lqu,int usp){
    const int BUFFERSIZE = 81920;    
    char buffer[BUFFERSIZE]={0};
    char name [100]={0};
    for (int ii=1;ii!=2768;ii++){
        snprintf ( name, 100, "BackhandShotsAllEnglandOpenLow/frame%d.h264", ii);
        // printf("%s\n",name);
        FILE * filp = fopen(name, "rb"); 
        int bytes_read = fread(buffer, sizeof(char), BUFFERSIZE, filp);
        SaveFrameDataToQueue(lqu,buffer, bytes_read,ii-1,0);
        fclose(filp);
        usleep(usp);
    }
}

void SelfStart(LiveQueue lqu,int usp){
    ReadFrameData(lqu,usp);
    // std::thread mThread(ReadFrameData);
	// TogatherRTSPServise(lqu);
}