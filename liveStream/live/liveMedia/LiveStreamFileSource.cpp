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
// "liveMedia"
// Copyright (c) 1996-2018 Live Networks, Inc.  All rights reserved.
// A file source that is a plain byte stream (rather than frames)
// Implementation

#include "LiveStreamFileSource.hh"
#include "GroupsockHelper.hh"

////////// LiveStreamFileSource //////////

LiveStreamFileSource*
LiveStreamFileSource::createNew(UsageEnvironment& env, LiveFrameQueue *queue){
  LiveStreamFileSource* newSource
    = new LiveStreamFileSource(env, queue);

  return newSource;
}

LiveStreamFileSource::LiveStreamFileSource(UsageEnvironment& env, LiveFrameQueue *queue)
  :  FramedSource(env), lastFrameTimeSatmp(0),  fLastPlayTime(0), tryCount(1){
    frameQueue = queue;
}

LiveStreamFileSource::~LiveStreamFileSource() {

// #ifndef READ_FROM_FILES_SYNCHRONOUSLY
//   envir().taskScheduler().turnOffBackgroundReadHandling(fileno(fFid));
// #endif

}

void LiveStreamFileSource::doGetNextFrame() {
  if (frameQueue==NULL) {
    handleClosure();
    return;
  }
  doReadFromFile();
}

void LiveStreamFileSource::doStopGettingFrames() {
  #ifdef DEBUG
  printf("LiveStreamFileSource::doStopGettingFrames\n");
  #endif
  frameQueue->setPlayState(0);
  envir().taskScheduler().unscheduleDelayedTask(nextTask());
}

void LiveStreamFileSource::fileReadableHandler(LiveStreamFileSource* source, int /*mask*/) {
  if (!source->isCurrentlyAwaitingData()) {
    source->doStopGettingFrames(); // we're not ready for the data yet
    return;
  }
  source->doReadFromFile();
}

void LiveStreamFileSource::doReadFromFile() {
    frameQueue->setPlayState(1);
    std::shared_ptr<FrameRawData> tmp = frameQueue->popFrame();
//   int nextSize = frameQueue->peepFrameSize();
//   if (nextSize>0){
  if (tmp!=nullptr){
    tryCount = 1;
    
    char* tmpMem = new char[frameQueue->tmpSize+tmp->iActualSize];
    if(frameQueue->tmpRawData!=nullptr){
      memcpy(tmpMem,frameQueue->tmpRawData,frameQueue->tmpSize);
      delete[] frameQueue->tmpRawData;
    }
    memcpy(tmpMem+frameQueue->tmpSize, tmp->frmRawData,tmp->iActualSize);

    frameQueue->tmpRawData = tmpMem;
    frameQueue->tmpSize += tmp->iActualSize; 
    
    if(frameQueue->tmpSize > fMaxSize){
      memcpy(fTo, frameQueue->tmpRawData,fMaxSize);
      int notyet = frameQueue->tmpSize-fMaxSize;
      char* tmpMem2 = new char[notyet];
  
      memcpy(tmpMem2,frameQueue->tmpRawData+fMaxSize,notyet);
      delete[] frameQueue->tmpRawData;

      frameQueue->tmpRawData = tmpMem2;
      frameQueue->tmpSize = notyet; 
      fFrameSize= fMaxSize;
    }else{
       memcpy(fTo, frameQueue->tmpRawData,frameQueue->tmpSize);
       delete[] frameQueue->tmpRawData;
       fFrameSize= frameQueue->tmpSize;
       frameQueue->tmpRawData=nullptr;
       frameQueue->tmpSize=0;
    }
   
  // Set the 'presentation time':
    if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) {
      // This is the first frame, so use the current time:
      gettimeofday(&fPresentationTime, NULL);
    } else {
      // Increment by the play time of the previous data:
      fLastPlayTime = (tmp->timestamp > lastFrameTimeSatmp)?(tmp->timestamp - lastFrameTimeSatmp)*1000:40000;
      unsigned uSeconds	= fPresentationTime.tv_usec + fLastPlayTime;
      fPresentationTime.tv_sec += uSeconds/1000000;
      fPresentationTime.tv_usec = uSeconds%1000000;
    }
    lastFrameTimeSatmp = tmp->timestamp;
    if (tmp->isIframe){
        frameQueue->lastRead = tmp->timestamp;
    }
    // Remember the play time of this data:
    // fLastPlayTime = 40000;
    fDurationInMicroseconds = fLastPlayTime;
    #ifdef DEBUG
    printf("send doReadFromFile Max %d NOW %d Video Timestamp %u Duration[%u]\n", fMaxSize,frameQueue->tmpSize,tmp->timestamp,fDurationInMicroseconds);
    #endif
    FramedSource::afterGetting(this);
    // nextTask() = envir().taskScheduler().scheduleDelayedTask(fLastPlayTime,
	// 			(TaskFunc*)FramedSource::afterGetting, this);
  }else{
    tryCount++;
    #ifdef DEBUG
    printf("No Video frame.... \n");
    #endif
    fFrameSize=0;
    nextTask() = envir().taskScheduler().scheduleDelayedTask(fLastPlayTime*tryCount,
				(TaskFunc*)FramedSource::afterGetting, this);
    
  }
 
//   printf("Video Frame ts: %d\n",fLastPlayTime);
  // Inform the reader that he has data:
// #ifdef READ_FROM_FILES_SYNCHRONOUSLY
  // To avoid possible infinite recursion, we need to return to the event loop to do this:
//   nextTask() = envir().taskScheduler().scheduleDelayedTask(fLastPlayTime,
				// (TaskFunc*)FramedSource::afterGetting, this);
// #else
  // Because the file read was done from the event loop, we can call the
  // 'after getting' function directly, without risk of infinite recursion:
//   FramedSource::afterGetting(this);
// #endif
}
