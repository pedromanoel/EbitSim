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

#include "SwarmManager.h"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <IPAddressResolver.h>

#include "AnnounceRequestMsg_m.h"
#include "AnnounceResponseMsg_m.h"

#include "BitTorrentClient.h"
#include "Choker.h"
#include "ContentManager.h"
#include "UserCommand_m.h"

// Dumb fix because of the CDT parser (https://bugs.eclipse.org/bugs/show_bug.cgi?id=332278)
#ifdef __CDT_PARSER__
#undef BOOST_FOREACH
#define BOOST_FOREACH(a, b) for(a; b; )
#endif

namespace {
std::string toStr(int i) {
    return boost::lexical_cast<std::string>(i);
}

std::string getEventStr(AnnounceType type) {
    std::string evStr;
#define CASE(X) case X: evStr = #X; break
    switch (type) {
    CASE(A_STARTED);
    CASE(A_STOPPED);
    CASE(A_COMPLETED);
    CASE(A_NORMAL);
    default:
        evStr = "Strange";
        break;
    }
#undef CASE
    return evStr;
}
}

// Private class SwarmModules
/*!
 * The callback class for the Tracker announces
 */
class SwarmManager::TrackerSocketCallback: public TCPSocket::CallbackInterface {
private:
    SwarmManager * parent;

    //! True if entering as a seeder.
    bool seeder;
    bool pending;

    AnnounceType lastAnnounceType;

    TCPSocket* socket;
    IPvXAddress trackerAddress;
    int trackerPort;
    AnnounceRequestMsg announceRequestBase;
    //! Timer to count the minimum interval between requests
    cMessage minimumAnnounceTimer;
    //! Timer to count the normal interval between requests
    cMessage normalAnnounceTimer;
public:
    TorrentMetadata torrent;

