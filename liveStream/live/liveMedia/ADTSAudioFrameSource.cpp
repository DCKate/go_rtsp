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
// A source object for AAC audio files in ADTS format
// Implementation

#include "ADTSAudioFrameSource.hh"
#include "GroupsockHelper.hh"

////////// ADTSAudioFileSource //////////

static unsigned const samplingFrequencyTable[16] = {
  96000, 88200, 64000, 48000,
  44100, 32000, 24000, 22050,
  16000, 12000, 11025, 8000,
  7350, 0, 0, 0
};

ADTSAudioFrameSource*
ADTSAudioFrameSource::createNew(UsageEnvironment& env, LiveFrameQueue *queue) {
  do {
    // Now, having opened the input file, read the fixed header of the first frame,
    // to get the audio stream's parameters:
    unsigned char fixedHeader[4]; // it's actually 3.5 bytes long
    if (queue->peepFrontFrameData((char*)&fixedHeader,4)==0){
        // env.setResultMsg("Could not read start of ADTS file");
        printf("Could not read start of ADTS file\n");
      break;
    }
    // Check the 'syncword':
    if (!(fixedHeader[0] == 0xFF && (fixedHeader[1]&0xF0) == 0xF0)) {
    //   env.setResultMsg("Bad 'syncword' at start of ADTS file");
      printf("Bad 'syncword' at start of ADTS file\n");
      break;
    }

    // Get and check the 'profile':
    // u_int8_t profile = 4;
    u_int8_t profile = (fixedHeader[2]&0xC0)>>6; // 2 bits
    if (profile == 3) {
    //   env.setResultMsg("Bad (reserved) 'profile': 3 in first frame of ADTS file");
      printf("Bad (reserved) 'profile': 3 in first frame of ADTS file\n");
      break;
    }

    // Get and check the 'sampling_frequency_index':
    // u_int8_t sampling_frequency_index = 3;
    u_int8_t sampling_frequency_index = (fixedHeader[2]&0x3C)>>2; // 4 bits
    if (samplingFrequencyTable[sampling_frequency_index] == 0) {
    //   env.setResultMsg("Bad 'sampling_frequency_index' in first frame of ADTS file");
      printf("Bad 'sampling_frequency_index' in first frame of ADTS file\n");
      break;
    }

    // Get and check the 'channel_configuration':
    // u_int8_t channel_configuration = 1;
    u_int8_t channel_configuration
      = ((fixedHeader[2]&0x01)<<2)|((fixedHeader[3]&0xC0)>>6); // 3 bits

    // If we get here, the frame header was OK.
    // Reset the fid to the beginning of the file:
    #ifdef DEBUG
    printf("ADTSAudioFrameSource::createNew profile[%d] sampling_frequency_index[%d] channel_configuration[%u]\n", 
        profile,sampling_frequency_index,channel_configuration);
    #endif
    return new ADTSAudioFrameSource(env, queue, profile,
				   sampling_frequency_index, channel_configuration);
  } while (0);

  // An error occurred:
  return NULL;
}

ADTSAudioFrameSource
::ADTSAudioFrameSource(UsageEnvironment& env, LiveFrameQueue *queue, u_int8_t profile,
		      u_int8_t samplingFrequencyIndex, u_int8_t channelConfiguration)
  : FramedSource(env),lastFrameTimeSatmp(0) {
  frameQueue = queue;
  fSamplingFrequency = samplingFrequencyTable[samplingFrequencyIndex];
  fNumChannels = channelConfiguration == 0 ? 2 : channelConfiguration;
  fuSecsPerFrame
    = (1024/*samples-per-frame*/*1000000) / fSamplingFrequency/*samples-per-second*/;

  // Construct the 'AudioSpecificConfig', and from it, the corresponding ASCII string:
  unsigned char audioSpecificConfig[2];
  u_int8_t const audioObjectType = profile + 1;
  audioSpecificConfig[0] = (audioObjectType<<3) | (samplingFrequencyIndex>>1);
  audioSpecificConfig[1] = (samplingFrequencyIndex<<7) | (channelConfiguration<<3);
  sprintf(fConfigStr, "%02X%02x", audioSpecificConfig[0], audioSpecificConfig[1]);
}

