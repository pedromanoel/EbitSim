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

#include "BitTorrentClient.h"
#include "ContentManager.h"
#include "PeerWire_m.h"
#include "PeerWireMsgBundle_m.h"

//#define DEBUG_CONN

Register_Class(PeerWireThread);

// Own methods
PeerWireThread::PeerWireThread() :
        connectionSm(*this), downloadSm(*this), uploadSm(*this), activeConnection(
                false), btClient(NULL), choker(NULL), contentManager(NULL), infoHash(
                -1), remotePeerId(-1), terminated(false), processing(false), peerWireMessageBuffer(
                "Awaiting PeerWire messages"), applicationMsgQueue(
                "Awaiting Application messages"), downloadRateTimer(
                "Download Rate Timer",
                BitTorrentClient::SELF_DOWNLOAD_RATE_TIMER), keepAliveTimer(
                "KeepAlive Timer", BitTorrentClient::SELF_KEEP_ALIVE_TIMER), snubbedTimer(
                "Snubbed Timer", BitTorrentClient::SELF_SNUBBED_TIMER), timeoutTimer(
                "Timeout Timer", BitTorrentClient::SELF_TIMEOUT_TIMER), uploadRateTimer(
                "Upload Rate Timer", BitTorrentClient::SELF_UPLOAD_RATE_TIMER) {
}

PeerWireThread::PeerWireThread(int infoHash, int remotePeerId) :
        connectionSm(*this), downloadSm(*this), uploadSm(*this), activeConnection(
                infoHash != -1 && remotePeerId != -1), btClient(NULL), choker(
                NULL), contentManager(NULL), infoHash(infoHash), remotePeerId(
                remotePeerId), terminated(false), processing(false), peerWireMessageBuffer(
                "Awaiting messages"), downloadRateTimer("Download Rate Timer",
                BitTorrentClient::SELF_DOWNLOAD_RATE_TIMER), keepAliveTimer(
                "KeepAlive Timer", BitTorrentClient::SELF_KEEP_ALIVE_TIMER), snubbedTimer(
                "Snubbed Timer", BitTorrentClient::SELF_SNUBBED_TIMER), timeoutTimer(
                "Timeout Timer", BitTorrentClient::SELF_TIMEOUT_TIMER), uploadRateTimer(
                "Upload Rate Timer", BitTorrentClient::SELF_UPLOAD_RATE_TIMER) {

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
    IPvXAddress localAddr = this->getSocket()->getLocalAddress();
    IPvXAddress remoteAddr = this->getSocket()->getRemoteAddress();
    int localPort = this->getSocket()->getLocalPort();
    int remotePort = this->getSocket()->getRemotePort();

    std::ostringstream out;
    out << "Connection (" << localAddr << ":" << localPort << ", ";
    out << remoteAddr << ":" << remotePort << ") closed.";

    this->printDebugMsg(out.str());
    // message to the state machine
    this->sendApplicationMessage(APP_DROP);
}
void PeerWireThread::dataArrived(cMessage *msg, bool urgent) {
    assert(
            dynamic_cast<PeerWireMsgBundle *>(msg) != NULL
                    || dynamic_cast<PeerWireMsg *>(msg) != NULL);

    std::ostringstream out;
    out << "Message \"" << msg->getName() << "\" arrived";

    // unpacks the bundle and inserts all messages in the messageQueue
    if (dynamic_cast<PeerWireMsgBundle *>(msg) != NULL) {
        cPacketQueue & bundle =
                static_cast<PeerWireMsgBundle *>(msg)->getBundle();
        while (!bundle.empty()) {
            this->peerWireMessageBuffer.insert(bundle.pop());
        }

        // the bundle is no longer needed, so delete it
        delete msg;
    } else {
        // insert the current message in the queue
        this->peerWireMessageBuffer.insert(msg);
    }

    // Will process the next thread if no thread is currently being processed.
    this->btClient->processNextThread();

    this->printDebugMsg(out.str());
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

    this->sendApplicationMessage(APP_DROP);
}
void PeerWireThread::init(TCPSrvHostApp* hostmodule, TCPSocket* socket) {
    // call parent method
    TCPServerThreadBase::init(hostmodule, socket);

    this->btClient = dynamic_cast<BitTorrentClient*>(this->hostmod);
}
void PeerWireThread::peerClosed() {
    if (this->sock->getState() == TCPSocket::CONNECTED) {
        this->sock->close();
    }
}
void PeerWireThread::statusArrived(TCPStatusInfo* status) {
}
/*!
 * Timer messages are not consumed, to allow re-scheduling of the timer.
 * The deletion of the message occurs at the dtor. All other self-messages
 * are consumed.
 */
