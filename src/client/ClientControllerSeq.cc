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

#include "ClientControllerSeq.h"

#include <IPAddressResolver.h>
#include <cxmlelement.h>
#include <cstring>

#include "DataSimulationControl.h"
#include "SwarmManager.h"

Define_Module(ClientControllerSeq);

// cListener method
void ClientControllerSeq::receiveSignal(cComponent *source,
        simsignal_t signalID, long infoHash) {

    if (signalID == this->seederSignal) {
        std::ostringstream out;
        out << "Became seeder of the swarm " << infoHash;
        this->printDebugMsg(out.str());
        // Start downloading next content
        if (!this->contentDownloadQueue.empty()) {
            scheduleAt(simTime(), &this->enterSwarmMsg);
        }
    }
}

// public methods
ClientControllerSeq::ClientControllerSeq() :
        enterSwarmMsg("Enter swarm"), enterSwarmSeederMsg(
                "Enter swarm seeding"), swarmManager(NULL), localPeerId(-1), debugFlag(
                false) {
}

ClientControllerSeq::~ClientControllerSeq() {
    cancelEvent(&this->enterSwarmMsg);
    cancelEvent(&this->enterSwarmSeederMsg);
}
int ClientControllerSeq::getPeerId() const {
    return this->localPeerId;
}
// Private methods
int ClientControllerSeq::numInitStages() const {
    return 4;
}

// Starting point of the simulation
void ClientControllerSeq::initialize(int stage) {
    if (stage == 0) {
        this->registerEmittedSignals();

        // Make the peerId equal to the module id, which is unique throughout the simulation.
        this->localPeerId = this->getParentModule()->getParentModule()->getId();

        // get references to other modules
        cModule *swarmManager = getParentModule()->getSubmodule("swarmManager");

        if (swarmManager == NULL) {
            throw cException("SwarmManager module not found");
        }

        this->swarmManager = check_and_cast<SwarmManager*>(swarmManager);

        this->updateStatusString();
        this->debugFlag = par("debugFlag").boolValue();
    } else if (stage == 1) {
        // Acquire the torrent metadata directly from the Tracker. The Tracker's
        // address is defined in the SwarmManager module
        std::string trackerAppPath =
                this->swarmManager->par("connectAddress").stringValue();
        trackerAppPath += ".trackerApp";

        // Get a pointer the Tracker module
        cModule * module = simulation.getModuleByPath(trackerAppPath.c_str());
        if (module == NULL) {
            throw std::logic_error("TrackerApp module not found");
        }
        TrackerApp* trackerApp = check_and_cast<TrackerApp *>(module);

        // Read the list of contents this Peer will download as defined by its profile parameter.
        cXMLElementList contents =
                par("profile").xmlValue()->getChildrenByTagName("content");
        if (contents.empty()) {
            throw std::invalid_argument(
                    "List of contents is empty. Check the xml file");
        }
        cXMLElementList::iterator it = contents.begin();

        // This Peer will seed all of the contents present in the profile.
        bool seeder = par("seeder").boolValue();

        for (; it != contents.end(); ++it) {
            cXMLElement * contentNode = (*it)->getFirstChild();
            std::string contentName = contentNode->getNodeValue();

            // This simulates the phase where the Client get the the .torrent files
            // from the torrent repository server.
            TorrentMetadata const& torrent = trackerApp->getTorrentMetaData(
                    contentName);

            // emit this signal warning that this peer is leeching this torrent
            // file. It can be used to count how many leechers there are in the
            // beginning of the simulation.
            if (!seeder) {
                DataSimulationControl data;
                data.setPeerId(this->localPeerId);
                data.setInfoHash(torrent.infoHash);
                emit(this->leecherSignal, &data);
            }
            this->contentDownloadQueue.push_back(torrent);
        }

        // if this btapp is defined, start downloading after its completion, or else
        // start from the startTime parameter
        char const* startAfter = par("startAfter").stringValue();

        if (strcmp(startAfter, "") != 0) {
            cModule * startAfterModule =
                    getParentModule()->getParentModule()->getSubmodule(
                            startAfter);
            if (startAfterModule == NULL) {
                throw std::logic_error(
                        "startAfter BitTorrentApp module not found");
            }
            // subscribe to the seeder signal from the 'start after' module
            this->seederSignal = registerSignal("ContentManager_BecameSeeder");
            startAfterModule->subscribe(this->seederSignal, this);
        } else {
            this->seederSignal = registerSignal("ContentManager_BecameSeeder");
            getParentModule()->subscribe(this->seederSignal, this);

            if (seeder) {
                // Will enter all swarms that this client is seeding at the
                // begining of the simulation.
                scheduleAt(simTime(), &this->enterSwarmSeederMsg);
            } else {
                // will schedule the entry to the first swarm on the queue
                scheduleAt(simTime() + par("startTime").doubleValue(),
                        &this->enterSwarmMsg);
            }
        }

    } else if (stage == 3) {
        if (this->debugFlag) {
            IPAddressResolver resolver;
            cModule* peerModule = this->getParentModule()->getParentModule();
            std::ostringstream out;
            out << "Address of peer[" << peerModule->getIndex();
            out << "] peerId = " << this->localPeerId << " is ";
            out << resolver.addressOf(peerModule);
            this->printDebugMsg(out.str());
        }
    }
}
// Private methods
void ClientControllerSeq::printDebugMsg(std::string s) {
    if (this->debugFlag) {
        // debug "header"
        std::cerr << simulation.getEventNumber() << " (T=";
        std::cerr << simulation.getSimTime() << ")(ClientControllerSeq) - ";
        std::cerr << "Peer " << this->localPeerId << ": ";
        std::cerr << s << "\n";
    }
}
void ClientControllerSeq::updateStatusString() {
    if (ev.isGUI()) {
        std::ostringstream out;
        out << "peerId: " << this->localPeerId;
        getDisplayString().setTagArg("t", 0, out.str().c_str());
    }
}
void ClientControllerSeq::registerEmittedSignals() {
    // signal that this Client entered a swarm
    this->leecherSignal = registerSignal("ClientControllerSeq_EnterLeecher");
}
void ClientControllerSeq::subscribeToSignals() {
    // subscribe to the BitTorrentApp module, since this signal comes from
    // the ContentManager, inside the SwarmManager.
    this->seederSignal = registerSignal("ContentManager_BecameSeeder");
    getParentModule()->subscribe(this->seederSignal, this);
}

// Protected methods
// TODO add startTimer (to tell when the Client will enter the swarm)
// TODO make creation of Peers dynamic? Research memory consumption gains
// TODO add stopTimer (to tell when the Client will leave the swarm)
// TODO add seedTimer (to tell how long the Client will be seeding). Maybe utilize the stopTimer.
void ClientControllerSeq::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        if (msg == &this->enterSwarmSeederMsg) {
            // Will seed all contents in the contentDowloadQueue
            while (!this->contentDownloadQueue.empty()) {
                TorrentMetadata & torrentMetadata =
                        this->contentDownloadQueue.front();
                this->swarmManager->enterSwarm(torrentMetadata, true);
                this->contentDownloadQueue.pop_front();
            }
        } else if (msg == &this->enterSwarmMsg) {
            // enter the swarm identified by the infoHash
            TorrentMetadata & torrentInfo = this->contentDownloadQueue.front();
            this->swarmManager->enterSwarm(torrentInfo, false);

            std::ostringstream out;
            out << "Entering swarm " << torrentInfo.infoHash;
            this->printDebugMsg(out.str());

            this->contentDownloadQueue.pop_front();
        }
    } else {
        throw cException("This module doesn't process messages");
    }
}
