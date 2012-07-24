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
#include <iostream>
#include <IPAddressResolver.h>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include "Application_m.h"
#include "PeerWire_m.h"
#include "PeerWireMsgBundle_m.h"
#include "PeerWireThread.h"
#include "Choker.h"
#include "ContentManager.h"
#include "SwarmManager.h"

// Dumb fix because of the CDT parser (https://bugs.eclipse.org/bugs/show_bug.cgi?id=332278)
#ifdef __CDT_PARSER__
#undef BOOST_FOREACH
#define BOOST_FOREACH(a, b) for(a; b; )
#endif

// Helper functions
namespace {
std::string toStr(int i) {
    return boost::lexical_cast<std::string>(i);
}
}

Define_Module(BitTorrentClient);

// Public Methods
BitTorrentClient::BitTorrentClient() :
    swarmManager(NULL), endOfProcessingTimer("End of processing"), doubleProcessingTimeHist(
        "Piece processing time histogram"), snubbedInterval(0), timeoutInterval(
        0), keepAliveInterval(0), downloadRateInterval(0), uploadRateInterval(
        0), localPort(-1), localPeerId(-1), debugFlag(false), globalNumberOfPeers(
        0), numActiveConn(0), numPassiveConn(0), prevNumUnconnected(0), prevNumConnected(
        0) {
}

BitTorrentClient::~BitTorrentClient() {
    this->cancelEvent(&this->endOfProcessingTimer);
    // delete all sockets
//    this->socketMap.deleteSockets();
    // delete all threads, which are not deleted when the sockets are destroyed.
    BOOST_FOREACH(PeerWireThread * thread, this->allThreads) {
        delete thread;
    }

    typedef std::pair<int, Swarm> map_t;
    BOOST_FOREACH(map_t s, this->swarmMap) {
        // stop and delete the dynamic modules
        s.second.contentManager->callFinish();
        s.second.contentManager->deleteModule();
        s.second.choker->callFinish();
        s.second.choker->deleteModule();
    }
}