void PeerWireThread::timerExpired(cMessage *timer) {
    simtime_t nextRound;
    // timers are not deleted, only rescheduled.
    switch (timer->getKind()) {
    case BitTorrentClient::SELF_KEEP_ALIVE_TIMER:
        this->sendApplicationMessage(APP_KEEP_ALIVE_TIMER);
        break;
    case BitTorrentClient::SELF_TIMEOUT_TIMER:
        this->sendApplicationMessage(APP_TIMEOUT);
        break;
    case BitTorrentClient::SELF_SNUBBED_TIMER:
        this->sendApplicationMessage(APP_SNUBBED_TIMER);
        break;
    case BitTorrentClient::SELF_DOWNLOAD_RATE_TIMER:
        this->sendApplicationMessage(APP_DOWNLOAD_RATE_TIMER);
        break;
    case BitTorrentClient::SELF_UPLOAD_RATE_TIMER:
        this->sendApplicationMessage(APP_UPLOAD_RATE_TIMER);
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
    } else {
        out << ", peerId unknown";
    }
    out << ", infoHash " << this->infoHash << " - " << s;

    this->btClient->printDebugMsg(out.str());
}
void PeerWireThread::printDebugMsgUpload(std::string s) {
    std::ostringstream out;
    out << "(ThreadUpload) connId " << this->sock->getConnectionId();
    if (this->remotePeerId != -1) {
        out << ", peerId " << this->remotePeerId;
    } else {
        out << ", peerId unknown";
    }
    out << ", infoHash " << this->infoHash << " - " << s;

    this->btClient->printDebugMsg(out.str());
}
void PeerWireThread::printDebugMsgDownload(std::string s) {
    std::ostringstream out;
    out << "(ThreadDownload) connId " << this->sock->getConnectionId();
    if (this->remotePeerId != -1) {
        out << ", peerId " << this->remotePeerId;
    } else {
        out << ", peerId unknown";
    }
    out << ", infoHash " << this->infoHash << " - " << s;

    this->btClient->printDebugMsg(out.str());
}
void PeerWireThread::printDebugMsgConnection(std::string s) {
    std::ostringstream out;
    out << "(ThreadConnection) connId " << this->sock->getConnectionId();
    if (this->remotePeerId != -1) {
        out << ", peerId " << this->remotePeerId;
    } else {
        out << ", peerId unknown";
    }
    out << ", infoHash " << this->infoHash << " - " << s;

    this->btClient->printDebugMsg(out.str());
}

