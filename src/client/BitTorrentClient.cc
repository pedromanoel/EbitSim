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

#include "BitTorrentClient.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <IPAddressResolver.h>

#include "Application_m.h"
#include "PeerWire_m.h"
#include "PeerWireMsgBundle_m.h"
#include "client/connection/PeerWireThread.h"
#include "SwarmManager.h"

Define_Module(BitTorrentClient);

// Public Methods
BitTorrentClient::BitTorrentClient() :
        swarmManager(NULL), endOfProcessingTimer("End of processing"),
        //    processNextThreadTimer("Start processing thread"),
        doubleProcessingTimeHist("Piece processing time histogram"), snubbedInterval(
                0), timeoutInterval(0), keepAliveInterval(0), /*oldUnchokeInterval(0),*/
        downloadRateInterval(0), uploadRateInterval(0), localPort(-1), localPeerId(
                -1), debugFlag(false), globalNumberOfPeers(0), numberOfActivePeers(
                0), numberOfPassivePeers(0), prevNumUnconnected(0), prevNumConnected(
                0) {
}

BitTorrentClient::~BitTorrentClient() {
    // delete all sockets
    this->socketMap.deleteSockets();
    // delete all threads, which are not deleted when the sockets are destroyed.
    std::set<PeerWireThread *>::iterator it = this->allThreads.begin();
    for (; it != this->allThreads.end(); ++it) {
        PeerWireThread * thread = *it;
        delete thread;
        thread = NULL;
    }
}

// Methods used by the Choker
void BitTorrentClient::chokePeer(int infoHash, int peerId) {
    Enter_Method("chokePeer(infoHash: %d, peerId: %d)", infoHash, peerId);

    this->printDebugMsg("calling getPeerEntry from chokePeer");
    PeerEntry & peer = this->getPeerEntry(infoHash, peerId);
    if (peer.isUnchoked()) {
        peer.setUnchoked(false, simTime());
        peer.getThread()->sendApplicationMessage(APP_CHOKE_PEER);
    }
}
PeerEntryPtrVector BitTorrentClient::getFastestToDownload(int infoHash) const {
    Enter_Method("getFastestToDownload(infoHash: %d)", infoHash);

    PeerMap const& peerMap = this->getSwarm(infoHash).get<2>();
    PeerEntryPtrVector orderedPeers;
    orderedPeers.reserve(peerMap.size());
    if (peerMap.size()) {
        PeerMapConstIt it = peerMap.begin();
        for (; it != peerMap.end(); ++it) {
            orderedPeers.push_back(&it->second);
        }

        std::sort(orderedPeers.begin(), orderedPeers.end(),
                PeerEntry::sortByDownloadRate);
    }

    return orderedPeers;
}
PeerEntryPtrVector BitTorrentClient::getFastestToUpload(int infoHash) {
    Enter_Method("getFastestToUpload(infoHash: %d)", infoHash);

    PeerMap const& peerMap = this->getSwarm(infoHash).get<2>();
    PeerEntryPtrVector vector;
    if (peerMap.size()) {
        PeerMapConstIt it = peerMap.begin();
        for (; it != peerMap.end(); ++it) {
            vector.push_back(&it->second);
        }

        std::sort(vector.begin(), vector.end(), PeerEntry::sortByUploadRate);
    }

    return vector;
}
void BitTorrentClient::unchokePeer(int infoHash, int peerId) {
    Enter_Method("unchokePeer(infoHash: %d, peerId: %d)", infoHash, peerId);

    this->printDebugMsg("calling getPeerEntry from unchokePeer");
    PeerEntry & peer = this->getPeerEntry(infoHash, peerId);

    if (!peer.isUnchoked()) {
        peer.setUnchoked(true, simTime());
        peer.getThread()->sendApplicationMessage(APP_UNCHOKE_PEER);
    }
}

