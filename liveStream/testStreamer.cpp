
#include <thread>
#include <unistd.h> 
#include "wrapperLiveStream.h"

LiveQueue lqu = LiveQueueInit();

void* ReadFrameData(){
    const int BUFFERSIZE = 81920;    
    char buffer[BUFFERSIZE]={0};
    char name [100]={0};
    for (int ii=1;ii!=2768;ii++){
        snprintf ( name, 100, "/Users/kate_hung/Documents/hlsdemo/BackhandShotsAllEnglandOpenLow/frame%d.h264", ii);
        // printf("%s\n",name);
        FILE * filp = fopen(name, "rb"); 
        int bytes_read = fread(buffer, sizeof(char), BUFFERSIZE, filp);
        SaveFrameDataToQueue(lqu,buffer, bytes_read,ii-1,0);
        fclose(filp);
        usleep(30000);
    }
}

int main(int argc, char** argv){
    std::thread mThread(ReadFrameData);
	TogatherRTSPServise(lqu);
}