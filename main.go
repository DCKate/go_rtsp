package main

/*
#cgo CXXFLAGS: -I./liveStream
#cgo CXXFLAGS: -I./liveStream/include/BasicUsageEnvironment -I./liveStream/include/groupsock
#cgo CXXFLAGS: -I./liveStream/include/liveMedia -I./liveStream/include/UsageEnvironment
#cgo LDFLAGS: -lstdc++  -L./liveStream -lliveStream
#include <stdlib.h>
#include "liveStream/wrapperLiveStream.h"
*/
import "C"
import (
	"fmt"
	"io/ioutil"
	"log"
	"time"
	"unsafe"
)

func ReadFrames(q C.LiveQueue) int {
	cc := 0
	for ii := 1; ii != 2768; ii++ {
		fname := fmt.Sprintf("BackhandShotsAllEnglandOpenLow/frame%d.h264", ii)
		dat, err := ioutil.ReadFile(fname)
		if err != nil {
			log.Println(err)
			continue
		}
		size := len(dat)
		C.SaveFrameDataToQueue(q, (*C.char)(unsafe.Pointer(&dat[0])), (C.int)(size), (C.int)(ii), 0)
		time.Sleep(time.Millisecond)
	}
	return cc
}

func main() {

	var SvrInfo = C.RTSPServerInfo{
		port: 8554,
	}
	log.Println(C.ServeRTSPServiseInit(&SvrInfo))
	lqu := C.LiveQueueInit()
	go ReadFrames(lqu)
	// lqu := C.LiveQueueInit()
	// go C.SelfStart(lqu, 10000)
	// C.TogatherRTSPServise(lqu)
	sname := C.CString("QWERTYUIOP")
	defer C.free(unsafe.Pointer(sname))
	desc := C.CString("GOOD DAY")
	defer C.free(unsafe.Pointer(desc))
	C.AddNewStream(&SvrInfo, sname, desc, lqu)
	C.StartServeRTSPServise(&SvrInfo)
}
