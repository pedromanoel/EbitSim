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

#include "SimulationController.h"

#include <TCPConnection.h>
#include <sstream>

Define_Module(SimulationController);
// private methods
void SimulationController::printDebugMsg(std::string s) {
    if (this->debugFlag) {
        // debug "header"
        std::cerr << simulation.getEventNumber() << " (T=";
        std::cerr << simulation.getSimTime() << ")(SimulationController) - ";
        std::cerr << s << "\n";
    }
}
void SimulationController::registerEmittedSignals() {

}
void SimulationController::subscribeToSignals() {
    this->leecherSignal = registerSignal("ClientController_EnterLeecher");
    this->seederSignal = registerSignal("TrackerApp_BecameSeeder");
    simulation.getSystemModule()->subscribe(this->leecherSignal, this);
    simulation.getSystemModule()->subscribe(this->seederSignal, this);
}

// protected methods
void SimulationController::initialize() {
    this->debugFlag = par("debugFlag").boolValue();
    this->subscribeToSignals();
}

void SimulationController::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        delete msg;
        endSimulation();
    } else {
        delete msg;
        throw cException("This module doesn't process messages");
    }
}
SimulationController::~SimulationController() {
    std::set<std::pair<int, int> >::iterator it = this->leechers.begin();

    std::ostringstream out;
    out << "Leechers at the end of simulation:\n";

    for (; it != this->leechers.end(); ++it) {
        out << "\t(peerId: " << it->first;
        out << ", infoHash: " << it->second << ")\n";
    }

    this->printDebugMsg(out.str());
}
void SimulationController::receiveSignal(cComponent *source,
        simsignal_t signalID, cObject *obj) {
    Enter_Method("receiveSignal");
    DataSimulationControl *data = static_cast<DataSimulationControl *>(obj);
    std::pair<int, int> leecher = std::make_pair(data->getPeerId(),
            data->getInfoHash());

    // Leecher that will request the content
    if (signalID == this->leecherSignal) {
        std::ostringstream out;
        out << "Peer " << data->getPeerId() << " will leech on content "
                << data->getInfoHash();
        this->printDebugMsg(out.str());

        this->leechers.insert(leecher);
    } else if (signalID == this->seederSignal) {
        std::ostringstream out;
        out << "Peer " << data->getPeerId() << " became a seeder for content "
                << data->getInfoHash();
        this->printDebugMsg(out.str());
        // the pair (peerId, infoHash) should be in the leechers set
        this->leechers.erase(leecher);

        // if the set is empty, all leecher became seeders for all the contents
        // they were interested in.
        if (this->leechers.empty()) {
            // give some time to close all the connections (2MSL is defaults to 2 minutes)

            scheduleAt(simTime() + TCP_TIMEOUT_2MSL,
                    new cMessage("End Simulation"));
        }
    }
}