// Private methods
void PeerWireThread::finishProcessing() {
    // finish processing this thread
    this->processing = false;
    this->printDebugMsg("Finished processing.");

    if (!this->terminated) {
        // process all application messages that were generated during the
        // processing of the last PeerWire message.
        while (!this->applicationMsgQueue.empty()) {
            cMessage * appMsg =
                    dynamic_cast<cMessage *>(this->applicationMsgQueue.pop());
            this->issueTransition(appMsg);
        }
    } else {
        // thread terminated, remove thread from BitTorrentClient
        this->printDebugMsg("Deleting thread.");
        this->btClient->removeThread(this);
    }
    // schedule the processing of the next thread
}
bool PeerWireThread::hasMessagesToProcess() {
    return !(this->applicationMsgQueue.empty()
            && this->peerWireMessageBuffer.empty());
}
bool PeerWireThread::isProcessing() {
    return this->processing;
}
void PeerWireThread::computeApplicationMsg(ApplicationMsg * msg) {
    this->applicationMsgQueue.insert(msg);
    this->btClient->processNextThread();
}
simtime_t PeerWireThread::processThread() {
    assert(*this->btClient->threadInProcessingIt == this);
    assert(!this->isProcessing());

    this->printDebugMsg("Started processing.");

    // start processing this thread
    this->processing = true;

    // process ApplicationMsg that where issued before the processing of a
    // PeerWireMsg
    while (!this->applicationMsgQueue.isEmpty()) {
        cMessage * appMsg =
                dynamic_cast<cMessage *>(this->applicationMsgQueue.pop());
        this->issueTransition(appMsg);
    }

    simtime_t processingTime = 0;
    // PeerWireMsg is the only type of message that take time to process.
    if (!this->peerWireMessageBuffer.isEmpty()) {
        processingTime = this->btClient->doubleProcessingTimeHist.random();
        cMessage *messageInProcessing =
                dynamic_cast<cMessage *>(this->peerWireMessageBuffer.pop());

        // issue the state machine transition at the beginning of the
        // processing of the PeerWireMsg
        this->issueTransition(messageInProcessing);
    }
    return processingTime;
}
void PeerWireThread::issueTransition(cMessage const* msg) { // get message Id
    int messageId;
    std::string debugString;
    if (dynamic_cast<ApplicationMsg const*>(msg)) {
        messageId = static_cast<ApplicationMsg const*>(msg)->getMessageId();
        std::ostringstream out;
        out << "ApplicationMsg " << msg->getName() << " processed";
        debugString = out.str();
    } else if (dynamic_cast<PeerWireMsg const*>(msg)) {
        messageId = static_cast<PeerWireMsg const*>(msg)->getMessageId();
        std::ostringstream out;
        out << "PeerWireMsg " << msg->getName() << " processed";
        debugString = out.str();
    } else {
        // consume application message
        delete msg;
        msg = NULL;
        throw std::logic_error("Wrong type of message");
    }
    try {
        switch (messageId) {
        // connectionSm transitions
        case PW_KEEP_ALIVE_MSG:
            this->printDebugMsgConnection(debugString);
            this->connectionSm.incomingPeerWireMsg();
            break;
        case PW_HANDSHAKE_MSG:
            this->printDebugMsgConnection(debugString);
            this->connectionSm.handshakeMsg(
                    dynamic_cast<Handshake const&>(*msg));
            break;

            // downloadSm transitions
        case PW_CHOKE_MSG:
            this->printDebugMsgDownload(debugString);
            this->connectionSm.incomingPeerWireMsg();
            this->downloadSm.chokeMsg();
            break;
        case PW_UNCHOKE_MSG:
            this->printDebugMsgDownload(debugString);
            this->connectionSm.incomingPeerWireMsg();
            this->downloadSm.unchokeMsg();
            break;
        case PW_HAVE_MSG:
            this->printDebugMsgDownload(debugString);
            this->connectionSm.incomingPeerWireMsg();
            this->downloadSm.haveMsg(dynamic_cast<HaveMsg const&>(*msg));
            break;
        case PW_BITFIELD_MSG:
            this->printDebugMsgDownload(debugString);
            this->connectionSm.incomingPeerWireMsg();
            this->downloadSm.bitFieldMsg(
                    dynamic_cast<BitFieldMsg const&>(*msg));
            break;
        case PW_PIECE_MSG:
            this->printDebugMsgDownload(debugString);
            this->connectionSm.incomingPeerWireMsg();
            this->downloadSm.pieceMsg(dynamic_cast<PieceMsg const&>(*msg));
            break;
            // uploadSm transitions
        case PW_INTERESTED_MSG:
            this->printDebugMsgUpload(debugString);
            this->connectionSm.incomingPeerWireMsg();
            this->uploadSm.interestedMsg();
            break;
        case PW_NOT_INTERESTED_MSG:
            this->printDebugMsgUpload(debugString);
            this->connectionSm.incomingPeerWireMsg();
            this->uploadSm.notInterestedMsg();
            break;
        case PW_REQUEST_MSG:
            this->printDebugMsgUpload(debugString);
            this->connectionSm.incomingPeerWireMsg();
            this->uploadSm.requestMsg(dynamic_cast<RequestMsg const&>(*msg));
            break;
        case PW_CANCEL_MSG:
            this->printDebugMsgUpload(debugString);
            this->connectionSm.incomingPeerWireMsg();
            this->uploadSm.cancelMsg(dynamic_cast<CancelMsg const&>(*msg));
            break;

            // Application Messages
        case APP_DROP:
            this->printDebugMsg(debugString);
            this->connectionSm.DROP();
            this->downloadSm.DROP();
            this->uploadSm.DROP();
            break;
        case APP_KEEP_ALIVE_TIMER:
            this->printDebugMsg(debugString);
            this->connectionSm.keepAliveTimer();
            break;
        case APP_TCP_ACTIVE_CONNECTION:
            this->printDebugMsg(debugString);
            this->connectionSm.tcpActiveConnection();
            break;
        case APP_TCP_PASSIVE_CONNECTION:
            this->printDebugMsg(debugString);
            this->connectionSm.tcpPassiveConnection();
            break;
        case APP_TIMEOUT:
            this->printDebugMsg(debugString);
            this->connectionSm.timeout();
            break;
        case APP_DOWNLOAD_RATE_TIMER:
            this->printDebugMsg(debugString);
            this->downloadSm.downloadRateTimer();
            break;
        case APP_PEER_INTERESTING:
            this->printDebugMsg(debugString);
            this->downloadSm.peerInteresting();
            break;
        case APP_PEER_NOT_INTERESTING:
            this->printDebugMsg(debugString);
            this->downloadSm.peerNotInteresting();
            break;
        case APP_SNUBBED_TIMER:
            this->printDebugMsg(debugString);
            this->downloadSm.snubbedTimer();
            break;
        case APP_CHOKE_PEER:
            this->printDebugMsg(debugString);
            this->uploadSm.chokePeer();
            break;
        case APP_UNCHOKE_PEER:
            this->printDebugMsg(debugString);
            this->uploadSm.unchokePeer();
            break;
        case APP_UPLOAD_RATE_TIMER:
            this->printDebugMsg(debugString);
            this->uploadSm.uploadRateTimer();
            break;
        }

        // consume the message
        delete msg;
        msg = NULL;
    } catch (statemap::TransitionUndefinedException & e) {
        std::ostringstream out;
        out << e.what();
        out << " - Transition " << e.getTransition();
        out << " in state " << e.getState();

        // consume application message
        delete msg;
        msg = NULL;

        throw std::logic_error(out.str());
    } catch (std::invalid_argument &e) {
        // consume application message
        delete msg;
        msg = NULL;
        throw std::logic_error("Passed wrong argument to the state machine.");
    }
}
//void PeerWireThread::oldUnchoked() {
//    this->btClient->setOldUnchoked(true, this->infoHash, this->remotePeerId);
//}
//void PeerWireThread::peerSnubbed() {
//    this->btClient->setSnubbed(true, this->infoHash, this->remotePeerId);
//}
void PeerWireThread::sendApplicationMessage(int id) {
    const char* name;
    switch (id) {
    case APP_DROP:
        name = "Drop Connection";
        break;
    case APP_KEEP_ALIVE_TIMER:
        name = "Keep Alive Timer Expired";
        break;
    case APP_TIMEOUT:
        name = "Keep Alive Timeout Expired";
        break;
    case APP_DOWNLOAD_RATE_TIMER:
        name = "Download Rate Timer Expired";
        break;
    case APP_PEER_INTERESTING:
        name = "Peer Interesting";
        break;
    case APP_PEER_NOT_INTERESTING:
        name = "Peer Not Interesting";
        break;
    case APP_SNUBBED_TIMER:
        name = "Snubbed Timer Expired";
        break;
    case APP_CHOKE_PEER:
        name = "Choke Peer";
        break;
    case APP_UNCHOKE_PEER:
        name = "Unchoke Peer";
        break;
    case APP_UPLOAD_RATE_TIMER:
        name = "Upload Rate Timer Expired";
        break;
    default:
        throw std::logic_error(
                "Trying to call a non-existing application transition");
        // TODO do something bad
        break;
    }

    std::ostringstream out;
    out << "ApplicationMsg \"" << name << "\" sent.";
    this->printDebugMsg(out.str());

    ApplicationMsg* appMessage = new ApplicationMsg(name);
    appMessage->setMessageId(id);
    // compute the application message
    this->computeApplicationMsg(appMessage);
}
