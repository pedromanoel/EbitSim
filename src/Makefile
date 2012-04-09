#
# OMNeT++/OMNEST Makefile for EbitSim
#
# This file was generated with the command:
#  opp_makemake -f --deep -Xlogs -I/Users/pevangelista/Work/omnetpp-workspace/boost_1_46_1 -I/Users/pevangelista/Work/omnetpp-workspace/smc_6_0_1/lib/C++ -I../../inet-20111118/src/networklayer/rsvp_te -I../../inet-20111118/src/transport/tcp -I../../inet-20111118/src/util -I../../inet-20111118/src/applications/tcpapp -I../../inet-20111118/src/world -I../../inet-20111118/src/transport/sctp -I../../inet-20111118/src/networklayer/ldp -I../../inet-20111118/src/transport/contract -I../../inet-20111118/src/networklayer/mpls -I../../inet-20111118/src/networklayer/ted -I../../inet-20111118/src/base -I../../inet-20111118/src/networklayer/common -I../../inet-20111118/src/linklayer/mfcore -I../../inet-20111118/src/transport/udp -I../../inet-20111118/src/applications/pingapp -I../../inet-20111118/src/networklayer/ipv4 -I../../inet-20111118/src/networklayer/ipv6 -I../../inet-20111118/src/networklayer/arp -I../../inet-20111118/src/networklayer/icmpv6 -I../../inet-20111118/src/linklayer/contract -I../../inet-20111118/src/util/headerserializers -I../../inet-20111118/src/networklayer/contract -L../../inet-20111118/out/$(CONFIGNAME)/src -linet -KINET_20111118_PROJ=../../inet-20111118
#

# Name of target to be created (-o option)
TARGET = EbitSim$(EXE_SUFFIX)

# User interface (uncomment one) (-u option)
USERIF_LIBS = $(ALL_ENV_LIBS) # that is, $(TKENV_LIBS) $(CMDENV_LIBS)
#USERIF_LIBS = $(CMDENV_LIBS)
#USERIF_LIBS = $(TKENV_LIBS)

# C++ include paths (with -I)
INCLUDE_PATH = \
    -I/Users/pevangelista/Work/omnetpp-workspace/boost_1_46_1 \
    -I/Users/pevangelista/Work/omnetpp-workspace/smc_6_0_1/lib/C++ \
    -I../../inet-20111118/src/networklayer/rsvp_te \
    -I../../inet-20111118/src/transport/tcp \
    -I../../inet-20111118/src/util \
    -I../../inet-20111118/src/applications/tcpapp \
    -I../../inet-20111118/src/world \
    -I../../inet-20111118/src/transport/sctp \
    -I../../inet-20111118/src/networklayer/ldp \
    -I../../inet-20111118/src/transport/contract \
    -I../../inet-20111118/src/networklayer/mpls \
    -I../../inet-20111118/src/networklayer/ted \
    -I../../inet-20111118/src/base \
    -I../../inet-20111118/src/networklayer/common \
    -I../../inet-20111118/src/linklayer/mfcore \
    -I../../inet-20111118/src/transport/udp \
    -I../../inet-20111118/src/applications/pingapp \
    -I../../inet-20111118/src/networklayer/ipv4 \
    -I../../inet-20111118/src/networklayer/ipv6 \
    -I../../inet-20111118/src/networklayer/arp \
    -I../../inet-20111118/src/networklayer/icmpv6 \
    -I../../inet-20111118/src/linklayer/contract \
    -I../../inet-20111118/src/util/headerserializers \
    -I../../inet-20111118/src/networklayer/contract \
    -I. \
    -Iclient \
    -Iclient/connection \
    -Iclient/messages \
    -Iclient/other \
    -Iclient/smc \
    -Iclient/smc/out \
    -Icontroller \
    -Inodes \
    -Itracker \
    -Itracker/connection \
    -Itracker/messages \
    -Itracker/other

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS = -L../../inet-20111118/out/$(CONFIGNAME)/src  -linet
LIBS += -Wl,-rpath,`abspath ../../inet-20111118/out/$(CONFIGNAME)/src`