// Methods used by the ContentManager
void BitTorrentClient::drop(int infoHash, int peerId) {
    Enter_Method("drop(infoHash: %d, peerId: %d)", infoHash, peerId);

    this->printDebugMsg("calling getPeerEntry from drop");
    PeerEntry & peer = this->getPeerEntry(infoHash, peerId);
    peer.getThread()->sendApplicationMessage(APP_DROP);
}
void BitTorrentClient::finishedDownload(int infoHash) {
    Enter_Method("finishedDownload(infoHash: %d)", infoHash);
    Swarm & swarm = this->getSwarm(infoHash);
    bool & seeding = swarm.get<4>();
    seeding = true;
    // no more active downloads
    UnconnectedList & unconnectedList = swarm.get<3>();
    unconnectedList.clear();
    this->swarmManager->finishedDownload(infoHash);
}
void BitTorrentClient::peerInteresting(int infoHash, int peerId) {
    Enter_Method("peerInteresting(infoHash: %d, peerId: %d)", infoHash, peerId);

    this->printDebugMsg("calling getPeerEntry from peerInteresting");
    PeerEntry & peer = this->getPeerEntry(infoHash, peerId);
    peer.getThread()->sendApplicationMessage(APP_PEER_INTERESTING);
}
void BitTorrentClient::peerNotInteresting(int infoHash, int peerId) {
    Enter_Method("peerNotInteresting(infoHash: %d, peerId: %d)", infoHash,
            peerId);

    this->printDebugMsg("calling getPeerEntry from peerNotInteresting");
    PeerEntry & peer = this->getPeerEntry(infoHash, peerId);
    peer.getThread()->sendApplicationMessage(APP_PEER_NOT_INTERESTING);
}
void BitTorrentClient::sendHaveMessage(int infoHash, int pieceIndex) {
    Enter_Method("sendHaveMessage(pieceIndex: %d)", pieceIndex);

    PeerMap const& peerMap = this->getSwarm(infoHash).get<2>();
    PeerMapConstIt it = peerMap.begin();

    // Send have message to all peers from the swarm connected with the Client
    while (it != peerMap.end()) {
        std::ostringstream nameStream;
        nameStream << "HaveMsg (" << pieceIndex << ")";
        HaveMsg* haveMsg = new HaveMsg(nameStream.str().c_str());
        haveMsg->setIndex(pieceIndex);
        (it->second).getThread()->sendPeerWireMsg(haveMsg);
        ++it;
    }
}

// Methods used by the SwarmManager
void BitTorrentClient::addSwarm(int infoHash, bool newSwarmSeeding) {
    Enter_Method("addSwarm(infoHash: %d, %s)", infoHash,
            (newSwarmSeeding ? "seeding" : "leeching"));

    // error if swarm already exists
    if (this->swarmMap.count(infoHash)) {
        throw std::logic_error("Swarm already defined");
    }

    // create swarm
    Swarm & swarm = this->swarmMap[infoHash];
    int & numActive = swarm.get<0>();
    int & numPassive = swarm.get<1>();
    bool & seeding = swarm.get<4>();
    numActive = 0;
    numPassive = 0;
    seeding = newSwarmSeeding;
}
void BitTorrentClient::removeSwarm(int infoHash) {
    Enter_Method("removeSwarm(infoHash: %d)", infoHash);

    SwarmMapIt it = this->swarmMap.find(infoHash);

    if (it == this->swarmMap.end()) {
        throw std::logic_error("Swarm don't exist");
    }

    // TODO close connections with all peers
    throw std::logic_error("Implement method");
}

// Private Methods
// Methods used by the SwarmManagerThread
void BitTorrentClient::addUnconnectedPeers(int infoHash,
        std::list<tuple<int, IPvXAddress, int> > & peers) {
    Enter_Method("addUnconnectedPeers(infoHash: %d, qtty: %d)", infoHash,
            peers.size());
    Swarm & swarm = this->getSwarm(infoHash);
    // only add unconnected peers if not seeding
    bool const& seeding = swarm.get<4>();
    if (!seeding) {
        UnconnectedList & unconnectedList = swarm.get<3>();
        unconnectedList.splice(unconnectedList.end(), peers);
        // sort the unconnected list in order to remove the duplicates
        unconnectedList.sort();
        unconnectedList.unique();
        // try to connect to more peers, if possible
        attemptActiveConnections(swarm, infoHash);

        // if the number of unconnected peers changed, emit a signal
        if (this->prevNumUnconnected != unconnectedList.size()) {
            this->prevNumUnconnected = unconnectedList.size();
            emit(this->numUnconnected_Signal, this->prevNumUnconnected);
        }
    }
}

