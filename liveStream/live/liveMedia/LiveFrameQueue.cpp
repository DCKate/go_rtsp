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

#include "LiveFrameQueue.hh"

////////// LiveStreamFrame //////////

FrameRawData::FrameRawData(char *pFrameData, int isI, int frmNo,int size,unsigned ts){
    frmRawData=(char *)std::malloc(size);
    memcpy(frmRawData, pFrameData, size);
    iActualSize = size;
    timestamp = ts;
    isIframe = isI;
    ifrmNo=frmNo;

}
FrameRawData::~FrameRawData(){
    std::free(frmRawData);
}

int LiveFrameQueue::peepFrameSize(){
    std::lock_guard<std::mutex> lock(queMutex);
    if(!rawFrames.empty()){
        std::shared_ptr<FrameRawData> rdata = rawFrames.front();
        return rdata->iActualSize;
    }
    return 0;
}

int LiveFrameQueue::peepFrontFrameData(char* dst,int size){
    std::lock_guard<std::mutex> lock(queMutex);
    if(!rawFrames.empty()){
        std::shared_ptr<FrameRawData> rdata = rawFrames.front();
        if (rdata->iActualSize>size){
            memcpy(dst,rdata->frmRawData,size);
            return size;
        }
    }
    return 0;
}

std::shared_ptr<FrameRawData> LiveFrameQueue::popFrame(){
    std::lock_guard<std::mutex> lock(queMutex);
    if(!rawFrames.empty()){
        std::shared_ptr<FrameRawData> rdata = rawFrames.front();
        rawFrames.pop();
        return rdata;
    }
    return nullptr;
}
bool LiveFrameQueue::saveFrameData(char *pFrameData, int isIframe, int frmNo,int size,unsigned ts){
    std::shared_ptr<FrameRawData> rdata(new FrameRawData(pFrameData,isIframe,frmNo,size,ts));
    // std::lock_guard<std::recursive_mutex> lock(queMutex);
    rawFrames.push(rdata);
    return true;
}

int LiveFrameQueue::dropFrames(unsigned rts){
    int c = 0;
    std::lock_guard<std::mutex> lock(queMutex);
    while(!rawFrames.empty()){
        std::shared_ptr<FrameRawData> rdata = rawFrames.front();
        if (rdata->timestamp >= rts){
            return c;
        }
        rawFrames.pop();
        c++;
    } 
    return c;
}

int LiveFrameQueue::clearAllFrames(){
    int c = 0;
    std::lock_guard<std::mutex> lock(queMutex);
    while(!rawFrames.empty()){
        rawFrames.pop();
        c++;
    } 
    return c;
}

int LiveFrameQueue::getPlayState(){
    return playState;
}
void LiveFrameQueue::setPlayState(int s){ //-1 init, 0 stop play, 1 playing
    if (playState!=s){
        playState = s;
    }
}