// Method used by the SwarmManager::SwarmModules
void BitTorrentClient::addUnconnectedPeers(int infoHash,
    UnconnectedList & peers) {
    Enter_Method("addUnconnectedPeers(infoHash: %d, qtty: %d)", infoHash,
        peers.size());
    Swarm & swarm = this->getSwarm(infoHash);

    // if seeding or closing, don't add unconnected peers
    if (!(swarm.seeding || swarm.closing)) {
        std::string out = "Adding " + toStr(peers.size()) + " peers";
        this->printDebugMsg(out);
        UnconnectedList & unconnectedList = swarm.unconnectedList;
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

// Methods used by the Choker
void BitTorrentClient::chokePeer(int infoHash, int peerId) {
    Enter_Method("chokePeer(infoHash: %d, peerId: %d)", infoHash, peerId);

    PeerStatus & peer = this->getPeerStatus(infoHash, peerId);
    if (peer.isUnchoked()) {
        peer.setUnchoked(false, simTime());
        peer.getThread()->sendApplicationMessage(APP_CHOKE_PEER);
    }
}
PeerVector BitTorrentClient::getFastestToDownload(int infoHash) const {
    Enter_Method("getFastestToDownload(infoHash: %d)", infoHash);
    Swarm const& swarm = this->getSwarm(infoHash);
    PeerMap const& peerMap = swarm.peerMap;
    PeerVector orderedPeers;
    orderedPeers.reserve(peerMap.size());
    if (peerMap.size()) {
        PeerMapConstIt it = peerMap.begin();
        for (; it != peerMap.end(); ++it) {
            orderedPeers.push_back(&it->second);
        }

        std::sort(orderedPeers.begin(), orderedPeers.end(),
            PeerStatus::sortByDownloadRate);
    }

    return orderedPeers;
}
PeerVector BitTorrentClient::getFastestToUpload(int infoHash) const {
    Enter_Method("getFastestToUpload(infoHash: %d)", infoHash);
    Swarm const& swarm = this->getSwarm(infoHash);
    PeerMap const& peerMap = swarm.peerMap;
    int peerMapSize = peerMap.size();
    PeerVector vector;
    if (peerMapSize) {
        vector.reserve(peerMapSize);
        PeerMapConstIt it = peerMap.begin();
        std::ostringstream out;
        out << "Connected peers: ";
        for (; it != peerMap.end(); ++it) {
            out << it->second.getPeerId() << " ";
            vector.push_back(&it->second);
        }
        this->printDebugMsg(out.str());

        std::sort(vector.begin(), vector.end(), PeerStatus::sortByUploadRate);
    }

    return vector;
}
void BitTorrentClient::unchokePeer(int infoHash, int peerId) {
    Enter_Method("unchokePeer(infoHash: %d, peerId: %d)", infoHash, peerId);

    PeerStatus & peer = this->getPeerStatus(infoHash, peerId);

    if (!peer.isUnchoked()) {
        peer.setUnchoked(true, simTime());
        peer.getThread()->sendApplicationMessage(APP_UNCHOKE_PEER);
    }
}

// Methods used by the ContentManager
void BitTorrentClient::closeConnection(int infoHash, int peerId) const {
    Enter_Method("closeConnection(infoHash: %d, peerId: %d)", infoHash, peerId);

    this->getPeerStatus(infoHash, peerId).getThread()->sendApplicationMessage(
        APP_CLOSE);
}
void BitTorrentClient::finishedDownload(int infoHash) {
    Enter_Method("finishedDownload(infoHash: %d)", infoHash);
    Swarm & swarm = this->getSwarm(infoHash);
    swarm.seeding = true;
    swarm.choker->par("seeder") = true;
    swarm.contentManager->par("seeder") = true;
    // no more active downloads
    swarm.unconnectedList.clear();
    this->swarmManager->finishedDownload(infoHash);
}
void BitTorrentClient::peerInteresting(int infoHash, int peerId) const {
    Enter_Method("peerInteresting(infoHash: %d, peerId: %d)", infoHash, peerId);
    this->getPeerStatus(infoHash, peerId).getThread()->sendApplicationMessage(
        APP_PEER_INTERESTING);
}
void BitTorrentClient::peerNotInteresting(int infoHash, int peerId) const {
    Enter_Method("peerNotInteresting(infoHash: %d, peerId: %d)", infoHash,
        peerId);
    this->getPeerStatus(infoHash, peerId).getThread()->sendApplicationMessage(
        APP_PEER_NOT_INTERESTING);
}
void BitTorrentClient::sendHaveMessages(int infoHash, int pieceIndex) const {
    Enter_Method("sendHaveMessage(pieceIndex: %d)", pieceIndex);
    Swarm const& swarm = this->getSwarm(infoHash);
    PeerMap const& peerMap = swarm.peerMap;
    PeerMapConstIt it = peerMap.begin();

    // Send have message to all peers from the swarm connected with the Client
    while (it != peerMap.end()) {
        std::string name = "HaveMsg (" + toStr(pieceIndex) + ")";
        HaveMsg* haveMsg = new HaveMsg(name.c_str());
        haveMsg->setIndex(pieceIndex);
        (it->second).getThread()->sendPeerWireMsg(haveMsg);
        ++it;
    }
}
void BitTorrentClient::sendPieceMessage(int infoHash, int peerId) const {
    Enter_Method("sendPieceMessage(infoHash: %d, peerId: %d)", infoHash,
        peerId);

    this->getPeerStatus(infoHash, peerId).getThread()->sendApplicationMessage(
        APP_SEND_PIECE_MSG);
}
double BitTorrentClient::updateDownloadRate(int infoHash, int peerId,
    unsigned long totalDownloaded) {
    double downloadRate = 0;
    PeerStatus & peer = this->getPeerStatus(infoHash, peerId);
    peer.setBytesDownloaded(simTime().dbl(), totalDownloaded);
    downloadRate = peer.getDownloadRate();
    return downloadRate;
}
double BitTorrentClient::updateUploadRate(int infoHash, int peerId,
    unsigned long totalUploaded) {
    double uploadRate = 0;
    PeerStatus & peer = this->getPeerStatus(infoHash, peerId);
    peer.setBytesUploaded(simTime().dbl(), totalUploaded);
    uploadRate = peer.getUploadRate();
    return uploadRate;
}

// Methods used by the SwarmManager
int BitTorrentClient::getLocalPeerId() const {
    return this->localPeerId;
}
void BitTorrentClient::createSwarm(int infoHash, int numOfPieces,
    int numOfSubPieces, int subPieceSize, bool newSwarmSeeding) {
    Enter_Method("addSwarm(infoHash: %d, %s)", infoHash,
        (newSwarmSeeding ? "seeding" : "leeching"));
    assert(!this->swarmMap.count(infoHash)); // Swarm must not exist

    // create Choker module
    cModule * choker;
    cModuleType *chokerManagerType = cModuleType::get("br.larc.usp.client."
        "Choker");
    std::string name_c = "choker_" + toStr(infoHash);
    choker = chokerManagerType->create(name_c.c_str(), this);
    choker->par("debugFlag") = this->subModulesDebugFlag;
    choker->par("infoHash") = infoHash;
    choker->par("seeder") = newSwarmSeeding;
    choker->finalizeParameters();
    choker->buildInside();
    choker->scheduleStart(simTime());
    choker->callInitialize();

    // create ContentManager module
    cModule * contentManager;
    cModuleType *contentManagerType = cModuleType::get("br.larc.usp.client."
        "ContentManager");
    std::string name_cm = "contentManager_" + toStr(infoHash);
    contentManager = contentManagerType->create(name_cm.c_str(), this);
    contentManager->par("numOfPieces") = numOfPieces;
    contentManager->par("numOfSubPieces") = numOfSubPieces;
    contentManager->par("subPieceSize") = subPieceSize;
    contentManager->par("debugFlag") = this->subModulesDebugFlag;
    contentManager->par("seeder") = newSwarmSeeding;
    contentManager->par("infoHash") = infoHash;
    contentManager->finalizeParameters();
    contentManager->buildInside();
    contentManager->scheduleStart(simTime());
    contentManager->callInitialize();

    // create swarm
    Swarm & swarm = this->swarmMap[infoHash];
    swarm.numActive = 0;
    swarm.numPassive = 0;
    swarm.seeding = newSwarmSeeding;
    swarm.closing = false;
    swarm.choker = static_cast<Choker*>(choker);
    swarm.contentManager = static_cast<ContentManager*>(contentManager);
}
void BitTorrentClient::deleteSwarm(int infoHash) {
    Enter_Method("removeSwarm(infoHash: %d)", infoHash);
    this->printDebugMsg("Leaving swarm");
    Swarm & swarm = this->getSwarm(infoHash);
    // Since it is not possible to delete the swarm before closing the threads,
    // set the closing flag to true and delete the swarm when all peers disconnect.
    swarm.closing = true;
    swarm.unconnectedList.clear();

    // close the connection with all the peers
    typedef std::pair<int, PeerStatus> map_t;
    BOOST_FOREACH(map_t const& peer, swarm.peerMap) {
        PeerWireThread * thread = peer.second.getThread();
        thread->sendApplicationMessage(APP_CLOSE);
    }
}

// Methods used by the PeerWireThread
void BitTorrentClient::addPeerToSwarm(int infoHash, int peerId,
    PeerWireThread* thread, bool active) {
    Swarm & swarm = this->getSwarm(infoHash);
    PeerMap & peerMap = swarm.peerMap;

    // if the number of connected peers changed, emit a signal
    if (this->prevNumConnected != peerMap.size()) {
        this->prevNumConnected = peerMap.size();
        emit(this->numConnected_Signal, this->prevNumConnected);
    }

    // The active connections are counted when they are initiated, but passive
    // connections cannot be known before the Handshake
    if (thread->activeConnection == false) {
        ++swarm.numPassive;
    }

    // Can't connect with the same Peer twice
    assert(!peerMap.count(peerId));
    peerMap.insert(std::make_pair(peerId, PeerStatus(peerId, thread)));

}
void BitTorrentClient::removePeerFromSwarm(int infoHash, int peerId, int connId,
    bool active) {
    Swarm & swarm = this->getSwarm(infoHash);
    // remove from the connected list
    PeerMap & peerMap = swarm.peerMap;
    peerMap.erase(peerId);
    // Remove the peer from the swarm modules
    // Added by contentManager->addPeerBitField() and choker->addPeer()
    swarm.contentManager->removePeerBitField(peerId);
    swarm.choker->removePeer(peerId);

    this->printDebugMsgConnections("removePeerInfo", infoHash, swarm);

    if (peerMap.empty()) {
        if (swarm.closing) {
            // stop and delete the dynamic modules, and delete the swarm
            swarm.contentManager->callFinish();
            swarm.contentManager->deleteModule();
            swarm.choker->callFinish();
            swarm.choker->deleteModule();
            this->swarmMap.erase(infoHash);

        } else {
            // There are no peers to unchoke
            swarm.choker->stopChokerRounds();
        }
    }
}
bool BitTorrentClient::canConnect(int infoHash, int peerId, bool active) const {
    bool successful = false;

    SwarmMapConstIt it = this->swarmMap.find(infoHash);
    if (it == this->swarmMap.end()) {
        std::string out = "No swarm " + toStr(infoHash) + " found";
        this->printDebugMsg(out);
    } else if (it->second.closing) {
        std::string out = "Swarm " + toStr(infoHash) + " closing";
        this->printDebugMsg(out);
    } else {
        Swarm const& swarm = it->second;
        PeerMap const& peerMap = swarm.peerMap;

        // can't connect if already in the PeerMap
        if (peerMap.count(peerId)) {
            std::string out = "Peer " + toStr(peerId) + " already connected";
            this->printDebugMsg(out);
        } else {
            // check if there is a free passive slot for connecting
            // if seeding, also count the active slots
            int availSlots = this->numPassiveConn - swarm.numPassive;
            if (swarm.seeding) {
                availSlots += this->numActiveConn - swarm.numActive;
            }

            // If active, already checked the slots at attemptActiveConnections()
            successful = active || availSlots > 0;

            if (active) {
                this->printDebugMsg("Accept active connection");
            } else if (availSlots > 0) {
                this->printDebugMsg("Can connect");
            } else {
                this->printDebugMsg("No slots available for connection");
            }
        }
    }
    return successful;
}

std::pair<Choker*, ContentManager*> BitTorrentClient::checkSwarm(int infoHash) {
    Choker * choker = NULL;
    ContentManager * contentManager = NULL;
    SwarmMapConstIt it = this->swarmMap.find(infoHash);
    if (it != this->swarmMap.end()) {
        choker = it->second.choker;
        contentManager = it->second.contentManager;
    }
    return std::make_pair(choker, contentManager);
}

void BitTorrentClient::processNextThread() {
    // allThreads will never be empty here
    assert(!this->allThreads.empty());

    std::list<PeerWireThread*>::iterator nextThreadIt =
        this->threadInProcessingIt;

    // don't start processing another thread if the current one is still processing
    if (!(*this->threadInProcessingIt)->busy
        && !this->endOfProcessingTimer.isScheduled()) {
        // increment the nextThreadIt until a thread with messages is found or
        // until a full circle is reached
        bool hasMessages = false;
        std::ostringstream out;
        out << "====== Next thread: ";
        out << (*nextThreadIt)->getThreadId();
        do {
            ++nextThreadIt;
            if (nextThreadIt == this->allThreads.end()) {
                nextThreadIt = this->allThreads.begin();
            }
            out << " > " << (*nextThreadIt)->getThreadId();
            hasMessages = (*nextThreadIt)->hasMessagesToProcess();
        } while (nextThreadIt != this->threadInProcessingIt && !hasMessages);

        out << " ======";
        this->printDebugMsg(out.str());

        if (hasMessages) {
            this->threadInProcessingIt = nextThreadIt;
            simtime_t processingTime =
                (*this->threadInProcessingIt)->startProcessing();
            emit(this->processingTime_Signal, processingTime);
            this->scheduleAt(simTime() + processingTime,
                &this->endOfProcessingTimer);
        } else {
            this->printDebugMsg("Idle");
        }
    }
}
void BitTorrentClient::setInterested(bool interested, int infoHash,
    int peerId) {
    this->getPeerStatus(infoHash, peerId).setInterested(interested);
}
void BitTorrentClient::setOldUnchoked(bool oldUnchoke, int infoHash,
    int peerId) {
    this->getPeerStatus(infoHash, peerId).setOldUnchoked(oldUnchoke);
}
void BitTorrentClient::setSnubbed(bool snubbed, int infoHash, int peerId) {
    this->getPeerStatus(infoHash, peerId).setSnubbed(snubbed);
}

// Private Methods
void BitTorrentClient::attemptActiveConnections(Swarm & swarm, int infoHash) {
    const PeerMap & peerMap = swarm.peerMap;
    // only make active connections if not seeding
    if (!(swarm.seeding || swarm.closing)) {
        UnconnectedList & unconnectedList = swarm.unconnectedList;

        if (unconnectedList.empty()) {
            this->swarmManager->askMorePeers(infoHash);
        } else {
            int numActiveSlost = this->numActiveConn - swarm.numActive;
            while (numActiveSlost && !unconnectedList.empty()) {
                PeerConnInfo peer = unconnectedList.front();
                // get the peerId
                int peerId = peer.get<0>();
                bool notConnected = !peerMap.count(peerId);
                bool notConnecting = this->activeConnectedPeers.count(
                    std::make_pair(infoHash, peerId)) == 0;
                // only connect if not already connecting or connected
                if (notConnected && notConnecting) {
                    --numActiveSlost; // decrease the number of slots
                    ++swarm.numActive; // increase the number of active connections
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

}
/*!
 * The Peer address is acquired from the Tracker.
 */
void BitTorrentClient::connect(int infoHash, PeerConnInfo const& peer) {
    // initialize variables with the tuple content
    int peerId, port;
    IPvXAddress ip;
    tie(peerId, ip, port) = peer;

    TCPSocket * socket = new TCPSocket();
    this->createThread(socket, infoHash, peerId);

    std::string out = "connId ";
    out += toStr(socket->getConnectionId());
    out += ", Opening connection with " + ip.str() + ":" + toStr(port);
    this->printDebugMsg(out);
    socket->connect(ip, port);
}
//void BitTorrentClient::closeListeningSocket() {
//    // If the socket isn't closed, close it.
//    if (this->serverSocket.getState() != TCPSocket::CLOSED) {
//        this->serverSocket.close();
//
//        this->printDebugMsg("closed listening socket.");
//    }
//}
void BitTorrentClient::emitReceivedSignal(int messageId) {
#define CASE( X, Y ) case (X): emit(Y, this->localPeerId); break;
    switch (messageId) {
    CASE(PW_CHOKE_MSG, this->chokeReceived_Signal)
    CASE(PW_UNCHOKE_MSG, this->unchokeReceived_Signal)
    CASE(PW_INTERESTED_MSG, this->interestedReceived_Signal)
    CASE(PW_NOT_INTERESTED_MSG, this->notInterestedReceived_Signal)
    CASE(PW_HAVE_MSG, this->haveReceived_Signal)
    CASE(PW_BITFIELD_MSG, this->bitFieldReceived_Signal)
    CASE(PW_REQUEST_MSG, this->requestReceived_Signal)
    CASE(PW_PIECE_MSG, this->pieceReceived_Signal)
    CASE(PW_CANCEL_MSG, this->cancelReceived_Signal)
    CASE(PW_KEEP_ALIVE_MSG, this->keepAliveReceived_Signal)
    CASE(PW_HANDSHAKE_MSG, this->handshakeReceived_Signal)
    }
#undef CASE
}
void BitTorrentClient::emitSentSignal(int messageId) {
#define CASE( X, Y ) case (X): emit(Y, this->localPeerId); break;
    switch (messageId) {
    CASE(PW_CHOKE_MSG, this->chokeSent_Signal)
    CASE(PW_UNCHOKE_MSG, this->unchokeSent_Signal)
    CASE(PW_INTERESTED_MSG, this->interestedSent_Signal)
    CASE(PW_NOT_INTERESTED_MSG, this->notInterestedSent_Signal)
    CASE(PW_HAVE_MSG, this->haveSent_Signal)
    CASE(PW_BITFIELD_MSG, this->bitFieldSent_Signal)
    CASE(PW_REQUEST_MSG, this->requestSent_Signal)
    CASE(PW_PIECE_MSG, this->pieceSent_Signal)
    CASE(PW_CANCEL_MSG, this->cancelSent_Signal)
    CASE(PW_KEEP_ALIVE_MSG, this->keepAliveSent_Signal)
    CASE(PW_HANDSHAKE_MSG, this->handshakeSent_Signal)
    }
#undef CASE
}
Swarm const& BitTorrentClient::getSwarm(int infoHash) const {
    assert(this->swarmMap.count(infoHash));
    return this->swarmMap.at(infoHash);
}
Swarm & BitTorrentClient::getSwarm(int infoHash) {
    assert(this->swarmMap.count(infoHash));
    return this->swarmMap.at(infoHash);
}
PeerStatus & BitTorrentClient::getPeerStatus(int infoHash, int peerId) {
    Swarm & swarm = this->getSwarm(infoHash);
    assert(swarm.peerMap.count(peerId));
    return swarm.peerMap.at(peerId);
}
PeerStatus const& BitTorrentClient::getPeerStatus(int infoHash,
    int peerId) const {
    Swarm const& swarm = this->getSwarm(infoHash);
    assert(swarm.peerMap.count(peerId));
    return swarm.peerMap.at(peerId);
}
//void BitTorrentClient::openListeningSocket() {
//    // only open the socket if it was already closed.
//    if (this->serverSocket.getState() == TCPSocket::CLOSED) {
//        this->serverSocket.renewSocket();
//
//        // bind the socket to the same address and port.
//        this->serverSocket.bind(this->localIp, this->localPort);
//        this->serverSocket.listen();
//
//        this->printDebugMsg("opened listening socket.");
//    }
//}

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
        cQueue const* bundle =
            static_cast<PeerWireMsgBundle const*>(msg)->getBundle();
        for (int i = 0; i < bundle->length(); ++i) {
            PeerWireMsg const* peerWireMsg =
                static_cast<PeerWireMsg const*>(bundle->get(i));
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
    out << " Num Active=" << swarm.numActive;
    out << " Num Passive=" << swarm.numPassive;
    out << " PeerMap size=" << swarm.peerMap.size();
    this->printDebugMsg(out.str());
}

void BitTorrentClient::createThread(TCPSocket * socket, int infoHash, int peerId) {
    socket->setOutputGate(gate("tcpOut"));
    assert((infoHash > 0 && peerId > 0) || (infoHash == -1 && peerId == -1));
    PeerWireThread *proc = new PeerWireThread(infoHash, peerId);
    // store proc here so it can be deleted at the destructor.
    this->allThreads.push_back(proc);
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

}
void BitTorrentClient::removeThread(PeerWireThread *thread) {
    if (*this->threadInProcessingIt == thread) {
        std::ostringstream out;
        out << "In processing: " << (void*) thread;
        thread->printDebugMsg(out.str());

        // Delete the current thread and set the iterator so that when
        // incremented, the thread after this one will be executed.
        // Steps        (1)    =>    (2)   =>   (3)    =>   (4)
        // Iterator  i         =>  i       =>        i =>      i
        // Set      |1|2|3|4|e => |2|3|4|e => |2|3|4|e => |2|3|4|e

        // (1) Iterator pointing to the current element
        this->allThreads.erase(this->threadInProcessingIt++);
        // (2) Erase current and keep the iterator valid by post-incrementing

        if (!this->allThreads.empty()) {
            // (3) If the iterator is begin(), send it to end()
            if (this->threadInProcessingIt == this->allThreads.begin()) {
                this->threadInProcessingIt = this->allThreads.end();
            }
            // (4) Decrement so that when incremented by processNextThread(),
            // the next thread is the one after the current one
            --this->threadInProcessingIt;
        }
    } else {
        this->printDebugMsg("Not processing");
        this->allThreads.remove(thread);
    }

    int infoHash = thread->infoHash;
    int peerId = thread->remotePeerId;

    SwarmMap::iterator it = this->swarmMap.find(infoHash);

    // The connection was established, so decrement the connection counter
    if (it != this->swarmMap.end()) {
        Swarm & swarm = it->second;
        // If the peer was actively connected, one active slot became available for
        // connection.
        if (this->activeConnectedPeers.erase(
            std::make_pair(infoHash, peerId))) {
            --swarm.numActive;
            this->attemptActiveConnections(swarm, infoHash);
        } else {
            --swarm.numPassive;
        }
    }

    // remove socket and delete thread.
    TCPSrvHostApp::removeThread(thread);
}
void BitTorrentClient::registerEmittedSignals() {
    // signal that this Client entered a swarm
#define SIGNAL(X, Y) this->X = registerSignal("BitTorrentClient_" #Y)
    SIGNAL(numUnconnected_Signal, NumUnconnected);
    SIGNAL(numConnected_Signal, NumConnected);
    SIGNAL(processingTime_Signal, ProcessingTime);

    // TODO use these signals
    SIGNAL(peerWireBytesSent_Signal, PeerWireBytesSent);
    SIGNAL(peerWireBytesReceived_Signal, PeerWireBytesReceived);
    SIGNAL(contentBytesSent_Signal, ContentBytesSent);
    SIGNAL(contentBytesReceived_Signal, ContentBytesReceived);

    SIGNAL(bitFieldSent_Signal, BitFieldSent);
    SIGNAL(bitFieldReceived_Signal, BitFieldReceived);
    SIGNAL(cancelSent_Signal, CancelSent);
    SIGNAL(cancelReceived_Signal, CancelReceived);
    SIGNAL(chokeSent_Signal, ChokeSent);
    SIGNAL(chokeReceived_Signal, ChokeReceived);
    SIGNAL(handshakeSent_Signal, HandshakeSent);
    SIGNAL(handshakeReceived_Signal, HandshakeReceived);
    SIGNAL(haveSent_Signal, HaveSent);
    SIGNAL(haveReceived_Signal, HaveReceived);
    SIGNAL(interestedSent_Signal, InterestedSent);
    SIGNAL(interestedReceived_Signal, InterestedReceived);
    SIGNAL(keepAliveSent_Signal, KeepAliveSent);
    SIGNAL(keepAliveReceived_Signal, KeepAliveReceived);
    SIGNAL(notInterestedSent_Signal, NotInterestedSent);
    SIGNAL(notInterestedReceived_Signal, NotInterestedReceived);
    SIGNAL(pieceSent_Signal, PieceSent);
    SIGNAL(pieceReceived_Signal, PieceReceived);
    SIGNAL(requestSent_Signal, RequestSent);
    SIGNAL(requestReceived_Signal, RequestReceived);
    SIGNAL(unchokeSent_Signal, UnchokeSent);
    SIGNAL(unchokeReceived_Signal, UnchokeReceived);
#undef SIGNAL
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

        this->swarmManager = check_and_cast<SwarmManager*>(
            getParentModule()->getSubmodule("swarmManager"));
        this->localPeerId = this->getParentModule()->getParentModule()->getId();

        // get parameters from the modules
        this->snubbedInterval = par("snubbedInterval");
        this->timeoutInterval = par("timeoutInterval");
        this->keepAliveInterval = par("keepAliveInterval");
        //        this->oldUnchokeInterval = par("oldUnchokeInterval");
        this->downloadRateInterval = par("downloadRateInterval");
        this->uploadRateInterval = par("uploadRateInterval");
        this->globalNumberOfPeers = par("globalNumberOfPeers"); //FIXME use this!
        this->numActiveConn = par("numberOfActivePeers");
        this->numPassiveConn = par("numberOfPassivePeers");

        IPAddressResolver resolver;
        this->localIp = resolver.addressOf(getParentModule()->getParentModule(),
            IPAddressResolver::ADDR_PREFER_IPv4);
        this->localPort = par("port");

        this->subModulesDebugFlag = par("subModulesDebugFlag").boolValue();
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
void BitTorrentClient::handleMessage(cMessage* msg) {
    if (!msg->isSelfMessage()) {
        // message arrived after the processing time, process as it normally would
        TCPSocket *socket = socketMap.findSocketFor(msg);

        if (socket == NULL) {
            if (msg->getKind() == TCP_I_ESTABLISHED) {
                // new connection, create socket and process message
                socket = new TCPSocket(msg);

                this->createThread(socket, -1, -1);
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
        this->peerWireStatistics(msg);
        socket->processMessage(msg);
    } else {
        if (msg == &this->endOfProcessingTimer) {
            (*this->threadInProcessingIt)->finishProcessing();
            if (!this->allThreads.empty()) {
                this->processNextThread();
            }
        } else if (msg->isName("Delete thread")) {
            this->printDebugMsg("Thread terminated.");
            PeerWireThread * thread = static_cast<PeerWireThread *>(msg->getContextPointer());
            this->removeThread(thread);
            delete msg;
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