# Output directory
PROJECT_OUTPUT_DIR = ../out
PROJECTRELATIVE_PATH = src
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc and .msg files
OBJS = \
    $O/client/BitTorrentClient.o \
    $O/client/Choker.o \
    $O/client/ClientController.o \
    $O/client/ClientControllerSeq.o \
    $O/client/ContentManager.o \
    $O/client/Statistics.o \
    $O/client/SwarmManager.o \
    $O/client/connection/PeerWireThread.o \
    $O/client/connection/PeerWireThreadConnection.o \
    $O/client/connection/PeerWireThreadDownload.o \
    $O/client/connection/PeerWireThreadUpload.o \
    $O/client/connection/SwarmManagerThread.o \
    $O/client/other/BitField.o \
    $O/client/other/DataRateRollingAverage.o \
    $O/client/other/NumberBitField.o \
    $O/client/other/PeerEntry.o \
    $O/client/other/RarestPieceCounter.o \
    $O/client/other/TcpIp.o \
    $O/client/smc/out/ConnectionSM_sm.o \
    $O/client/smc/out/DownloadSM_sm.o \
    $O/client/smc/out/UploadSM_sm.o \
    $O/controller/DataSimulationControl.o \
    $O/controller/SimulationController.o \
    $O/nodes/GlobalStatistics.o \
    $O/tracker/TrackerApp.o \
    $O/tracker/connection/PeerConnectionThread.o \
    $O/tracker/other/PeerInfo.o \
    $O/client/messages/Application_m.o \
    $O/client/messages/PeerWire_m.o \
    $O/client/messages/PeerWireMsgBundle_m.o \
    $O/tracker/messages/AnnounceRequestMsg_m.o \
    $O/tracker/messages/AnnounceResponseMsg_m.o

# Message files
MSGFILES = \
    client/messages/Application.msg \
    client/messages/PeerWire.msg \
    client/messages/PeerWireMsgBundle.msg \
    tracker/messages/AnnounceRequestMsg.msg \
    tracker/messages/AnnounceResponseMsg.msg

# Other makefile variables (-K)
INET_20111118_PROJ=../../inet-20111118

#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc or configuser.vc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
ifneq ("$(OMNETPP_ROOT)","")
CONFIGFILE = $(OMNETPP_ROOT)/Makefile.inc
else
CONFIGFILE = $(shell opp_configfilepath)
endif
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

# Simulation kernel and user interface libraries
OMNETPP_LIB_SUBDIR = $(OMNETPP_LIB_DIR)/$(TOOLCHAIN_NAME)
OMNETPP_LIBS = -L"$(OMNETPP_LIB_SUBDIR)" -L"$(OMNETPP_LIB_DIR)" -loppmain$D $(USERIF_LIBS) $(KERNEL_LIBS) $(SYS_LIBS)

COPTS = $(CFLAGS)  $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)

# we want to recompile everything if COPTS changes,
# so we store COPTS into $COPTS_FILE and have object
# files depend on it (except when "make depend" was called)
COPTS_FILE = $O/.last-copts
ifneq ($(MAKECMDGOALS),depend)
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
$(shell $(MKPATH) "$O" && echo "$(COPTS)" >$(COPTS_FILE))
endif
endif

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
# >>>
# <<<
#------------------------------------------------------------------------------

# Main target
all: $O/$(TARGET)
	$(LN) $O/$(TARGET) .

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile
	@$(MKPATH) $O
	$(CXX) $(LDFLAGS) -o $O/$(TARGET)  $(OBJS) $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS)

.PHONY: all clean cleanall depend msgheaders

.SUFFIXES: .cc

$O/%.o: %.cc $(COPTS_FILE)
	@$(MKPATH) $(dir $@)
	$(CXX) -c $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(MSGC) -s _m.cc $(MSGCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)

clean:
	-rm -rf $O
	-rm -f EbitSim EbitSim.exe libEbitSim.so libEbitSim.a libEbitSim.dll libEbitSim.dylib
	-rm -f ./*_m.cc ./*_m.h
	-rm -f client/*_m.cc client/*_m.h
	-rm -f client/connection/*_m.cc client/connection/*_m.h
	-rm -f client/messages/*_m.cc client/messages/*_m.h
	-rm -f client/other/*_m.cc client/other/*_m.h
	-rm -f client/smc/*_m.cc client/smc/*_m.h
	-rm -f client/smc/out/*_m.cc client/smc/out/*_m.h
	-rm -f controller/*_m.cc controller/*_m.h
	-rm -f nodes/*_m.cc nodes/*_m.h
	-rm -f tracker/*_m.cc tracker/*_m.h
	-rm -f tracker/connection/*_m.cc tracker/connection/*_m.h
	-rm -f tracker/messages/*_m.cc tracker/messages/*_m.h
	-rm -f tracker/other/*_m.cc tracker/other/*_m.h

cleanall: clean
	-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES)  ./*.cc client/*.cc client/connection/*.cc client/messages/*.cc client/other/*.cc client/smc/*.cc client/smc/out/*.cc controller/*.cc nodes/*.cc tracker/*.cc tracker/connection/*.cc tracker/messages/*.cc tracker/other/*.cc

# DO NOT DELETE THIS LINE -- make depend depends on it.
$O/client/BitTorrentClient.o: client/BitTorrentClient.cc \
	client/BitTorrentClient.h \
	client/SwarmManager.h \
	client/connection/PeerWireThread.h \
	client/messages/Application_m.h \
	client/messages/PeerWireMsgBundle_m.h \
	client/messages/PeerWire_m.h \
	client/other/BitField.h \
	client/other/DataRateRollingAverage.h \
	client/other/PeerEntry.h \
	client/other/TcpIp.h \
	client/smc/out/ConnectionSM_sm.h \
	client/smc/out/DownloadSM_sm.h \
	client/smc/out/UploadSM_sm.h \
	tracker/messages/AnnounceRequestMsg_m.h \
	tracker/messages/AnnounceResponseMsg_m.h \
	tracker/other/PeerInfo.h \
	$(INET_20111118_PROJ)/src/applications/tcpapp/TCPSrvHostApp.h \
	$(INET_20111118_PROJ)/src/base/INETDefs.h \
	$(INET_20111118_PROJ)/src/base/INotifiable.h \
	$(INET_20111118_PROJ)/src/base/ModuleAccess.h \
	$(INET_20111118_PROJ)/src/base/NotificationBoard.h \
	$(INET_20111118_PROJ)/src/base/NotifierConsts.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddress.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddressResolver.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPv6Address.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPvXAddress.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPCommand_m.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocket.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocketMap.h
$O/client/Choker.o: client/Choker.cc \
	client/BitTorrentClient.h \
	client/Choker.h \
	client/SwarmManager.h \
	client/other/BitField.h \
	client/other/DataRateRollingAverage.h \
	client/other/PeerEntry.h \
	client/other/TcpIp.h \
	tracker/messages/AnnounceRequestMsg_m.h \
	tracker/messages/AnnounceResponseMsg_m.h \
	tracker/other/PeerInfo.h \
	$(INET_20111118_PROJ)/src/applications/tcpapp/TCPSrvHostApp.h \
	$(INET_20111118_PROJ)/src/base/INETDefs.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddress.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPv6Address.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPvXAddress.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPCommand_m.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocket.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocketMap.h
$O/client/ClientController.o: client/ClientController.cc \
	client/ClientController.h \
	client/SwarmManager.h \
	client/other/BitField.h \
	controller/DataSimulationControl.h \
	tracker/TrackerApp.h \
	tracker/messages/AnnounceRequestMsg_m.h \
	tracker/messages/AnnounceResponseMsg_m.h \
	tracker/other/PeerInfo.h \
	$(INET_20111118_PROJ)/src/applications/tcpapp/TCPSrvHostApp.h \
	$(INET_20111118_PROJ)/src/base/INETDefs.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddress.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddressResolver.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPv6Address.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPvXAddress.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPCommand_m.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocket.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocketMap.h
$O/client/ClientControllerSeq.o: client/ClientControllerSeq.cc \
	client/ClientControllerSeq.h \
	client/SwarmManager.h \
	client/other/BitField.h \
	controller/DataSimulationControl.h \
	tracker/TrackerApp.h \
	tracker/messages/AnnounceRequestMsg_m.h \
	tracker/messages/AnnounceResponseMsg_m.h \
	tracker/other/PeerInfo.h \
	$(INET_20111118_PROJ)/src/applications/tcpapp/TCPSrvHostApp.h \
	$(INET_20111118_PROJ)/src/base/INETDefs.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddress.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddressResolver.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPv6Address.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPvXAddress.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPCommand_m.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocket.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocketMap.h
$O/client/ContentManager.o: client/ContentManager.cc \
	client/BitTorrentClient.h \
	client/ContentManager.h \
	client/SwarmManager.h \
	client/messages/PeerWireMsgBundle_m.h \
	client/messages/PeerWire_m.h \
	client/other/BitField.h \
	client/other/DataRateRollingAverage.h \
	client/other/PeerEntry.h \
	client/other/RarestPieceCounter.h \
	client/other/TcpIp.h \
	tracker/messages/AnnounceRequestMsg_m.h \
	tracker/messages/AnnounceResponseMsg_m.h \
	tracker/other/PeerInfo.h \
	$(INET_20111118_PROJ)/src/applications/tcpapp/TCPSrvHostApp.h \
	$(INET_20111118_PROJ)/src/base/INETDefs.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddress.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPv6Address.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPvXAddress.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPCommand_m.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocket.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocketMap.h
$O/client/Statistics.o: client/Statistics.cc \
	client/Statistics.h \
	nodes/GlobalStatistics.h
$O/client/SwarmManager.o: client/SwarmManager.cc \
	client/BitTorrentClient.h \
	client/Choker.h \
	client/ClientController.h \
	client/ContentManager.h \
	client/SwarmManager.h \
	client/connection/SwarmManagerThread.h \
	client/other/BitField.h \
	client/other/DataRateRollingAverage.h \
	client/other/PeerEntry.h \
	client/other/RarestPieceCounter.h \
	client/other/TcpIp.h \
	tracker/TrackerApp.h \
	tracker/messages/AnnounceRequestMsg_m.h \
	tracker/messages/AnnounceResponseMsg_m.h \
	tracker/other/PeerInfo.h \
	$(INET_20111118_PROJ)/src/applications/tcpapp/TCPSrvHostApp.h \
	$(INET_20111118_PROJ)/src/base/INETDefs.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddress.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddressResolver.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPv6Address.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPvXAddress.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPCommand_m.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocket.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocketMap.h
$O/client/connection/PeerWireThread.o: client/connection/PeerWireThread.cc \
	client/BitTorrentClient.h \
	client/ContentManager.h \
	client/connection/PeerWireThread.h \
	client/messages/Application_m.h \
	client/messages/PeerWireMsgBundle_m.h \
	client/messages/PeerWire_m.h \
	client/other/BitField.h \
	client/other/DataRateRollingAverage.h \
	client/other/PeerEntry.h \
	client/other/RarestPieceCounter.h \
	client/other/TcpIp.h \
	client/smc/out/ConnectionSM_sm.h \
	client/smc/out/DownloadSM_sm.h \
	client/smc/out/UploadSM_sm.h \
	$(INET_20111118_PROJ)/src/applications/tcpapp/TCPSrvHostApp.h \
	$(INET_20111118_PROJ)/src/base/INETDefs.h \
	$(INET_20111118_PROJ)/src/base/INotifiable.h \
	$(INET_20111118_PROJ)/src/base/ModuleAccess.h \
	$(INET_20111118_PROJ)/src/base/NotificationBoard.h \
	$(INET_20111118_PROJ)/src/base/NotifierConsts.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddress.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPv6Address.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPvXAddress.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPCommand_m.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocket.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocketMap.h
$O/client/connection/PeerWireThreadConnection.o: client/connection/PeerWireThreadConnection.cc \
	client/BitTorrentClient.h \
	client/Choker.h \
	client/ContentManager.h \
	client/SwarmManager.h \
	client/connection/PeerWireThread.h \
	client/messages/Application_m.h \
	client/messages/PeerWireMsgBundle_m.h \
	client/messages/PeerWire_m.h \
	client/other/BitField.h \
	client/other/DataRateRollingAverage.h \
	client/other/PeerEntry.h \
	client/other/RarestPieceCounter.h \
	client/other/TcpIp.h \
	client/smc/out/ConnectionSM_sm.h \
	client/smc/out/DownloadSM_sm.h \
	client/smc/out/UploadSM_sm.h \
	tracker/messages/AnnounceRequestMsg_m.h \
	tracker/messages/AnnounceResponseMsg_m.h \
	tracker/other/PeerInfo.h \
	$(INET_20111118_PROJ)/src/applications/tcpapp/TCPSrvHostApp.h \
	$(INET_20111118_PROJ)/src/base/INETDefs.h \
	$(INET_20111118_PROJ)/src/base/INotifiable.h \
	$(INET_20111118_PROJ)/src/base/ModuleAccess.h \
	$(INET_20111118_PROJ)/src/base/NotificationBoard.h \
	$(INET_20111118_PROJ)/src/base/NotifierConsts.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddress.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPv6Address.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPvXAddress.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPCommand_m.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocket.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocketMap.h
$O/client/connection/PeerWireThreadDownload.o: client/connection/PeerWireThreadDownload.cc \
	client/BitTorrentClient.h \
	client/ContentManager.h \
	client/connection/PeerWireThread.h \
	client/messages/Application_m.h \
	client/messages/PeerWireMsgBundle_m.h \
	client/messages/PeerWire_m.h \
	client/other/BitField.h \
	client/other/DataRateRollingAverage.h \
	client/other/PeerEntry.h \
	client/other/RarestPieceCounter.h \
	client/other/TcpIp.h \
	client/smc/out/ConnectionSM_sm.h \
	client/smc/out/DownloadSM_sm.h \
	client/smc/out/UploadSM_sm.h \
	$(INET_20111118_PROJ)/src/applications/tcpapp/TCPSrvHostApp.h \
	$(INET_20111118_PROJ)/src/base/INETDefs.h \
	$(INET_20111118_PROJ)/src/base/INotifiable.h \
	$(INET_20111118_PROJ)/src/base/ModuleAccess.h \
	$(INET_20111118_PROJ)/src/base/NotificationBoard.h \
	$(INET_20111118_PROJ)/src/base/NotifierConsts.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddress.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPv6Address.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPvXAddress.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPCommand_m.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocket.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocketMap.h
$O/client/connection/PeerWireThreadUpload.o: client/connection/PeerWireThreadUpload.cc \
	client/BitTorrentClient.h \
	client/Choker.h \
	client/ContentManager.h \
	client/connection/PeerWireThread.h \
	client/messages/Application_m.h \
	client/messages/PeerWire_m.h \
	client/other/BitField.h \
	client/other/DataRateRollingAverage.h \
	client/other/PeerEntry.h \
	client/other/RarestPieceCounter.h \
	client/other/TcpIp.h \
	client/smc/out/ConnectionSM_sm.h \
	client/smc/out/DownloadSM_sm.h \
	client/smc/out/UploadSM_sm.h \
	$(INET_20111118_PROJ)/src/applications/tcpapp/TCPSrvHostApp.h \
	$(INET_20111118_PROJ)/src/base/INETDefs.h \
	$(INET_20111118_PROJ)/src/base/INotifiable.h \
	$(INET_20111118_PROJ)/src/base/ModuleAccess.h \
	$(INET_20111118_PROJ)/src/base/NotificationBoard.h \
	$(INET_20111118_PROJ)/src/base/NotifierConsts.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddress.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPv6Address.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPvXAddress.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPCommand_m.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocket.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocketMap.h
$O/client/connection/SwarmManagerThread.o: client/connection/SwarmManagerThread.cc \
	client/BitTorrentClient.h \
	client/ClientController.h \
	client/SwarmManager.h \
	client/connection/SwarmManagerThread.h \
	client/other/BitField.h \
	client/other/DataRateRollingAverage.h \
	client/other/PeerEntry.h \
	client/other/TcpIp.h \
	tracker/TrackerApp.h \
	tracker/messages/AnnounceRequestMsg_m.h \
	tracker/messages/AnnounceResponseMsg_m.h \
	tracker/other/PeerInfo.h \
	$(INET_20111118_PROJ)/src/applications/tcpapp/TCPSrvHostApp.h \
	$(INET_20111118_PROJ)/src/base/INETDefs.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddress.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPv6Address.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPvXAddress.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPCommand_m.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocket.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocketMap.h
$O/client/messages/Application_m.o: client/messages/Application_m.cc \
	client/messages/Application_m.h
$O/client/messages/PeerWireMsgBundle_m.o: client/messages/PeerWireMsgBundle_m.cc \
	client/messages/PeerWireMsgBundle_m.h \
	client/messages/PeerWire_m.h \
	client/other/BitField.h
$O/client/messages/PeerWire_m.o: client/messages/PeerWire_m.cc \
	client/messages/PeerWire_m.h \
	client/other/BitField.h
$O/client/other/BitField.o: client/other/BitField.cc \
	client/other/BitField.h
$O/client/other/DataRateRollingAverage.o: client/other/DataRateRollingAverage.cc \
	client/other/DataRateRollingAverage.h
$O/client/other/NumberBitField.o: client/other/NumberBitField.cc \
	client/other/NumberBitField.h
$O/client/other/PeerEntry.o: client/other/PeerEntry.cc \
	client/other/DataRateRollingAverage.h \
	client/other/PeerEntry.h \
	client/other/TcpIp.h \
	$(INET_20111118_PROJ)/src/base/INETDefs.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddress.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPv6Address.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPvXAddress.h
$O/client/other/RarestPieceCounter.o: client/other/RarestPieceCounter.cc \
	client/other/BitField.h \
	client/other/RarestPieceCounter.h
$O/client/other/TcpIp.o: client/other/TcpIp.cc \
	client/other/TcpIp.h \
	$(INET_20111118_PROJ)/src/base/INETDefs.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddress.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPv6Address.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPvXAddress.h
$O/client/smc/out/ConnectionSM_sm.o: client/smc/out/ConnectionSM_sm.cc \
	client/connection/PeerWireThread.h \
	client/messages/Application_m.h \
	client/messages/PeerWire_m.h \
	client/other/BitField.h \
	client/smc/out/ConnectionSM_sm.h \
	client/smc/out/DownloadSM_sm.h \
	client/smc/out/UploadSM_sm.h \
	$(INET_20111118_PROJ)/src/applications/tcpapp/TCPSrvHostApp.h \
	$(INET_20111118_PROJ)/src/base/INETDefs.h \
	$(INET_20111118_PROJ)/src/base/INotifiable.h \
	$(INET_20111118_PROJ)/src/base/ModuleAccess.h \
	$(INET_20111118_PROJ)/src/base/NotificationBoard.h \
	$(INET_20111118_PROJ)/src/base/NotifierConsts.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddress.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPv6Address.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPvXAddress.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPCommand_m.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocket.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocketMap.h
$O/client/smc/out/DownloadSM_sm.o: client/smc/out/DownloadSM_sm.cc \
	client/connection/PeerWireThread.h \
	client/messages/Application_m.h \
	client/messages/PeerWire_m.h \
	client/other/BitField.h \
	client/smc/out/ConnectionSM_sm.h \
	client/smc/out/DownloadSM_sm.h \
	client/smc/out/UploadSM_sm.h \
	$(INET_20111118_PROJ)/src/applications/tcpapp/TCPSrvHostApp.h \
	$(INET_20111118_PROJ)/src/base/INETDefs.h \
	$(INET_20111118_PROJ)/src/base/INotifiable.h \
	$(INET_20111118_PROJ)/src/base/ModuleAccess.h \
	$(INET_20111118_PROJ)/src/base/NotificationBoard.h \
	$(INET_20111118_PROJ)/src/base/NotifierConsts.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddress.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPv6Address.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPvXAddress.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPCommand_m.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocket.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocketMap.h
$O/client/smc/out/UploadSM_sm.o: client/smc/out/UploadSM_sm.cc \
	client/connection/PeerWireThread.h \
	client/messages/Application_m.h \
	client/messages/PeerWire_m.h \
	client/other/BitField.h \
	client/smc/out/ConnectionSM_sm.h \
	client/smc/out/DownloadSM_sm.h \
	client/smc/out/UploadSM_sm.h \
	$(INET_20111118_PROJ)/src/applications/tcpapp/TCPSrvHostApp.h \
	$(INET_20111118_PROJ)/src/base/INETDefs.h \
	$(INET_20111118_PROJ)/src/base/INotifiable.h \
	$(INET_20111118_PROJ)/src/base/ModuleAccess.h \
	$(INET_20111118_PROJ)/src/base/NotificationBoard.h \
	$(INET_20111118_PROJ)/src/base/NotifierConsts.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddress.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPv6Address.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPvXAddress.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPCommand_m.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocket.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocketMap.h
$O/controller/DataSimulationControl.o: controller/DataSimulationControl.cc \
	controller/DataSimulationControl.h
$O/controller/SimulationController.o: controller/SimulationController.cc \
	controller/DataSimulationControl.h \
	controller/SimulationController.h \
	$(INET_20111118_PROJ)/src/base/INETDefs.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddress.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPv6Address.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPvXAddress.h \
	$(INET_20111118_PROJ)/src/transport/tcp/TCP.h \
	$(INET_20111118_PROJ)/src/transport/tcp/TCPConnection.h \
	$(INET_20111118_PROJ)/src/transport/tcp/TCPSegment.h \
	$(INET_20111118_PROJ)/src/transport/tcp/TCPSegment_m.h
$O/nodes/GlobalStatistics.o: nodes/GlobalStatistics.cc \
	nodes/GlobalStatistics.h
$O/tracker/TrackerApp.o: tracker/TrackerApp.cc \
	client/other/BitField.h \
	controller/DataSimulationControl.h \
	tracker/TrackerApp.h \
	tracker/connection/PeerConnectionThread.h \
	tracker/messages/AnnounceRequestMsg_m.h \
	tracker/messages/AnnounceResponseMsg_m.h \
	tracker/other/PeerInfo.h \
	$(INET_20111118_PROJ)/src/applications/tcpapp/TCPSrvHostApp.h \
	$(INET_20111118_PROJ)/src/base/INETDefs.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddress.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPv6Address.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPvXAddress.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPCommand_m.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocket.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocketMap.h
$O/tracker/connection/PeerConnectionThread.o: tracker/connection/PeerConnectionThread.cc \
	client/other/BitField.h \
	tracker/TrackerApp.h \
	tracker/connection/PeerConnectionThread.h \
	tracker/messages/AnnounceRequestMsg_m.h \
	tracker/messages/AnnounceResponseMsg_m.h \
	tracker/other/PeerInfo.h \
	$(INET_20111118_PROJ)/src/applications/tcpapp/TCPSrvHostApp.h \
	$(INET_20111118_PROJ)/src/base/INETDefs.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddress.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPv6Address.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPvXAddress.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPCommand_m.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocket.h \
	$(INET_20111118_PROJ)/src/transport/contract/TCPSocketMap.h
$O/tracker/messages/AnnounceRequestMsg_m.o: tracker/messages/AnnounceRequestMsg_m.cc \
	client/other/BitField.h \
	tracker/messages/AnnounceRequestMsg_m.h
$O/tracker/messages/AnnounceResponseMsg_m.o: tracker/messages/AnnounceResponseMsg_m.cc \
	client/other/BitField.h \
	tracker/messages/AnnounceRequestMsg_m.h \
	tracker/messages/AnnounceResponseMsg_m.h \
	tracker/other/PeerInfo.h \
	$(INET_20111118_PROJ)/src/base/INETDefs.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddress.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPv6Address.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPvXAddress.h
$O/tracker/other/PeerInfo.o: tracker/other/PeerInfo.cc \
	client/other/BitField.h \
	tracker/messages/AnnounceRequestMsg_m.h \
	tracker/other/PeerInfo.h \
	$(INET_20111118_PROJ)/src/base/INETDefs.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPAddress.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPv6Address.h \
	$(INET_20111118_PROJ)/src/networklayer/contract/IPvXAddress.h