ADTSAudioFrameSource::~ADTSAudioFrameSource() {
  
}

// Note: We should change the following to use asynchronous file reading, #####
// as we now do with ByteStreamFileSource. #####
void ADTSAudioFrameSource::doGetNextFrame() {
//   int nextSize = frameQueue->peepFrameSize();
  std::shared_ptr<FrameRawData> tmp = frameQueue->popFrame();
  int index = 0;
//   if (nextSize>0){
  if (tmp!=nullptr){    
    // Begin by reading the 7-byte fixed_variable headers:
    unsigned char headers[7];
    memcpy(headers,tmp->frmRawData,sizeof headers);
    index+=sizeof headers;

    // // Extract important fields from the headers:
    Boolean protection_absent = headers[1]&0x01;
    u_int16_t frame_length
        = ((headers[3]&0x03)<<11) | (headers[4]<<3) | ((headers[5]&0xE0)>>5);
    #ifdef DEBUG
    u_int16_t syncword = (headers[0]<<4) | (headers[1]>>4);
    fprintf(stderr, "Read frame: syncword 0x%x, protection_absent %d, frame_length %d\n", syncword, protection_absent, frame_length);
    if (syncword != 0xFFF){ 
        fprintf(stderr, "WARNING: Bad syncword!\n");
    }
    #endif
    unsigned numBytesToRead
        = frame_length > sizeof headers ? frame_length - sizeof headers : 0;

    // If there's a 'crc_check' field, skip it:
    if (!protection_absent) {
        // SeekFile64(fFid, 2, SEEK_CUR);
        index+=2;
        numBytesToRead = numBytesToRead > 2 ? numBytesToRead - 2 : 0;
    }

    // Next, read the raw frame data into the buffer provided:
    if (numBytesToRead > fMaxSize) {
        fNumTruncatedBytes = numBytesToRead - fMaxSize;
        numBytesToRead = fMaxSize;
    }
    
    memcpy(fTo, tmp->frmRawData+index,numBytesToRead);
    fFrameSize = numBytesToRead;
    #ifdef DEBUG
    printf("ReadByte %d Truncate %d Audio TimeStamp %u \n",fFrameSize,fNumTruncatedBytes, tmp->timestamp);
    #endif
    // Set the 'presentation time':
    if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) {
        // This is the first frame, so use the current time:
        gettimeofday(&fPresentationTime, NULL);
        
    } else {
        // Increment by the play time of the previous frame:
        fuSecsPerFrame =(tmp->timestamp > lastFrameTimeSatmp)?(tmp->timestamp - lastFrameTimeSatmp)*1000:((1024/*samples-per-frame*/*1000000) / fSamplingFrequency);
        unsigned uSeconds = fPresentationTime.tv_usec + fuSecsPerFrame;
        fPresentationTime.tv_sec += uSeconds/1000000;
        fPresentationTime.tv_usec = uSeconds%1000000;
    }
    lastFrameTimeSatmp = tmp->timestamp;
    fDurationInMicroseconds = fuSecsPerFrame;
    FramedSource::afterGetting(this);
  }else{
    #ifdef DEBUG
    printf("No Audio frame.... \n");
    #endif
    fFrameSize=0;
    nextTask() = envir().taskScheduler().scheduleDelayedTask(fuSecsPerFrame*2,
				(TaskFunc*)FramedSource::afterGetting, this);
  }
  
//   FramedSource::afterGetting(this);
  // Switch to another task, and inform the reader that he has data:
//   nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
// 				(TaskFunc*)FramedSource::afterGetting, this);
}