// Methods used by the PeerWireThread
void BitTorrentClient::addConnectedPeer(int infoHash, int peerId,
        PeerWireThread* thread, bool active) {
    Swarm & swarm = this->getSwarm(infoHash);
    PeerMap & peerMap = swarm.get<2>();

    // if the number of connected peers changed, emit a signal
    if (this->prevNumConnected != peerMap.size()) {
        this->prevNumConnected = peerMap.size();
        emit(this->numConnected_Signal, this->prevNumConnected);
    }

    if (thread->activeConnection == false) {
        int & numPassive = swarm.get<1>();
        ++numPassive;
    }

    PeerMapIt lb = peerMap.lower_bound(peerId);
    // only try to add if Peer not already connected
    if (lb == peerMap.end() || lb->first != peerId) {
        // use lb as a hint for insertion
        peerMap.insert(lb, std::make_pair(peerId, PeerEntry(peerId, thread)));
    } else {
        throw std::logic_error("Can't connect with the same Peer twice");
    }

}
void BitTorrentClient::calculateDownloadRate(int infoHash, int peerId) {
    throw std::logic_error("Implement this calculateDownloadRate.");
}
void BitTorrentClient::calculateUploadRate(int infoHash, int peerId) {
    throw std::logic_error("Implement this calculateUploadRate.");
}
bool BitTorrentClient::canConnect(int infoHash, int peerId, bool active) const {
    Swarm const& swarm = this->getSwarm(infoHash);
    PeerMap const& peerMap = swarm.get<2>();

    bool successful = false;

    // can't connect if already in the PeerMap
    if (!peerMap.count(peerId)) {

        // check if there are passive slots available for connection
        int const& numPassive = swarm.get<1>();
        int numAvailableSlots = this->numberOfPassivePeers - numPassive;

        // active slots are used as passive slots when seeding
        bool const& seeding = swarm.get<4>();
        if (seeding) {
            int const& numActive = swarm.get<0>();
            numAvailableSlots += this->numberOfActivePeers - numActive;
        }

        this->printDebugMsgConnections("canConnect", infoHash, swarm);

        // Check slots only when the connection is passive, because active connections
        // are checked when made
        successful = active || numAvailableSlots > 0;
    } else {
        this->printDebugMsg("Peer already in swarm");
    }

    return successful;
}

void BitTorrentClient::processNextThread() {
    std::set<PeerWireThread*>::iterator nextThreadIt =
            this->threadInProcessingIt;

    // allThreads will never be empty, because this method is called from the
    // thread, therefore there is at least one element in the set.
    assert(!this->allThreads.empty());

    // don't start processing another thread if the current one is still processing
    if (!(*this->threadInProcessingIt)->isProcessing()
            && !this->endOfProcessingTimer.isScheduled()) {
        // search the circular queue for a thread with messages to process

        bool hasMessages = false;

        do {
            ++nextThreadIt;

            // implements a circular queue
            if (nextThreadIt == this->allThreads.end()) {
                nextThreadIt = this->allThreads.begin();
            }

            hasMessages = (*nextThreadIt)->hasMessagesToProcess();
        } while (nextThreadIt != this->threadInProcessingIt && !hasMessages);

        if (hasMessages) {
            std::ostringstream out;
            out << "Changing from thread "
                    << (*this->threadInProcessingIt)->remotePeerId;
            out << " to " << (*nextThreadIt)->remotePeerId;
            this->printDebugMsg(out.str());

            this->threadInProcessingIt = nextThreadIt;
            simtime_t processingTime =
                    (*this->threadInProcessingIt)->processThread();
            emit(this->processingTime, processingTime);
            this->scheduleAt(simTime() + processingTime,
                    &this->endOfProcessingTimer);
        }
    }
}

