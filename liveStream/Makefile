INCLUDES = -I./include/BasicUsageEnvironment -I./include/groupsock 
INCLUDES += -I./Include/liveMedia -I./include/UsageEnvironment

##### Change the following for your environment:
COMPILE_OPTS =		$(INCLUDES) -I. $(EXTRA_LDFLAGS)
C =			c
C_COMPILER =		cc
C_FLAGS =		$(COMPILE_OPTS)
CPP =			cpp
CPLUSPLUS_COMPILER =	c++
CPLUSPLUS_FLAGS =	$(COMPILE_OPTS) -Wall
OBJ =			o
LINK =			c++ -o 
LINK_OPTS =		-L.
CONSOLE_LINK_OPTS =	$(LINK_OPTS)
LIBRARY_LINK =		libtool -s -o 
LIBRARY_LINK_OPTS =	
LIB_SUFFIX =			a
LIBS_FOR_CONSOLE_APPLICATION =
LIBS_FOR_GUI_APPLICATION =
EXE =
##### End of variables to change

NAME = libliveStream
LIVESTREAM_LIB = $(NAME).$(LIB_SUFFIX)
ALL = $(LIVESTREAM_LIB)
all:	$(ALL)

extra:	testStreamer$(EXE)

.$(CPP).$(OBJ):
	$(CPLUSPLUS_COMPILER) -c $(CPLUSPLUS_FLAGS) $<

WRAPPER_SOURCE_OBJS = wrapperLiveStream.$(OBJ) 
WRAPPER_EXTRA_OBJS = wrapperLiveStream.$(OBJ) testStreamer.$(OBJ)

LIVESTREAM_LIB_OBJS = $(WRAPPER_SOURCE_OBJS)
LIBS =			$(USAGE_ENVIRONMENT_LIB) $(BASIC_USAGE_ENVIRONMENT_LIB) $(LIVEMEDIA_LIB) $(GROUPSOCK_LIB)

Lib_DIR = lib/
USAGE_ENVIRONMENT_LIB = $(Lib_DIR)/libUsageEnvironment.$(LIB_SUFFIX)
BASIC_USAGE_ENVIRONMENT_LIB = $(Lib_DIR)/libBasicUsageEnvironment.$(LIB_SUFFIX)
LIVEMEDIA_LIB = $(Lib_DIR)/libliveMedia.$(LIB_SUFFIX)
GROUPSOCK_LIB = $(Lib_DIR)/libgroupsock.$(LIB_SUFFIX)

$(LIVESTREAM_LIB): $(LIVESTREAM_LIB_OBJS) \
    $(PLATFORM_SPECIFIC_LIB_OBJS)
	$(LIBRARY_LINK)$@ $(LIBRARY_LINK_OPTS) \
		$(LIVESTREAM_LIB_OBJS) \
		$(USAGE_ENVIRONMENT_LIB) \
		$(BASIC_USAGE_ENVIRONMENT_LIB) \
		$(LIVEMEDIA_LIB) \
		$(GROUPSOCK_LIB)



wrapperLiveStream.$(CPP):		wrapperLiveStream.h
testStreamer.$(CPP): wrapperLiveStream.h

testStreamer$(EXE):	$(WRAPPER_EXTRA_OBJS) $(LIBS)
	$(LINK)$@ $(CONSOLE_LINK_OPTS) $(WRAPPER_EXTRA_OBJS) $(LIBS)

clean:
	-rm -rf *.$(OBJ) $(ALL) core *.core *~ include/*~

##### Any additional, platform-specific rules come here:
