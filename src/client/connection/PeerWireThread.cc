// EbitSim - Enhanced BitTorrent Simulation
// This program is under the terms of the Creative Commons
// Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)
//
// You are free:
//
//    to Share - to copy, distribute and transmit the work
//    to Remix - to adapt the work
//
// Under the following conditions:
//
//    Attribution - You must attribute the work in the manner specified by the
//    author or licensor (but not in any way that suggests that they endorse you
//    or your use of the work).
//
//    Noncommercial - You may not use this work for commercial purposes.
//
//    Share Alike - If you alter, transform, or build upon this work, you may
//    distribute the resulting work only under the same or similar license to
//    this one.
//
// With the understanding that:
//
//    Waiver - Any of the above conditions can be waived if you get permission
//    from the copyright holder.
//
//    Public Domain - Where the work or any of its elements is in the public
//    domain under applicable law, that status is in no way affected by the
//    license.
//
//    Other Rights - In no way are any of the following rights affected by the
//    license:
//        - Your fair dealing or fair use rights, or other applicable copyright
//          exceptions and limitations;
//        - The author's moral rights;
//        - Rights other persons may have either in the work itself or in how
//          the work is used, such as publicity or privacy rights.
//
//    Notice - For any reuse or distribution, you must make clear to others the
//    license terms of this work. The best way to do this is with a link to this
//    web page. <http://creativecommons.org/licenses/by-nc-sa/3.0/>
//
// Author:
//     Pedro Manoel Fabiano Alves Evangelista <pevangelista@larc.usp.br>
//     Supervised by Prof Tereza Cristina M. B. Carvalho <carvalho@larc.usp.br>
//     Graduate Student at Escola Politecnica of University of Sao Paulo, Brazil
//
// Contributors:
//     Marcelo Carneiro do Amaral <mamaral@larc.usp.br>
//     Victor Souza <victor.souza@ericsson.com>
//
// Disclaimer:
//     This work is part of a Master Thesis developed by:
//        Pedro Evangelista, graduate student at
//        Laboratory of Computer Networks and Architecture
//        Escola Politecnica
//        University of Sao Paulo
//        Brazil
//     and supported by:
//        Innovation Center
//        Ericsson Telecomunicacoes S.A., Brazil.
//
// UNLESS OTHERWISE MUTUALLY AGREED TO BY THE PARTIES IN WRITING AND TO THE
// FULLEST EXTENT PERMITTED BY APPLICABLE LAW, LICENSOR OFFERS THE WORK AS-IS
// AND MAKES NO REPRESENTATIONS OR WARRANTIES OF ANY KIND CONCERNING THE WORK,
// EXPRESS, IMPLIED, STATUTORY OR OTHERWISE, INCLUDING, WITHOUT LIMITATION,
// WARRANTIES OF TITLE, MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
// NONINFRINGEMENT, OR THE ABSENCE OF LATENT OR OTHER DEFECTS, ACCURACY, OR THE
// PRESENCE OF ABSENCE OF ERRORS, WHETHER OR NOT DISCOVERABLE. SOME
// JURISDICTIONS DO NOT ALLOW THE EXCLUSION OF IMPLIED WARRANTIES, SO THIS
// EXCLUSION MAY NOT APPLY TO YOU.
//

#include "client/connection/PeerWireThread.h"

#include <cassert>
#include <cpacketqueue.h>
#include <omnetpp.h>
#include <vector>
#include <boost/lexical_cast.hpp>

#include "BitTorrentClient.h"
#include "ContentManager.h"
#include "Application_m.h"
#include "PeerWireMsgBundle_m.h"

//#define DEBUG_CONN

//Register_Class(PeerWireThread);

namespace {
std::string toStr(int i) {
    return boost::lexical_cast<std::string>(i);
}
}

// Own methods
//PeerWireThread::PeerWireThread() :
//    // initialize the state machines
//    connectionSm(*this), downloadSm(*this), uploadSm(*this),
//    // initialize the pointers to other objects
//    btClient(NULL), choker(NULL), contentManager(NULL),
//    // initialize the connection parameters
//    activeConnection(false), infoHash(-1), remotePeerId(-1), //
//    terminating(false), busy(false),
//    // initialize the cObjects with names
//    messageQueue("Awaiting messages"), //
//    postProcessingAppMsg("Post-processing messages"), //
//    downloadRateTimer("Download Rate Timer", APP_DOWNLOAD_RATE_TIMER), //
//    keepAliveTimer("KeepAlive Timer", APP_KEEP_ALIVE_TIMER), //
//    snubbedTimer("Snubbed Timer", APP_SNUBBED_TIMER), //
//    timeoutTimer("Timeout Timer", APP_TIMEOUT_TIMER), //
//    uploadRateTimer("Upload Rate Timer", APP_UPLOAD_RATE_TIMER) {
//}

PeerWireThread::PeerWireThread(int infoHash, int remotePeerId) :
    // initialize the state machines
    connectionSm(*this), downloadSm(*this), uploadSm(*this),
    // initialize the pointers to other objects
    btClient(NULL), choker(NULL), contentManager(NULL), //
    // initialize the connection parameters
    activeConnection(true), infoHash(infoHash), remotePeerId(remotePeerId), //
    terminating(false), busy(false),
    // initialize the cObjects with names
    messageQueue("Awaiting messages"), //
    postProcessingAppMsg("Post-processing messages"), //
    downloadRateTimer("Download Rate Timer", APP_DOWNLOAD_RATE_TIMER), //
    keepAliveTimer("KeepAlive Timer", APP_KEEP_ALIVE_TIMER), //
    snubbedTimer("Snubbed Timer", APP_SNUBBED_TIMER), //
    timeoutTimer("Timeout Timer", APP_TIMEOUT_TIMER), //
    uploadRateTimer("Upload Rate Timer", APP_UPLOAD_RATE_TIMER) {
    // Active connection if both the infoHash and the remotePeerId are undefined
    this->activeConnection = !(infoHash == -1 && remotePeerId == -1);
}
PeerWireThread::~PeerWireThread() {
    this->cancelEvent(&this->downloadRateTimer);
    this->cancelEvent(&this->keepAliveTimer);
    this->cancelEvent(&this->snubbedTimer);
    this->cancelEvent(&this->timeoutTimer);
    this->cancelEvent(&this->uploadRateTimer);
}

// Implementation of virtual methods from TCPServerThreadBase
void PeerWireThread::closed() {
    this->printDebugMsg("Connection locally closed");
    this->sendApplicationMessage(APP_TCP_LOCAL_CLOSE);
}
void PeerWireThread::peerClosed() {
    this->printDebugMsg("Peer closed.");
    this->sendApplicationMessage(APP_TCP_REMOTE_CLOSE);
}
void PeerWireThread::dataArrived(cMessage *msg, bool urgent) {
    std::string out = "Message \"" + std::string(msg->getName()) + "\" arrived";
    this->printDebugMsg(out);
    this->sendPeerWireMessage(msg);
}
void PeerWireThread::established() {
    // first transition, issued directly to the state machine
    std::ostringstream out;

    IPvXAddress localAddr = this->sock->getLocalAddress();
    IPvXAddress remoteAddr = this->sock->getRemoteAddress();
    int localPort = this->sock->getLocalPort();
    int remotePort = this->sock->getRemotePort();

    if (this->activeConnection) {
        this->connectionSm.tcpActiveConnection();
        out << "Active connection with (";
    } else {
        this->connectionSm.tcpPassiveConnection();
        out << "Passive connection with (";
    }

    out << localAddr << ":" << localPort;
    out << ", " << remoteAddr << ":" << remotePort << ")";
    this->printDebugMsg(out.str());
}
void PeerWireThread::failure(int code) {
    std::ostringstream out;
    out << "Connection failure - ";
    switch (code) {
    case TCP_I_CONNECTION_REFUSED:
        out << "refused";
        break;
    case TCP_I_CONNECTION_RESET:
        out << "reset";
        break;
    case TCP_I_TIMED_OUT:
        out << "timed out";
        break;
    }
    this->printDebugMsg(out.str());
    throw std::logic_error("TCP connection failure");
}
void PeerWireThread::init(TCPSrvHostApp* hostmodule, TCPSocket* socket) {
    // call parent method
    TCPServerThreadBase::init(hostmodule, socket);

    this->btClient = dynamic_cast<BitTorrentClient*>(this->hostmod);
}
void PeerWireThread::timerExpired(cMessage *timer) {
    simtime_t nextRound;
    // timers are not deleted, only rescheduled.
    int kind = timer->getKind();
    switch (kind) {
    case APP_DOWNLOAD_RATE_TIMER:
    case APP_KEEP_ALIVE_TIMER:
    case APP_SNUBBED_TIMER:
    case APP_TIMEOUT_TIMER:
    case APP_UPLOAD_RATE_TIMER:
        this->sendApplicationMessage(kind);
        break;
    default:
        throw std::logic_error("Unknown timer");
        break;
    }
}

