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

#include <boost/lexical_cast.hpp>
#include <string>
#include <IPAddressResolver.h>

#include "AnnounceRequestMsg_m.h"

#include "BitTorrentClient.h"
#include "Choker.h"
#include "ContentManager.h"
#include "SwarmManagerThread.h"
#include "UserCommand_m.h"

// Private class SwarmModules
/*!
 * A simple container for the swarm-related modules and the related torrent
 * metadata
 */
class SwarmManager::SwarmModules {
public:
    ContentManager* contentManager;
    Choker* choker;
    TorrentMetadata torrent;

    //! Create the modules ContentManager and Choker as a child of parent.
    SwarmModules(TorrentMetadata const& torrent, bool seeder, bool debugFlag,
        cModule * parent) :
            torrent(torrent) {

        using boost::lexical_cast;
        using std::string;
        // create ContentManager module
        cModule* contentManager;
        cModuleType *contentManagerType = cModuleType::get("br.larc.usp.client."
            "ContentManager");

        string name_cm = "contentManager_"
            + lexical_cast<string>(torrent.infoHash);
        contentManager = contentManagerType->create(name_cm.c_str(), parent);
        contentManager->par("numOfPieces") = torrent.numOfPieces;
        contentManager->par("numOfSubPieces") = torrent.numOfSubPieces;
        contentManager->par("subPieceSize") = torrent.subPieceSize;
        contentManager->par("debugFlag") = debugFlag;
        contentManager->par("seeder") = seeder;
        contentManager->par("infoHash") = torrent.infoHash;
        contentManager->finalizeParameters();
        contentManager->buildInside();
        contentManager->scheduleStart(simTime());
        contentManager->callInitialize();

        // create Choker module
        cModule* choker;
        cModuleType *chokerManagerType = cModuleType::get("br.larc.usp.client."
            "Choker");
        string name_c = "choker_" + lexical_cast<string>(torrent.infoHash);
        choker = chokerManagerType->create(name_c.c_str(), parent);
        choker->par("debugFlag") = debugFlag;
        choker->par("infoHash") = torrent.infoHash;
        choker->par("seeder") = seeder;
        choker->finalizeParameters();
        choker->buildInside();
        choker->scheduleStart(simTime());
        choker->callInitialize();

        this->contentManager = static_cast<ContentManager*>(contentManager);
        this->choker = static_cast<Choker*>(choker);
    }
    ~SwarmModules() {
    }
};

Define_Module(SwarmManager);
// Public methods
SwarmManager::SwarmManager() :
        localPeerId(-1), debugFlag(false), numWant(0), port(-1), refreshInterval(
            0) {
}
SwarmManager::~SwarmManager() {
    this->socketMap.deleteSockets();
    // delete thread objects
    std::map<int, SwarmManagerThread *>::iterator threadIt;
    threadIt = this->swarmThreads.begin();
    for (; threadIt != this->swarmThreads.end(); ++threadIt) {
        delete threadIt->second;
        threadIt->second = NULL;
    }
}

void SwarmManager::finishedDownload(int infoHash) {
    // tell simulator that this method is being called.
    Enter_Method("finishedDownload()");

    // change the status of the swarm modules
    this->swarmModulesMap.at(infoHash).choker->par("seeder") = true;
    this->swarmModulesMap.at(infoHash).contentManager->par("seeder") = true;

    SwarmManagerThread * thread = this->swarmThreads.at(infoHash);
    thread->sendAnnounce(A_COMPLETED);
}

std::pair<Choker*, ContentManager*> SwarmManager::checkSwarm(int infoHash) {
    Enter_Method("checkSwarm(infoHash: %d)", infoHash);
    std::map<int, SwarmModules>::iterator it = this->swarmModulesMap.find(
        infoHash);
    Choker * choker = NULL;
    ContentManager * contentManager = NULL;
    if (it != this->swarmModulesMap.end()) {
        choker = it->second.choker;
        contentManager = it->second.contentManager;
    }
    return std::make_pair(choker, contentManager);
}
void SwarmManager::stopChoker(int infoHash) {
    Enter_Method("stopChoker(infoHash: %d)", infoHash);
    SwarmModules & SwarmModules = this->swarmModulesMap.at(infoHash);

    SwarmModules.choker->stopChoker();
}

// Private methods

// Tracker communication methods
void SwarmManager::enterSwarm(TorrentMetadata const& torrentMetadata,
    bool seeder, IPvXAddress const& trackerAddress, int trackerPort) {

    // Create a new SwarmModules object and insert it in the SwarmModules map.
    // If the swarm already exists, throw an exception.
    if (this->swarmModulesMap.count(torrentMetadata.infoHash)) {
        throw std::logic_error("Tried to create swarm, "
            "but it already exists.");
    }

    SwarmModules const& swarmModules = SwarmModules(torrentMetadata, seeder,
        this->subModulesDebugFlag, this);
    this->swarmModulesMap.insert(
        std::make_pair(torrentMetadata.infoHash, swarmModules));
    // start the swarm in the BitTorrentClient
    this->bitTorrentClient->addSwarm(torrentMetadata.infoHash, seeder);

    // create the announce message
    AnnounceRequestMsg announceRequest("Announce request");
    announceRequest.setInfoHash(torrentMetadata.infoHash);
    announceRequest.setDownloaded(0);
    announceRequest.setEvent(A_STARTED);
    announceRequest.setNumWant(numWant);
    announceRequest.setPeerId(this->localPeerId);
    announceRequest.setPort(this->port);
    announceRequest.setUploaded(0);

    // send the announce to acquire the list of peers for this swarm
    SwarmManagerThread * thread = new SwarmManagerThread(announceRequest,
        this->refreshInterval, seeder, trackerAddress, trackerPort);
    TCPSocket * socket = new TCPSocket();
    socket->setOutputGate(gate("tcpOut"));
    socket->setCallbackObject(thread);
    thread->init(this, socket);

    this->swarmThreads[torrentMetadata.infoHash] = thread;
    thread->sendAnnounce(A_STARTED);

    // FIXME put this where the response from the tracker arrives
    // entered the swarm for real
    // emit(this->enteredSwarmSignal, simTime());
    emit(this->enterSwarmSignal, simTime());
}