void BitTorrentClient::removePeerInfo(int infoHash, int peerId, int connId,
        bool active) {
    Swarm & swarm = this->getSwarm(infoHash);
    // remove from the connected list
    PeerMap & peerMap = swarm.get<2>();
    peerMap.erase(peerId);

    // try to remove from the active connected peers
    // if the peer was actively connected, decrement numOfActive, effectively
    // opening one active connection slot. Else, decrement numOfPassive.
    int & numPassive = swarm.get<1>();
    int & numActive = swarm.get<0>();

    if (this->activeConnectedPeers.erase(std::make_pair(infoHash, peerId))) {
        // decrement numOfActive
        --numActive;
        attemptActiveConnections(swarm, infoHash);
    } else {
        // decrement numOfPassive
        --numPassive;
    }

    this->printDebugMsgConnections("removePeerInfo", infoHash, swarm);

    if (peerMap.empty()) {
        this->swarmManager->stopChoker(infoHash);
    }
}
void BitTorrentClient::setInterested(bool interested, int infoHash,
        int peerId) {
    this->printDebugMsg("calling getPeerEntry from setInterested");
    this->getPeerEntry(infoHash, peerId).setInterested(interested);
    //    throw std::logic_error("Implement this setInterested.");
}
void BitTorrentClient::setOldUnchoked(bool oldUnchoke, int infoHash,
        int peerId) {
    this->printDebugMsg("calling getPeerEntry from setOldUnchoked");
    this->getPeerEntry(infoHash, peerId).setOldUnchoked(oldUnchoke);
}
void BitTorrentClient::setSnubbed(bool snubbed, int infoHash, int peerId) {
    this->printDebugMsg("calling getPeerEntry from setSnubbed");
    this->getPeerEntry(infoHash, peerId).setSnubbed(snubbed);
}

// Private Methods
void BitTorrentClient::attemptActiveConnections(Swarm & swarm, int infoHash) {
    PeerMap const& peerMap = swarm.get<2>();
    // only make active connections if not seeding
    bool const& seeding = swarm.get<4>();
    if (!seeding) {
        UnconnectedList & unconnectedList = swarm.get<3>();
        int & numActive = swarm.get<0>();

        int numActiveSlost = this->numberOfActivePeers - numActive;

        while (numActiveSlost && unconnectedList.size()) {
            tuple<int, IPvXAddress, int> peer = unconnectedList.front();
            int peerId = peer.get<0>();
            bool notConnected = !peerMap.count(peerId);
            bool notConnecting = this->activeConnectedPeers.count(
                    std::make_pair(infoHash, peerId)) == 0;

            // only connect if not already connecting or connected
            if (notConnected && notConnecting) {
                --numActiveSlost; // decrease the number of slots
                ++numActive; // increase the number of active connections

                // get unconnected peer, connect with it then remove it from the list
                this->connect(infoHash, peer); // establish the tcp connection
                this->activeConnectedPeers.insert(
                        std::make_pair(infoHash, peerId));
            }
            unconnectedList.pop_front();
        }

        this->printDebugMsgConnections("attemptActiveConnections", infoHash,
                swarm);
    }
}
/*!
 * The Peer address is acquired from the Tracker.
 */