// public methods
void PeerWireThread::printDebugMsg(std::string s) {
    std::ostringstream out;
    out << "(Thread) connId " << this->sock->getConnectionId();
    if (this->remotePeerId != -1) {
        out << ", peerId " << this->remotePeerId;
    }
    if (this->infoHash != -1) {
        out << ", infoHash " << this->infoHash;
    }
    out << " - " << s;

    this->btClient->printDebugMsg(out.str());
}
void PeerWireThread::printDebugMsgUpload(std::string s) {
    std::ostringstream out;
    out << "(ThreadUpload) connId " << this->sock->getConnectionId();
    if (this->remotePeerId != -1) {
        out << ", peerId " << this->remotePeerId;
    }
    if (this->infoHash != -1) {
        out << ", infoHash " << this->infoHash;
    }
    out << " - " << s;

    this->btClient->printDebugMsg(out.str());
}
void PeerWireThread::printDebugMsgDownload(std::string s) {
    std::ostringstream out;
    out << "(ThreadDownload) connId " << this->sock->getConnectionId();
    if (this->remotePeerId != -1) {
        out << ", peerId " << this->remotePeerId;
    }
    if (this->infoHash != -1) {
        out << ", infoHash " << this->infoHash;
    }
    out << " - " << s;

    this->btClient->printDebugMsg(out.str());
}
void PeerWireThread::printDebugMsgConnection(std::string s) {
    std::ostringstream out;
    out << "(ThreadConnection) connId " << this->sock->getConnectionId();
    if (this->remotePeerId != -1) {
        out << ", peerId " << this->remotePeerId;
    }
    if (this->infoHash != -1) {
        out << ", infoHash " << this->infoHash;
    }
    out << " - " << s;

    this->btClient->printDebugMsg(out.str());
}