    /*!
     * Create and start the modules ContentManager and Choker
     *
     * @param parent The parent module of the ContentManager and Choker.
     * @param socket The socket object used to contact the Tracker.
     * @param bitTorrentClient The module that connects to the Peers returned by
     * the Tracker.
     * @param announceRequestBase The message which generates the announces to
     * the Tracker.
     * @param trackerAddress The IP address of the Tracker.
     * @param trackerPort The open port of the Tracker.
     * @param torrent The torrent file used to initialize the ContentManager
     * module.
     * @param seeder True if the ContentManager should initialize with all of
     * the pieces available.
     * @param debugFlag True if the ContentManager and Choker modules should
     * print debug messages.
     */
    TrackerSocketCallback(SwarmManager * parent, TorrentMetadata const& torrent,
        bool seeder, TCPSocket* socket, IPvXAddress const& trackerAddress,
        int trackerPort) :
        torrent(torrent), parent(parent), seeder(seeder), pending(false), //
        socket(socket), lastAnnounceType(A_STARTED), //
        trackerAddress(trackerAddress), trackerPort(trackerPort), //
        announceRequestBase("Announce Request"), //
        normalAnnounceTimer("NormalAnnounceTimer"), //
        minimumAnnounceTimer("MinimumAnnounceTimer") {

        this->announceRequestBase.setInfoHash(this->torrent.infoHash);
        this->announceRequestBase.setNumWant(this->parent->numWant);
        this->announceRequestBase.setPeerId(this->parent->localPeerId);
        this->announceRequestBase.setPort(this->parent->localPort);
        this->announceRequestBase.setDownloaded(0);
        this->announceRequestBase.setUploaded(0);
        this->announceRequestBase.setByteLength(
            this->announceRequestBase.getMessageLength());

        this->normalAnnounceTimer.setContextPointer(this);
        this->minimumAnnounceTimer.setContextPointer(this);
    }
    ~TrackerSocketCallback() {
        delete socket;
        // cancel the periodic announce timer
        this->parent->cancelEvent(&this->normalAnnounceTimer);
        this->parent->cancelEvent(&this->minimumAnnounceTimer);
    }
    //!@name TCPSocket::CallbackInterface virtual methods
    //@{
    void socketDataArrived(int connId, void *yourPtr, cPacket *msg,
        bool urgent) {
        // If stopping, don't send the response to the peer.
        if (this->lastAnnounceType != A_STOPPED) {
            AnnounceResponseMsg* response =
                check_and_cast<AnnounceResponseMsg*>(msg);
            // get list of Peers returned by the tracker and tell the
            // BitTorrentClient to connect with them.
            std::list<PeerConnInfo> peers;
            for (unsigned int i = 0; i < response->getPeersArraySize(); ++i) {
                PeerInfo& info = response->getPeers(i);
                PeerConnInfo peer = make_tuple(info.getPeerId(), info.getIp(),
                    info.getPort());
                peers.push_back(peer);
            }
            if (peers.size()) {
                this->parent->bitTorrentClient->addUnconnectedPeers(
                    this->torrent.infoHash, peers);
            }
        }
        delete msg;
        msg = NULL;
    }
    void socketPeerClosed(int connId, void *yourPtr) {
        this->socket->close(); // close the connection locally
    }
    void socketClosed(int connId, void *yourPtr) {
        // Renewing the socket changes the connid, so remove from the socketMap
        this->parent->socketMap.removeSocket(this->socket);
        // make the socket ready to connect again
        this->socket->renewSocket();

        switch (this->lastAnnounceType) {
        case A_COMPLETED: {
            // Check if the peer will leave the swarm after completed
            if (uniform(0, 1) > parent->par("remainingSeeders").doubleValue()) {
                cMessage * leaveMsg = new cMessage("Leave");
                leaveMsg->setContextPointer(this);
                this->parent->scheduleAt(simTime(), leaveMsg);
            }
        }
            break;
        case A_STOPPED: {
            // Exiting the swarm, delete the callback object
            cMessage * deleteMsg = new cMessage("Delete");
            deleteMsg->setContextPointer(this);
            this->parent->scheduleAt(simTime(), deleteMsg);
        }
            break;
        case A_STARTED: /* empty case */
        case A_NORMAL: /* empty case */
            // Schedule the next announce only if leeching, since the seeder
            // won't actively connect
            if (!this->seeder) {
                this->pending = false; // the request has been responded

                // Stop and start the timers
                simtime_t nextMin = simTime(); // next minimum timeout
                simtime_t nextNormal = simTime(); // next normaltimeout
                nextMin += this->parent->minimumRefreshInterval;
                nextNormal += this->parent->normalRefreshInterval;
                this->parent->cancelEvent(&this->minimumAnnounceTimer);
                this->parent->cancelEvent(&this->normalAnnounceTimer);
                this->parent->scheduleAt(nextMin, &this->minimumAnnounceTimer);
                this->parent->scheduleAt(nextNormal,
                    &this->normalAnnounceTimer);
            }
            break;
        }
    }
    //@}

    /*!
     * Send the announce if the conditions are met. Called directly or when one
     * of the timers expire.
     * @param type The type of the announce to be sent to the Tracker.
     */
    void sendAnnounce(AnnounceType type) {
        // Don't connect if the socket is already connected
        if (this->socket->getState() == TCPSocket::NOT_BOUND) {

            bool sendAnnounce = false;
            // If not a A_NORMAL announce, OR
            // if the minimum timer is not scheduled, send the announce
            sendAnnounce = (type != A_NORMAL)
                || !this->minimumAnnounceTimer.isScheduled();

            if (sendAnnounce && !this->pending) {
                // Don't send the next announce until this one has been responded
                this->pending = true;
                std::string const& typeStr = getEventStr(type);
#ifdef DEBUG_MSG
                this->parent->printDebugMsg(
                    typeStr + " "
                        + toStr(this->announceRequestBase.getInfoHash()));
#endif
                this->lastAnnounceType = type;
                // Connect the socket and send a new announce request
                AnnounceRequestMsg *announceRequest = announceRequestBase.dup();
                announceRequest->setEvent(type);

                this->parent->socketMap.addSocket(this->socket);
                this->socket->connect(this->trackerAddress, this->trackerPort);
                this->socket->send(announceRequest);
            } else {
                // The next announce will have this type
                this->lastAnnounceType = type;
#ifdef DEBUG_MSG
                this->parent->printDebugMsg("Don't send the announce");
#endif
            }
        }
    }
};