void BitTorrentClient::connect(int infoHash,
        tuple<int, IPvXAddress, int> const& peer) {
    // initialize variables with the tuple content
    int peerId, port;
    IPvXAddress ip;
    tie(peerId, ip, port) = peer;

    TCPSocket * socket = new TCPSocket();
    socket->setOutputGate(gate("tcpOut"));

    PeerWireThread *proc = new PeerWireThread(infoHash, peerId);
    // save the pointer to proc so it can be properly disposed at the end of the
    // simulation.
    this->allThreads.insert(proc);

    if (this->allThreads.size() == 1) {
        // there was no thread before this insertion, so no processing was being
        // made. Set the iterator to point to the beginning of the set
        this->threadInProcessingIt = this->allThreads.begin();
    }

    // attaches the callback obj to the socket
    socket->setCallbackObject(proc);
    proc->init(this, socket);

    this->socketMap.addSocket(socket);

    //update display string
    this->updateDisplay();

    std::ostringstream out;
    out << "connId " << socket->getConnectionId() << ", ";
    out << "Opening connection with " << ip << ":" << port;
    this->printDebugMsg(out.str());
    socket->connect(ip, port);
}
void BitTorrentClient::closeListeningSocket() {
    // If the socket isn't closed, close it.
    if (this->serverSocket.getState() != TCPSocket::CLOSED) {
        this->serverSocket.close();

        this->printDebugMsg("closed listening socket.");
    }
}
void BitTorrentClient::emitReceivedSignal(int messageId) {
    switch (messageId) {
    case PW_CHOKE_MSG:
        emit(this->chokeReceived_Signal, this->localPeerId);
        break;
    case PW_UNCHOKE_MSG:
        emit(this->unchokeReceived_Signal, this->localPeerId);
        break;
    case PW_INTERESTED_MSG:
        emit(this->interestedReceived_Signal, this->localPeerId);
        break;
    case PW_NOT_INTERESTED_MSG:
        emit(this->notInterestedReceived_Signal, this->localPeerId);
        break;
    case PW_HAVE_MSG:
        emit(this->haveReceived_Signal, this->localPeerId);
        break;
    case PW_BITFIELD_MSG:
        emit(this->bitFieldReceived_Signal, this->localPeerId);
        break;
    case PW_REQUEST_MSG:
        emit(this->requestReceived_Signal, this->localPeerId);
        break;
    case PW_PIECE_MSG:
        emit(this->pieceReceived_Signal, this->localPeerId);
        break;
    case PW_CANCEL_MSG:
        emit(this->cancelReceived_Signal, this->localPeerId);
        break;
    case PW_KEEP_ALIVE_MSG:
        emit(this->keepAliveReceived_Signal, this->localPeerId);
        break;
    case PW_HANDSHAKE_MSG:
        emit(this->handshakeReceived_Signal, this->localPeerId);
        break;
    }
}
void BitTorrentClient::emitSentSignal(int messageId) {
    switch (messageId) {
    case PW_CHOKE_MSG:
        emit(this->chokeSent_Signal, this->localPeerId);
        break;
    case PW_UNCHOKE_MSG:
        emit(this->unchokeSent_Signal, this->localPeerId);
        break;
    case PW_INTERESTED_MSG:
        emit(this->interestedSent_Signal, this->localPeerId);
        break;
    case PW_NOT_INTERESTED_MSG:
        emit(this->notInterestedSent_Signal, this->localPeerId);
        break;
    case PW_HAVE_MSG:
        emit(this->haveSent_Signal, this->localPeerId);
        break;
    case PW_BITFIELD_MSG:
        emit(this->bitFieldSent_Signal, this->localPeerId);
        break;
    case PW_REQUEST_MSG:
        emit(this->requestSent_Signal, this->localPeerId);
        break;
    case PW_PIECE_MSG:
        emit(this->pieceSent_Signal, this->localPeerId);
        break;
    case PW_CANCEL_MSG:
        emit(this->cancelSent_Signal, this->localPeerId);
        break;
    case PW_KEEP_ALIVE_MSG:
        emit(this->keepAliveSent_Signal, this->localPeerId);
        break;
    case PW_HANDSHAKE_MSG:
        emit(this->handshakeSent_Signal, this->localPeerId);
        break;
    }
}
Swarm & BitTorrentClient::getSwarm(int infoHash) {
    try {
        return this->swarmMap.at(infoHash);
    } catch (std::out_of_range & e) {
        std::ostringstream out;
        out << "Swarm " << infoHash << " not found";
        throw std::logic_error(out.str());
    }
}
Swarm const& BitTorrentClient::getSwarm(int infoHash) const {
    try {
        return this->swarmMap.at(infoHash);
    } catch (std::out_of_range & e) {
        std::ostringstream out;
        out << "Swarm " << infoHash << " not found";
        throw std::logic_error(out.str());
    }
}
PeerEntry & BitTorrentClient::getPeerEntry(int infoHash, int peerId) {
    try {
        PeerMap & peerMap = this->getSwarm(infoHash).get<2>();
        return peerMap.at(peerId);
    } catch (std::out_of_range & e) {
        std::ostringstream out;
        out << "Peer " << peerId << " not in swarm " << infoHash;
        throw std::logic_error(out.str());
    } catch (std::logic_error & e) {
        throw e;
    }

}
void BitTorrentClient::openListeningSocket() {
    // only open the socket if it was already closed.
    if (this->serverSocket.getState() == TCPSocket::CLOSED) {
        this->serverSocket.renewSocket();

        // bind the socket to the same address and port.
        this->serverSocket.bind(this->localIp, this->localPort);
        this->serverSocket.listen();

        this->printDebugMsg("opened listening socket.");
    }
}

