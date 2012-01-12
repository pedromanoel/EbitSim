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

#include <string>
#include <IPAddressResolver.h>

#include "BitTorrentClient.h"
#include "Choker.h"
#include "ContentManager.h"
#include "SwarmManagerThread.h"

// Private class SwarmEngine

class SwarmManager::SwarmEngine {
public:
	ContentManager* contentManager;
	Choker* choker;
	TorrentMetadata torrent;

	SwarmEngine(ContentManager* contentManager, Choker* choker,
			TorrentMetadata const& torrent) :
		contentManager(contentManager), choker(choker), torrent(torrent) {
	}
	~SwarmEngine() {
	}
};

Define_Module( SwarmManager)
;
// Public methods
SwarmManager::SwarmManager() :
	/*bitTorrentClient(NULL), connectedPeerManager(NULL), */localPeerId(-1),
			debugFlag(false), /*keepAlive("Announce"), */numWant(0), port(-1),
			refreshInterval(0), trackerPort(-1) {
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

// Tracker communication methods
//void TrackerClient::sendAnnounce() {
//    sendHttpRequest(A_NORMAL);
//}
void SwarmManager::enterSwarm(TorrentMetadata const& torrentInfo, bool seeder) {
	// tell simulator that this method is being called.
	Enter_Method("enterSwarm()");

	// create the swarm in the SwarmManager
	this->setSwarm(torrentInfo, seeder);
	emit(this->enterSwarmSignal, simTime());

	SwarmEngine const& swarmEngine =
			this->swarmEngines.at(torrentInfo.infoHash);
	// send the announce to acquire the list of peers for this swarm
	SwarmManagerThread * thread = new SwarmManagerThread(this->numWant,
			this->localPeerId, this->port, this->refreshInterval, torrentInfo, seeder);
	TCPSocket * socket = new TCPSocket();
	socket->setOutputGate(gate("tcpOut"));
	socket->setCallbackObject(thread);
	thread->init(this, socket);

	this->swarmThreads[torrentInfo.infoHash] = thread;
	thread->sendAnnounce(A_STARTED);
}
void SwarmManager::finishedDownload(int infoHash) {
	// tell simulator that this method is being called.
	Enter_Method("finishedDownload()");

	// change the status of the swarm modules
	this->swarmEngines.at(infoHash).choker->par("seeder") = true;
	this->swarmEngines.at(infoHash).contentManager->par("seeder") = true;

	SwarmManagerThread * thread = this->swarmThreads.at(infoHash);
	thread->sendAnnounce(A_COMPLETED);
}
void SwarmManager::leaveSwarm(int infoHash) {
	Enter_Method("leaveSwarm()");

	SwarmManagerThread * thread = this->swarmThreads.at(infoHash);
	thread->sendAnnounce(A_STOPPED);
}
std::pair<Choker*, ContentManager*> SwarmManager::checkSwarm(int infoHash) {
	Enter_Method("checkSwarm(infoHash: %d)", infoHash);
	std::map<int, SwarmEngine>::iterator it = this->swarmEngines.find(infoHash);
	Choker * choker = NULL;
	ContentManager * contentManager = NULL;
	if (it != this->swarmEngines.end()) {
		choker = it->second.choker;
		contentManager = it->second.contentManager;
	}
	return std::make_pair(choker, contentManager);
}
void SwarmManager::stopChoker(int infoHash) {
	Enter_Method("stopChoker(infoHash: %d)", infoHash);
	SwarmEngine & swarmEngine = this->swarmEngines.at(infoHash);

	swarmEngine.choker->stopChoker();
}

// Private methods
void SwarmManager::setSwarm(TorrentMetadata const& torrent, bool seeder) {
	// swarm don't exist
	if (!this->swarmEngines.count(torrent.infoHash)) {
		// entered the swarm for real
		emit(this->enteredSwarmSignal, simTime());

		// create ContentManager
		ContentManager* contentManager;
		cModuleType *contentManagerType = cModuleType::get(
				"br.larc.usp.iptv.client.ContentManager");
		{
			std::ostringstream name;
			name << "contentManager-" << torrent.infoHash;
			contentManager
					= static_cast<ContentManager*> (contentManagerType->create(
							name.str().c_str(), this));
		}
		// set up parameters and gate sizes before we set up its submodules
		contentManager->par("numOfPieces") = torrent.numOfPieces;
		contentManager->par("numOfSubPieces") = torrent.numOfSubPieces;
		contentManager->par("subPieceSize") = torrent.subPieceSize;
		contentManager->par("debugFlag") = this->subModulesDebugFlag;
		//contentManager->par("haveBundleSize"); default value
		//contentManager->par("requestBundleSize"); default value
		contentManager->par("seeder") = seeder;
		contentManager->par("infoHash") = torrent.infoHash;
		contentManager->finalizeParameters();
		contentManager->buildInside();
		contentManager->scheduleStart(simTime());
		contentManager->callInitialize();

		// create Choker
		Choker* choker;
		cModuleType *chokerManagerType = cModuleType::get(
				"br.larc.usp.iptv.client.Choker");
		{
			std::ostringstream name;
			name << "choker-" << torrent.infoHash;
			choker = static_cast<Choker*> (chokerManagerType->create(
					name.str().c_str(), this));
		}
		// default values for now.
		choker->par("debugFlag") = this->subModulesDebugFlag;
		choker->par("infoHash") = torrent.infoHash;
		choker->par("seeder") = seeder;
		choker->finalizeParameters();
		choker->buildInside();
		choker->scheduleStart(simTime());
		choker->callInitialize();

		// save the newly created modules
		SwarmEngine const& swarmEngine = SwarmEngine(contentManager, choker,
				torrent);
		this->swarmEngines.insert(std::make_pair(torrent.infoHash, swarmEngine));
		// create the swarm in the BitTorrentClient
		this->bitTorrentClient->addSwarm(torrent.infoHash, seeder);
	} else {
		// TODO can a Peer exit then reenter a swarm? If so, can it change from leecher to seeder or vice-versa?
		//        bool isContentManagerSeeder =
		//                this->swarmEngines[torrent.infoHash].contentManager->par(
		//                        "seeder").boolValue();
		//        if (seeder && !isContentManagerSeeder) {
		//            throw std::logic_error(
		//                    "Tried to create a seeder ContentManager, but ContentManager is already a leecher.");
		//        }
		throw std::logic_error("Tried to create swarm, but it already exists.");
	}
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
				static_cast<TCPServerThreadBase *> (msg->getContextPointer());

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
	} else {
		TCPSocket *socket = this->socketMap.findSocketFor(msg);
		if (!socket) {
			// the only message that should arrive without a socket is PEER_CLOSED
			if (msg->getKind() != TCPSocket::PEER_CLOSED) {
				throw std::logic_error(
						"Socket should exist. This module don't accept connections.");
			} else {
				delete msg;
			}
		} else {
			socket->processMessage(msg);
		}
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
		this->bitTorrentClient = check_and_cast<BitTorrentClient*> (
				bitTorrentClient);

		//        cModule* dataRateCollector =TODO
		//                getParentModule()->getParentModule()->getSubmodule(
		//                        "dataRateCollector");
		//        if (dataRateCollector == NULL) {
		//            throw cException("DataRateCollector module not found");
		//        }
		//        this->dataRateCollector = check_and_cast<DataRateCollector*> (
		//                dataRateCollector);

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
	} else if (stage == 3) {
		cModule * tracker = simulation.getModuleByPath(par("connectAddress"));
		if (tracker == NULL) {
			throw cException("Tracker not found");
		}

		// get the Tracker's address and port
		this->trackerAddress = IPAddressResolver().addressOf(tracker,
				IPAddressResolver::ADDR_IPv4);
		this->trackerPort = par("connectPort").longValue();
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