void SwarmManager::leaveSwarm(int infoHash) {
    SwarmManagerThread * thread = this->swarmThreads.at(infoHash);
    thread->sendAnnounce(A_STOPPED);
}

void SwarmManager::printDebugMsg(std::string s) {
    if (this->debugFlag) {
        // debug "header"
        std::cerr << simulation.getEventNumber() << " (T=";
        std::cerr << simulation.getSimTime() << ")(SwarmManager) - ";
        std::cerr << "Peer " << this->localPeerId << ": ";
        std::cerr << s << "\n";
    }
}
void SwarmManager::setStatusString(const char * s) {
    if (ev.isGUI()) {
        getDisplayString().setTagArg("t", 0, s);
    }
}

// Protected methods
void SwarmManager::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        TCPServerThreadBase * thread =
            static_cast<TCPServerThreadBase *>(msg->getContextPointer());

        switch (msg->getKind()) {
        case SwarmManagerThread::SELF_THREAD_DELETION:
            this->removeThread(thread);
            delete msg;
            msg = NULL;
            break;
        default:
            thread->timerExpired(msg);
            break;
        }
    } else if (msg->arrivedOn("tcpIn")) {
        // Only PEER_CLOSED is accepted without a socket
        TCPSocket *socket = this->socketMap.findSocketFor(msg);
        if (socket) {
            socket->processMessage(msg);
        } else if (msg->getKind() == TCPSocket::PEER_CLOSED) {
            delete msg;
        } else {
            delete msg;
            throw cException("The message must have a matching socket");
        }
    } else if (msg->arrivedOn("userCommand")) {
        cObject * controlInfo = msg->getControlInfo();
        switch (msg->getKind()) {
        case USER_COMMAND_ENTER_SWARM: {
            EnterSwarmCommand * enterSwarmMsg = check_and_cast<
                    EnterSwarmCommand *>(controlInfo);
            enterSwarm(enterSwarmMsg->getTorrentMetadata(),
                enterSwarmMsg->getSeeder(), enterSwarmMsg->getTrackerAddress(),
                enterSwarmMsg->getTrackerPort());
        }
            break;
        case USER_COMMAND_LEAVE_SWARM: {
            LeaveSwarmCommand * leaveSwarmMsg = check_and_cast<
                    LeaveSwarmCommand *>(controlInfo);
            leaveSwarm(leaveSwarmMsg->getInfoHash());
        }
            break;
        default:
            throw cException("Bad user command");
            break;
        }
        delete msg;
    }
}
void SwarmManager::initialize(int stage) {
    if (stage == 0) {
        registerEmittedSignals();
        cModule* bitTorrentClient = getParentModule()->getSubmodule(
            "bitTorrentClient");
        if (bitTorrentClient == NULL) {
            throw cException("BitTorrentClient module not found");
        }
        this->bitTorrentClient = check_and_cast<BitTorrentClient*>(
            bitTorrentClient);

        // TODO dataRateCollector

        // Make the peerId equal to the module id, which is unique throughout the simulation.
        this->localPeerId = this->getParentModule()->getParentModule()->getId();

        // Get parameters from ned file
        this->numWant = par("numWant").longValue();
        this->port = bitTorrentClient->par("port").longValue();
        this->refreshInterval = par("refreshInterval").doubleValue();

        if (this->numWant == 0) {
            throw cException("numWant must be bigger than 0s");
        }
        if (this->refreshInterval == 0) {
            throw cException("refreshInterval must be bigger than 0s");
        }

        this->debugFlag = par("debugFlag").boolValue();
        this->subModulesDebugFlag = par("subModulesDebugFlag").boolValue();
    }
}

int SwarmManager::numInitStages() const {
    return 4;
}

double SwarmManager::getDownloadRate() {
    //Since the peer has a only one interface, the gateId is 0
    //    double downloadRate = this->dataRateCollector->getDownloadRate(0);
    //    emit(this->downloadRateSignal, downloadRate);TODO
    double downloadRate = 0;
    return downloadRate;
}
double SwarmManager::getUploadRate() {
    //Since the peer has a only one interface, the gateId is 0
    //    double uploadRate = this->dataRateCollector->getUploadRate(0);
    //    emit(this->uploadRateSignal, uploadRate);TODO
    double uploadRate = 0;
    return uploadRate;
}
void SwarmManager::registerEmittedSignals() {
    // Configure signals
    this->downloadRateSignal = registerSignal("SwarmManager_DownloadRate");
    this->uploadRateSignal = registerSignal("SwarmManager_UploadRate");
    this->enterSwarmSignal = registerSignal("SwarmManager_EnterSwarm");
    this->enteredSwarmSignal = registerSignal("SwarmManager_EnteredSwarm");
}