void BitTorrentClient::peerWireStatistics(cMessage const*msg, bool sending =
        false) {
    if (dynamic_cast<PeerWireMsg const*>(msg)) {
        PeerWireMsg const* peerWireMsg = static_cast<PeerWireMsg const*>(msg);
        int messageId = peerWireMsg->getMessageId();

        // bytes of useful content
        int content = 0;
        // bytes used by the PeerWire protocol
        int size = peerWireMsg->getByteLength();

        if (messageId == PW_PIECE_MSG) {
            content = static_cast<PieceMsg const*>(peerWireMsg)->getBlockSize();
            size -= content;
        }

        if (sending) {
            this->emitSentSignal(messageId);
            emit(this->contentBytesSent_Signal, content);
            emit(this->peerWireBytesSent_Signal, size);
        } else {
            this->emitReceivedSignal(messageId);
            emit(this->contentBytesReceived_Signal, content);
            emit(this->peerWireBytesReceived_Signal, size);
        }
    } else if (dynamic_cast<PeerWireMsgBundle const*>(msg)) {
        cQueue const& bundle =
                static_cast<PeerWireMsgBundle const*>(msg)->getBundle();
        for (int i = 0; i < bundle.length(); ++i) {
            PeerWireMsg const* peerWireMsg =
                    static_cast<PeerWireMsg const*>(bundle.get(i));
            int messageId = peerWireMsg->getMessageId();
            int size = peerWireMsg->getByteLength();

            // PieceMsg are not bundled ever
            if (sending) {
                this->emitSentSignal(messageId);
                emit(this->peerWireBytesSent_Signal, size);
            } else {
                this->emitReceivedSignal(messageId);
                emit(this->peerWireBytesReceived_Signal, size);
            }
        }
    }
}
void BitTorrentClient::printDebugMsg(std::string s) const {
    if (this->debugFlag) {
        // debug "header"
        std::cerr << simulation.getEventNumber() << " (T=";
        std::cerr << simulation.getSimTime() << ")(BitTorrentClient) - ";
        std::cerr << "Peer " << this->localPeerId << ": ";
        std::cerr << s << "\n";
    }
}

void BitTorrentClient::printDebugMsgConnections(std::string methodName,
        int infoHash, Swarm const&swarm) const {
    std::ostringstream out;

    out << methodName << "(" << infoHash << "): ";
    out << " Num Active=" << swarm.get<0>();
    out << " Num Passive=" << swarm.get<1>();
    out << " PeerMap size=" << swarm.get<2>().size();
    this->printDebugMsg(out.str());
}