Define_Module(SwarmManager);
// Public methods
SwarmManager::SwarmManager() :
    localPeerId(-1), debugFlag(false), numWant(0), localPort(-1), normalRefreshInterval(
        0), minimumRefreshInterval(0) {
}
SwarmManager::~SwarmManager() {
    typedef std::pair<int, TrackerSocketCallback *> map_t;

    BOOST_FOREACH(map_t const& m, this->callbacksByInfoHash) {
        delete m.second;
    }
}

void SwarmManager::askMorePeers(int infoHash) {
    Enter_Method("askMorePeers(%d)", infoHash);
#ifdef DEBUG_MSG
    this->printDebugMsg("Ask for more peers for swarm " + toStr(infoHash));
#endif
    TrackerSocketCallback * socketCallback = this->callbacksByInfoHash.at(
        infoHash);
    socketCallback->sendAnnounce(A_NORMAL);

}
void SwarmManager::finishedDownload(int infoHash) {
    // tell simulator that this method is being called.
    Enter_Method("finishedDownload(%d)", infoHash);
#ifdef DEBUG_MSG
    this->printDebugMsg("Finished download for swarm " + toStr(infoHash));
#endif

    TrackerSocketCallback * socketCallback = this->callbacksByInfoHash.at(
        infoHash);
    socketCallback->sendAnnounce(A_COMPLETED);
}

// Private methods
// Tracker communication methods
void SwarmManager::enterSwarm(TorrentMetadata const& torrent, bool seeder,
    IPvXAddress const& trackerAddress, int trackerPort) {

    // The swarm must be new
    assert(!this->callbacksByInfoHash.count(torrent.infoHash));

    // Create the socket and the callback objects
    TCPSocket * socket = new TCPSocket();
    socket->setOutputGate(gate("tcpOut"));

    TrackerSocketCallback * socketCallback = new TrackerSocketCallback(this,
        torrent, seeder, socket, trackerAddress, trackerPort);
    socket->setCallbackObject(socketCallback);

    socketCallback->sendAnnounce(A_STARTED);

    // Create a new SwarmModules object and insert it in the SwarmModules map.
    this->callbacksByInfoHash[torrent.infoHash] = socketCallback;

    this->bitTorrentClient->createSwarm(torrent.infoHash, torrent.numOfPieces,
        torrent.numOfSubPieces, torrent.subPieceSize, seeder);
    emit(this->enterSwarmSignal, simTime());
    emit(this->emittedPeerId_Signal, this->localPeerId);
}
void SwarmManager::leaveSwarm(int infoHash) {
    this->callbacksByInfoHash.at(infoHash)->sendAnnounce(A_STOPPED);
    // remove the swarm from the application, closing all connections
    // with other peers
    this->bitTorrentClient->deleteSwarm(infoHash);
    delete this->callbacksByInfoHash.at(infoHash);
    this->callbacksByInfoHash.erase(infoHash);
    emit(this->leaveSwarmSignal, simTime());
    emit(this->emittedPeerId_Signal, this->localPeerId);
}