// Private methods
void PeerWireThread::cancelMessages() {
    // cancel all messages that were going to be executed
    while (!this->messageQueue.empty()) {
        cObject * msg = this->messageQueue.pop();
        std::string name(msg->getName());
        std::string out = "Canceled message \"" + name + "\"";
        this->printDebugMsg(out);
        delete msg;
    }
    while (!this->postProcessingAppMsg.empty()) {
        cObject * msg = this->postProcessingAppMsg.pop();
        std::string name(msg->getName());
        std::string out = "Canceled post-message \"" + name + "\"";
        this->printDebugMsg(out);
        delete msg;
    }
}
bool PeerWireThread::hasMessagesToProcess() {
    return !this->messageQueue.empty();
}
simtime_t PeerWireThread::startProcessing() {
    assert(*(this->btClient->threadInProcessingIt) == this && !this->busy);
    // start processing this thread
    this->busy = true;

    // process all ApplicationMsgs, since they take zero processing time
    processAppMessages();

    // PeerWireMsg is the only type of message that take time to process.
    simtime_t processingTime = 0;
    if (!this->messageQueue.empty()) {
        cObject *o = this->messageQueue.pop();
        cMessage *messageInProcessing = static_cast<cMessage *>(o);
        assert(dynamic_cast<PeerWireMsg *>(messageInProcessing)); // not NULL
        processingTime = this->btClient->doubleProcessingTimeHist.random();
        this->issueTransition(messageInProcessing);
    }
    return processingTime;
}
void PeerWireThread::finishProcessing() {
    // finish processing this thread
    this->busy = false;

    if (!this->terminating) {
        this->printDebugMsg("Post-processing.");
        // process all application messages that were generated during the
        // processing of the last PeerWire message.
        while (!this->postProcessingAppMsg.empty()) {
            cObject *o = this->postProcessingAppMsg.pop();
            cMessage *msg = static_cast<cMessage *>(o);
            this->issueTransition(msg);
        }
        // Process all ApplicationMsg until a PeerWireMsg is found
        this->processAppMessages();
        this->printDebugMsg("Finished processing.");
    } else {
        // thread terminated, remove thread from BitTorrentClient
        this->printDebugMsg("Thread terminated.");
        this->btClient->removeThread(this);
    }
}
void PeerWireThread::processAppMessages() {
    while (!this->messageQueue.empty()) {
        cObject * o = this->messageQueue.front();
        ApplicationMsg *appMsg = dynamic_cast<ApplicationMsg *>(o);
        if (appMsg != NULL) {
            this->messageQueue.pop();
            this->issueTransition(appMsg);
        } else {
            break; // not ApplicationMsg, so break out of loop
        }
    }
}
void PeerWireThread::sendApplicationMessage(int id) {
    char const* name;
    char const* debugMsg;

    switch (id) {
#define CASE(X) case X:\
        name = #X;\
        debugMsg = "ApplicationMsg " #X " issued.";\
        break
    // Content Manager events
    CASE(APP_CLOSE);
    CASE(APP_PEER_INTERESTING);
    CASE(APP_PEER_NOT_INTERESTING);
    CASE(APP_SEND_PIECE_MSG);
        // Choker events
    CASE(APP_CHOKE_PEER);
    CASE(APP_UNCHOKE_PEER);
        // TCP events
    CASE(APP_TCP_ACTIVE_CONNECTION);
    CASE(APP_TCP_PASSIVE_CONNECTION);
    CASE(APP_TCP_LOCAL_CLOSE);
    CASE(APP_TCP_REMOTE_CLOSE);
        // timers
    CASE(APP_DOWNLOAD_RATE_TIMER);
    CASE(APP_KEEP_ALIVE_TIMER);
    CASE(APP_SNUBBED_TIMER);
    CASE(APP_TIMEOUT_TIMER);
    CASE(APP_UPLOAD_RATE_TIMER);
    default:
        throw std::logic_error(
            "Trying to call a non-existing application transition");
        break;
    }
#undef CASE
    this->printDebugMsg(std::string(debugMsg));
    ApplicationMsg* appMessage = new ApplicationMsg(name);
    appMessage->setMessageId(id);
    // these messages are generated only when processing messages
    switch (id) {
    case APP_CLOSE:
    case APP_PEER_INTERESTING:
    case APP_PEER_NOT_INTERESTING:
    case APP_SEND_PIECE_MSG:
    case APP_CHOKE_PEER:
    case APP_UNCHOKE_PEER:
        if (this->busy) {
            this->postProcessingAppMsg.insert(appMessage);
        } else {
            this->messageQueue.insert(appMessage);
        }
        break;
    default:
        this->messageQueue.insert(appMessage);
        break;
    }
    // Try to ensure the processing of the first message to arrive.
    if (!this->busy) this->btClient->processNextThread();
}
void PeerWireThread::issueTransition(cMessage const* msg) { // get message Id
    ApplicationMsg const* appMsg = dynamic_cast<ApplicationMsg const*>(msg);
    PeerWireMsg const* pwMsg = dynamic_cast<PeerWireMsg const*>(msg);

    int msgId;

    if (appMsg) {
        msgId = appMsg->getMessageId();
    } else if (pwMsg) {
        msgId = pwMsg->getMessageId();
    } else {
        // consume application message
        delete msg;
        msg = NULL;
        throw std::logic_error("Wrong type of message");
    }

    std::string msgName = msg->getName();
    std::string debugString = "Processing " + msgName;
    try {
        switch (msgId) {
#define CONST_CAST(X) static_cast<X const&>(*msg)
#define CASE_CONN( X , Y ) case X:\
            this->printDebugMsgConnection(debugString);\
            this->connectionSm.Y;\
            break
#define CASE_APP_CONN( X , Y ) case X:\
            this->printDebugMsg(debugString);\
            this->connectionSm.Y;\
            break
#define CASE_DOWNLOAD( X , Y ) case X:\
            this->printDebugMsgDownload(debugString);\
            this->connectionSm.incomingPeerWireMsg();\
            this->downloadSm.Y;\
            break
#define CASE_APP_DOWNLOAD( X , Y ) case X:\
            this->printDebugMsg(debugString);\
            this->downloadSm.Y;\
            break
#define CASE_UPLOAD( X , Y ) case X:\
            this->printDebugMsgUpload(debugString);\
            this->connectionSm.incomingPeerWireMsg();\
            this->uploadSm.Y;\
            break
#define CASE_APP_UPLOAD( X , Y ) case X:\
            this->printDebugMsg(debugString);\
            this->uploadSm.Y;\
            break

        // connectionSm PeerWire transitions
        CASE_CONN(PW_KEEP_ALIVE_MSG, incomingPeerWireMsg());
        CASE_CONN(PW_HANDSHAKE_MSG, handshakeMsg(CONST_CAST(Handshake)));
            // connectionSM Application transitions
        CASE_APP_CONN(APP_CLOSE, applicationClose());
        CASE_APP_CONN(APP_TCP_REMOTE_CLOSE, remoteClose());
        CASE_APP_CONN(APP_TCP_LOCAL_CLOSE, localClose());
        CASE_APP_CONN(APP_TCP_ACTIVE_CONNECTION, tcpActiveConnection());
        CASE_APP_CONN(APP_TCP_PASSIVE_CONNECTION, tcpPassiveConnection());
            // connectionSM timers
        CASE_APP_CONN(APP_KEEP_ALIVE_TIMER, keepAliveTimer());
        CASE_APP_CONN(APP_TIMEOUT_TIMER, timeout());

            // downloadSm PeerWire transitions
        CASE_DOWNLOAD(PW_CHOKE_MSG, chokeMsg());
        CASE_DOWNLOAD(PW_UNCHOKE_MSG, unchokeMsg());
        CASE_DOWNLOAD(PW_HAVE_MSG, haveMsg(CONST_CAST(HaveMsg)));
        CASE_DOWNLOAD(PW_BITFIELD_MSG, bitFieldMsg(CONST_CAST(BitFieldMsg)));
        CASE_DOWNLOAD(PW_PIECE_MSG, pieceMsg(CONST_CAST(PieceMsg)));
            // downloadSm Application transitions
        CASE_APP_DOWNLOAD(APP_PEER_INTERESTING, peerInteresting());
        CASE_APP_DOWNLOAD(APP_PEER_NOT_INTERESTING, peerNotInteresting());
            // downloadSm timers
        CASE_APP_DOWNLOAD(APP_DOWNLOAD_RATE_TIMER, downloadRateTimer());
        CASE_APP_DOWNLOAD(APP_SNUBBED_TIMER, snubbedTimer());

            // uploadSm PeerWire transitions
        CASE_UPLOAD(PW_INTERESTED_MSG, interestedMsg());
        CASE_UPLOAD(PW_NOT_INTERESTED_MSG, notInterestedMsg());
        CASE_UPLOAD(PW_REQUEST_MSG, requestMsg(CONST_CAST(RequestMsg)));
        CASE_UPLOAD(PW_CANCEL_MSG, cancelMsg(CONST_CAST(CancelMsg)));
            // uploadSm Application transitions
        CASE_APP_UPLOAD(APP_CHOKE_PEER, chokePeer());
        CASE_APP_UPLOAD(APP_UNCHOKE_PEER, unchokePeer());
        CASE_APP_UPLOAD(APP_SEND_PIECE_MSG, sendPieceMsg());
            // uploadSm timers
        CASE_APP_UPLOAD(APP_UPLOAD_RATE_TIMER, uploadRateTimer());

#undef CONST_CAST
#undef CASE_CONN
#undef CASE_APP_CONN
#undef CASE_DOWNLOAD
#undef CASE_APP_DOWNLOAD
#undef CASE_UPLOAD
#undef CASE_APP_UPLOAD
        }

        delete msg;
        msg = NULL; // consume the message
    } catch (statemap::TransitionUndefinedException & e) {
        delete msg;
        msg = NULL; // consume the message

        std::ostringstream out;
        out << e.what();
        out << " - Transition " << e.getTransition();
        out << " in state " << e.getState();
        throw std::logic_error(out.str());
    } catch (std::invalid_argument &e) {
        delete msg;
        msg = NULL; // consume the message
        throw std::logic_error("Passed wrong argument to the state machine.");
    }
}
void PeerWireThread::sendPeerWireMessage(cMessage * msg) {
    PeerWireMsgBundle * bundleMsg = dynamic_cast<PeerWireMsgBundle *>(msg);
    PeerWireMsg* peerWireMsg = dynamic_cast<PeerWireMsg *>(msg);
    assert(bundleMsg || peerWireMsg); // either one or the other

    if (bundleMsg != NULL) {
        // unpacks the bundle and inserts all messages in the messageQueue
        cPacketQueue * bundle = bundleMsg->getBundle();
        while (!bundle->empty()) {
            this->messageQueue.insert(bundle->pop());
        }
        delete bundle;
        delete bundleMsg; // the bundle is no longer needed, so delete it
    } else {
        this->messageQueue.insert(peerWireMsg);
    }

    // Try to ensure the processing of the first message to arrive.
    if (!this->busy) this->btClient->processNextThread();
}
//void PeerWireThread::oldUnchoked() {
//    this->btClient->setOldUnchoked(true, this->infoHash, this->remotePeerId);
//}