void BitTorrentClient::removeThread(PeerWireThread *thread) {
    if (*this->threadInProcessingIt == thread) {
        // remove while keeping the iterator valid
        this->allThreads.erase(this->threadInProcessingIt++);

        // go back one element, if possible, since processNextThread will
        // increment the iterator
        if (!this->allThreads.empty()
                && this->threadInProcessingIt != this->allThreads.begin()) {
            --this->threadInProcessingIt;
        }
    } else {
        this->allThreads.erase(thread);
    }
    // remove socket and delete thread.
    TCPSrvHostApp::removeThread(thread);
    thread = NULL;
}
void BitTorrentClient::registerEmittedSignals() {
    // signal that this Client entered a swarm
    this->numUnconnected_Signal = registerSignal(
            "BitTorrentClient_NumUnconnected");
    this->numConnected_Signal = registerSignal("BitTorrentClient_NumConnected");

    this->processingTime = registerSignal("BitTorrentClient_ProcessingTime");

    // TODO use these signals
    this->peerWireBytesSent_Signal = registerSignal(
            "BitTorrentClient_PeerWireBytesSent");
    this->peerWireBytesReceived_Signal = registerSignal(
            "BitTorrentClient_PeerWireBytesReceived");
    this->contentBytesSent_Signal = registerSignal(
            "BitTorrentClient_ContentBytesSent");
    this->contentBytesReceived_Signal = registerSignal(
            "BitTorrentClient_ContentBytesReceived");

    this->bitFieldSent_Signal = registerSignal("BitTorrentClient_BitFieldSent");
    this->bitFieldReceived_Signal = registerSignal(
            "BitTorrentClient_BitFieldReceived");
    this->cancelSent_Signal = registerSignal("BitTorrentClient_CancelSent");
    this->cancelReceived_Signal = registerSignal(
            "BitTorrentClient_CancelReceived");
    this->chokeSent_Signal = registerSignal("BitTorrentClient_ChokeSent");
    this->chokeReceived_Signal = registerSignal(
            "BitTorrentClient_ChokeReceived");
    this->handshakeSent_Signal = registerSignal(
            "BitTorrentClient_HandshakeSent");
    this->handshakeReceived_Signal = registerSignal(
            "BitTorrentClient_HandshakeReceived");
    this->haveSent_Signal = registerSignal("BitTorrentClient_HaveSent");
    this->haveReceived_Signal = registerSignal("BitTorrentClient_HaveReceived");
    this->interestedSent_Signal = registerSignal(
            "BitTorrentClient_InterestedSent");
    this->interestedReceived_Signal = registerSignal(
            "BitTorrentClient_InterestedReceived");
    this->keepAliveSent_Signal = registerSignal(
            "BitTorrentClient_KeepAliveSent");
    this->keepAliveReceived_Signal = registerSignal(
            "BitTorrentClient_KeepAliveReceived");
    this->notInterestedSent_Signal = registerSignal(
            "BitTorrentClient_NotInterestedSent");
    this->notInterestedReceived_Signal = registerSignal(
            "BitTorrentClient_NotInterestedReceived");
    this->pieceSent_Signal = registerSignal("BitTorrentClient_PieceSent");
    this->pieceReceived_Signal = registerSignal(
            "BitTorrentClient_PieceReceived");
    this->requestSent_Signal = registerSignal("BitTorrentClient_RequestSent");
    this->requestReceived_Signal = registerSignal(
            "BitTorrentClient_RequestReceived");
    this->unchokeSent_Signal = registerSignal("BitTorrentClient_UnchokeSent");
    this->unchokeReceived_Signal = registerSignal(
            "BitTorrentClient_UnchokeReceived");
}
void BitTorrentClient::subscribeToSignals() {
}
// Protected methods
int BitTorrentClient::numInitStages() const {
    return 4;
}
void BitTorrentClient::finish() {
    //    this->doubleProcessingTimeHist.record();
}
void BitTorrentClient::initialize(int stage) {
    if (stage == 0) {
        this->registerEmittedSignals();
    } else if (stage == 3) {
        // create a listening connection
        TCPSrvHostApp::initialize();

        cModule* swarmManager = getParentModule()->getSubmodule("swarmManager");
        if (swarmManager == NULL) {
            throw cException("SwarmManager module not found");
        }

        this->swarmManager = check_and_cast<SwarmManager*>(swarmManager);
        // TODO get from ClientController
        this->localPeerId = getParentModule()->getParentModule()->getId();

        // get parameters from the modules
        this->snubbedInterval = par("snubbedInterval");
        this->timeoutInterval = par("timeoutInterval");
        this->keepAliveInterval = par("keepAliveInterval");
        //        this->oldUnchokeInterval = par("oldUnchokeInterval");
        this->downloadRateInterval = par("downloadRateInterval");
        this->uploadRateInterval = par("uploadRateInterval");
        this->globalNumberOfPeers = par("globalNumberOfPeers");
        this->numberOfActivePeers = par("numberOfActivePeers");
        this->numberOfPassivePeers = par("numberOfPassivePeers");

        IPAddressResolver resolver;
        this->localIp = resolver.addressOf(getParentModule()->getParentModule(),
                IPAddressResolver::ADDR_PREFER_IPv4);
        this->localPort = par("port");

        this->debugFlag = par("debugFlag").boolValue();

        char const* histogramFileName = par("processingTimeHistogram");
        FILE * histogramFile = fopen(histogramFileName, "r");
        if (histogramFile == NULL) {
            throw std::invalid_argument("Histogram file not found");
        } else {
            this->doubleProcessingTimeHist.loadFromFile(histogramFile);
            fclose(histogramFile);
        }
    }
}

