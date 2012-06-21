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

#include "ClientController.h"

#include <IPAddressResolver.h>
#include <cxmlelement.h>
#include <cstring>
#include <boost/lexical_cast.hpp>
#include <ctopology.h>

#include "SwarmManager.h"
#include "UserCommand_m.h"

Define_Module(ClientController);

// cListener method
//void ClientController::receiveSignal(cComponent *source, simsignal_t signalID,
//    long infoHash) {
//}

// public methods
ClientController::ClientController() :
        debugFlag(false) {
}

ClientController::~ClientController() {
}
// Private methods
int ClientController::numInitStages() const {
    return 4;
}

// Starting point of the simulation
void ClientController::initialize(int stage) {
    if (stage == 3) {
        // get the parameters
        std::string sTrackerAddress = par("trackerAddress").stringValue();
        int trackerPort = par("trackerPort").longValue();
        bool debugFlag = par("debugFlag").boolValue();
        double seederPercentage = par("seederPercentage").doubleValue();
        simtime_t startTime = par("startTime").doubleValue();
        cXMLElement * profile = par("profile").xmlValue();

        // verify parameters
        if (seederPercentage <= 0.0 || seederPercentage >= 1.0) {
            throw cException("The percentage of Seeders is invalid");
        }
        cXMLElementList contentList = profile->getChildrenByTagName("content");
        if (contentList.empty()) {
            throw cException("List of contents is empty. Check the xml file");
        }
        // throw an error if unable to resolve
        IPvXAddress trackerAddress = IPAddressResolver().resolve(
            sTrackerAddress.c_str(), IPAddressResolver::ADDR_IPv4);

        // get references to other modules
        TrackerApp* trackerApp = check_and_cast<TrackerApp *>(
            simulation.getModuleByPath(
                sTrackerAddress.append(".trackerApp").c_str()));

        // For each content in the profile, schedule the start messages.
        cXMLElementList::iterator it = contentList.begin();
        for (int num = 0; it != contentList.end(); ++it, ++num) {
            using boost::lexical_cast;

            char const* contentName =
                (*it)->getFirstChildWithTag("name")->getNodeValue();
            simtime_t interarrival = lexical_cast<double>(
                (*it)->getFirstChildWithTag("interarrival")->getNodeValue());

            // This simulates the phase where the Client get the the .torrent
            // files from the torrent repository server.
            TorrentMetadata const& torrentMetadata =
                trackerApp->getTorrentMetaData(contentName);

            scheduleStartMessages(startTime, interarrival, seederPercentage,
                torrentMetadata, trackerAddress, trackerPort);

        }

        this->updateStatusString();

//        cMessage *msg = new cMessage("Leave Swarm");
//        msg->setKind(USER_COMMAND_LEAVE_SWARM);
//        LeaveSwarmCommand * leaveSwarmCommand = new LeaveSwarmCommand();
//        LeaveSwarmCommand->setInfoHash(infoHash);
//        msg->setControlInfo(leaveSwarmCommand);
    }
}
// Private methods

void ClientController::scheduleStartMessages(simtime_t const& startTime,
    simtime_t const& interarrivalTime, double seederPercentage,
    TorrentMetadata const& torrentMetadata, IPvXAddress const& trackerAddress,
    int trackerPort) {
    cTopology topo;
    topo.extractByProperty("peer");

    int numNodes = topo.getNumNodes();
    // round up to 1
    int numSeeders = (numNodes * seederPercentage) < 1? 1 : numNodes * seederPercentage;

    simtime_t enterTime = startTime;

    for (int i = 0; i < topo.getNumNodes(); ++i) {
        SwarmManager * swarmManager = check_and_cast<SwarmManager *>(
            topo.getNode(i)->getModule()->getSubmodule("swarmManager"));

        // The first numSeeders will start immediately
        bool seeder = i < numSeeders;

        cMessage *msg = new cMessage("Enter Swarm");
        msg->setKind(USER_COMMAND_ENTER_SWARM);
        msg->setContextPointer(swarmManager);
        EnterSwarmCommand * enterSwarmCommand = new EnterSwarmCommand();
        enterSwarmCommand->setSeeder(seeder);
        enterSwarmCommand->setTorrentMetadata(torrentMetadata);
        enterSwarmCommand->setTrackerAddress(trackerAddress);
        enterSwarmCommand->setTrackerPort(trackerPort);
        msg->setControlInfo(enterSwarmCommand);

        // all seeders start at the beginning of the simulation
        if (seeder) {
            scheduleAt(simTime(), msg);
        } else {
            scheduleAt(enterTime, msg);
            enterTime += exponential(interarrivalTime);
        }
    }

}
void ClientController::printDebugMsg(std::string s) {
    if (this->debugFlag) {
//        // debug "header"
//        std::cerr << simulation.getEventNumber() << " (T=";
//        std::cerr << simulation.getSimTime() << ")(ClientController) - ";
//        std::cerr << "Peer " << this->localPeerId << ": ";
//        std::cerr << s << "\n";
    }
}
void ClientController::updateStatusString() {
    if (ev.isGUI()) {
//        std::ostringstream out;
//        out << "peerId: " << this->localPeerId;
//        getDisplayString().setTagArg("t", 0, out.str().c_str());
    }
}
//void ClientController::registerEmittedSignals() {
//}
//void ClientController::subscribeToSignals() {
//}

// Protected methods
// TODO add startTimer (to tell when the Client will enter the swarm)
// TODO make creation of Peers dynamic? Research memory consumption gains
// TODO add stopTimer (to tell when the Client will leave the swarm)
// TODO add seedTimer (to tell how long the Client will be seeding). Maybe utilize the stopTimer.
void ClientController::handleMessage(cMessage *msg) {
    if (!msg->isSelfMessage()) {
        throw cException("This module doesn't process messages");
    }
    SwarmManager * swarmManager =
        static_cast<SwarmManager *>(msg->getContextPointer());
    // Send the scheduled message directly to the swarm manager module
    sendDirect(msg, swarmManager, "userCommand");
}