// Handle message methods
void SwarmManager::treatUserCommand(cMessage * msg) {
    cObject * controlInfo = msg->getControlInfo();
    if (msg->getKind() == USER_COMMAND_ENTER_SWARM) {
        EnterSwarmCommand * enterSwarmMsg = check_and_cast<EnterSwarmCommand *>(
            controlInfo);
        this->enterSwarm(enterSwarmMsg->getTorrentMetadata(),
            enterSwarmMsg->getSeeder(), enterSwarmMsg->getTrackerAddress(),
            enterSwarmMsg->getTrackerPort());
    } else {
        throw cException("Bad user command");
    }
    delete msg;
}
void SwarmManager::treatSelfMessages(cMessage *msg) {
    TrackerSocketCallback * socketCallback =
        static_cast<TrackerSocketCallback *>(msg->getContextPointer());
    if (msg->isName("Leave")) {
        this->leaveSwarm(socketCallback->torrent.infoHash);
        delete msg;
    } else if (msg->isName("Delete")) {
        delete socketCallback;
    } else if (msg->isName("NormalAnnounceTimer")
        || msg->isName("MinimumAnnounceTimer")) {
        socketCallback->sendAnnounce(A_NORMAL);
        // don't delete the message, because it is reusable
    }
}
void SwarmManager::treatTCPMessage(cMessage * msg) {
// This module is not open to connections, so the socket should exist.
    TCPSocket * socket = this->socketMap.findSocketFor(msg);
    assert(socket); // Socket should exist
    socket->processMessage(msg);
}

void SwarmManager::printDebugMsg(std::string s) {
#ifdef DEBUG_MSG
    if (this->debugFlag) {
        // debug "header"
        std::cerr << simulation.getEventNumber();
        std::cerr << ";" << simulation.getSimTime();
        std::cerr << ";(smanager);Peer " << this->localPeerId << ";";
        std::cerr << s << "\n";
    }
#endif
}
void SwarmManager::setStatusString(const char * s) {
    if (ev.isGUI()) {
        getDisplayString().setTagArg("t", 0, s);
    }
}

// Protected methods
void SwarmManager::handleMessage(cMessage *msg) {
    if (msg->arrivedOn("userCommand")) {
        this->treatUserCommand(msg);
    } else if (msg->arrivedOn("tcpIn")) {
        this->treatTCPMessage(msg);
    } else {
        this->treatSelfMessages(msg);
    }
}
void SwarmManager::initialize() {
    registerEmittedSignals();
    cModule* bitTorrentClient = getParentModule()->getSubmodule(
        "bitTorrentClient");
    if (bitTorrentClient == NULL) {
        throw cException("BitTorrentClient module not found");
    }
    this->bitTorrentClient = check_and_cast<BitTorrentClient*>(
        bitTorrentClient);

    // Make the peerId equal to the module id, which is unique throughout the simulation.
    this->localPeerId = this->getParentModule()->getParentModule()->getId();

    // Get parameters from ned file
    this->numWant = par("numWant").longValue();
    this->localPort = bitTorrentClient->par("port").longValue();
    this->normalRefreshInterval = par("normalRefreshInterval").doubleValue();
    this->minimumRefreshInterval = par("minimumRefreshInterval").doubleValue();

    if (this->numWant == 0) {
        throw cException("numWant must be bigger than 0s");
    }
    if (this->normalRefreshInterval == 0) {
        throw cException("normalRefreshInterval must be bigger than 0s");
    }
    if (this->minimumRefreshInterval == 0) {
        throw cException("normalRefreshInterval must be bigger than 0s");
    }
    if (this->normalRefreshInterval < this->minimumRefreshInterval) {
        throw cException(
            "normalRefreshInterval must be bigger than minimumRefreshInterval");
    }

    this->debugFlag = par("debugFlag").boolValue();
}

void SwarmManager::registerEmittedSignals() {
    // Configure signals
    this->downloadRateSignal = registerSignal("SwarmManager_DownloadRate");
    this->uploadRateSignal = registerSignal("SwarmManager_UploadRate");
    this->enterSwarmSignal = registerSignal("SwarmManager_EnterSwarm");
    this->leaveSwarmSignal = registerSignal("SwarmManager_LeaveSwarm");
    this->emittedPeerId_Signal = registerSignal("SwarmManager_EmittedPeerId");
}