/*!
 * This method is a copy of the TCPSrvHostApp::handleMessage method, but with
 * one major difference: this method ignores PEER_CLOSED messages that arrive
 * without a corresponding socket.
 *
 * This is done because every time the TCP sm transitions from FIN-WAIT-2 to
 * TIME-WAIT as a response to the FIN sent by the peer (check out TCP state
 * machine), the TCP module sends to the application a PEER_CLOSED message, in
 * addition to the CLOSED message, which is the only message expected.
 *
 * Therefore, when the application receives the CLOSED message, it deletes the
 * corresponding thread and socket through the closed() callback method. Right
 * after that, the PEER_CLOSED message arrives, and since the associated socket
 * doesn't exist anymore, a new socket (with state ESTABLISHED) is created and
 * its closed() method is executed inside the peerClosed() callback method. This
 * sends a CLOSE event to the TCP and an error occurs, because this connection
 * is already being closed.
 *
 * FIXME is this the right way to do this?
 */
void BitTorrentClient::handleMessage(cMessage* msg) {
    if (!msg->isSelfMessage()) {
        // message arrived after the processing time, process as it normally would
        TCPSocket *socket = socketMap.findSocketFor(msg);

        if (socket == NULL) {
            if (msg->getKind() == TCP_I_ESTABLISHED) {
                // new connection, create socket and process message
                socket = new TCPSocket(msg);
                socket->setOutputGate(gate("tcpOut"));

                PeerWireThread *proc = new PeerWireThread();
                // save the pointer to proc so it can be properly disposed at the end of the
                // simulation.
                this->allThreads.insert(proc);
                if (this->allThreads.size() == 1) {
                    // there was no thread before this insertion, so no processing was being
                    // made. Set the iterator to point to the beginning of the set
                    this->threadInProcessingIt = this->allThreads.begin();
                }

                // attaches the callback obj to the socket
                socket->setCallbackObject(proc);
                proc->init(this, socket);

                this->socketMap.addSocket(socket);

                //update display string
                this->updateDisplay();
            } else {
                // ignore message
                std::ostringstream out;
                out << "Message " << msg->getName()
                        << " don't belong to any socket";

                delete msg;
                msg = NULL;
                throw std::logic_error(out.str());
            }
        }

        // statistics about the PeerWireMsgs
        // TODO
        this->peerWireStatistics(msg);
        socket->processMessage(msg);
    } else {
        if (msg == &this->endOfProcessingTimer) {
            (*this->threadInProcessingIt)->finishProcessing();
            if (!this->allThreads.empty()) {
                this->processNextThread();
            }
        } else {
            // PeerWireThread self-messages
            PeerWireThread *thread =
                    static_cast<PeerWireThread *>(msg->getContextPointer());

            assert(thread != NULL);
            // pass the message along to the thread
            thread->timerExpired(msg);
        }
    }
}
