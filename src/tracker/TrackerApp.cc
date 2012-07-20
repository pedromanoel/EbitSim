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

#include "TrackerApp.h"

#include <algorithm>
#include <cstring>
#include <cstdio>//TODO debug
#include <boost/iterator/counting_iterator.hpp>
#include <cxmlelement.h>

#include "AnnounceRequestMsg_m.h"
#include "AnnounceResponseMsg_m.h"
//#include "DataSimulationControl.h"
#include "PeerConnectionThread.h"

Define_Module(TrackerApp);

TrackerApp::TrackerApp() :
        debugFlag(false), totalBytesUploaded(0), totalBytesDownloaded(0) {
}
TrackerApp::~TrackerApp() {
}

void TrackerApp::printDebugMsg(std::string s) {
    if (this->debugFlag) {
        // debug "header"
        std::cerr << simulation.getEventNumber() << " (T=";
        std::cerr << simulation.getSimTime() << ")(TrackerApp) - ";
        std::cerr << "Tracker: ";
        std::cerr << s << "\n";
    }
}
std::vector<PeerInfo const*> TrackerApp::getListOfPeers(
        PeerInfo const& peerInfo, int infoHash,
        unsigned int numberOfPeers) const {
    std::vector<PeerInfo const*> returnList;

    // TODO make possible to select different selection algorithms
    returnList = this->getRandomPeers(peerInfo, infoHash, numberOfPeers);

    return returnList;
}

std::vector<PeerInfo const*> TrackerApp::getRandomPeers(
        PeerInfo const& peerInfo, int infoHash,
        unsigned int numberOfPeers) const {

    std::vector<PeerInfo const*> returnList;
    // the size of the peerList minus self
    returnList.reserve(numberOfPeers - 1);

    SwarmPeerList const & peerList = this->swarms.at(infoHash);
    SwarmPeerList::iterator it = peerList.begin();

    // get all the pointers to the PeerInfo objects, except for self
    for (; it != peerList.end(); ++it) {
        if (*it != peerInfo) {
            returnList.push_back(&(*it));
        }
    }

    // random shuffle the return list
    std::random_shuffle(returnList.begin(), returnList.end(), intrand);

    // throw away the extra pointers.
    if (numberOfPeers < returnList.size()) {
        returnList.resize(numberOfPeers);
    }

    return returnList;
}

// TODO save all other information
void TrackerApp::updatePeerStatus(PeerInfo const& peerInfo, int infoHash,
        AnnounceType status) {
    SwarmPeerList & peerList = this->swarms.at(infoHash);
    SwarmPeerList::iterator it = peerList.find(peerInfo);

    if (it != peerList.end()) {
        if (status == A_COMPLETED) {
            // the peer was a leecher, and now is a seeder
//            DataSimulationControl data;
//            data.setPeerId(peerInfo.getPeerId());
//            data.setInfoHash(infoHash);
//            this->printDebugMsg("Became a seeder");
//            emit(this->seederSignal, &data);
        }
    }
}

void TrackerApp::initialize() {
    this->registerEmittedSignals();
    TCPSrvHostApp::initialize();

    this->debugFlag = par("debugFlag").boolValue();

    // read all contents from the xml file.
    cXMLElementList contentsList =
            par("contents").xmlValue()->getChildrenByTagName("content");

    if (contentsList.empty()) {
        throw std::invalid_argument(
                "List of contents is empty. Check the xml file");
    }
    cXMLElementList::iterator it = contentsList.begin();

    for (; it != contentsList.end(); ++it) {
        TorrentMetadata torrentMetadata;
        cXMLElement * child;

        // create the torrent metadata
        child = (*it)->getFirstChildWithTag("numOfPieces");
        torrentMetadata.numOfPieces = atoi(child->getNodeValue());
        child = (*it)->getFirstChildWithTag("numOfSubPieces");
        torrentMetadata.numOfSubPieces = atoi(child->getNodeValue());
        child = (*it)->getFirstChildWithTag("subPieceSize");
        torrentMetadata.subPieceSize = atoi(child->getNodeValue());
        torrentMetadata.infoHash = simulation.getUniqueNumber();
        this->swarms[torrentMetadata.infoHash];

        // save a list of torrents for each video content
        std::string contentName((*it)->getAttribute("name"));
        this->contents[contentName] = torrentMetadata;
    }
}

void TrackerApp::handleMessage(cMessage* msg) {
    if (msg->isSelfMessage()) {
        PeerConnectionThread *thread =
                static_cast<PeerConnectionThread *>(msg->getContextPointer());
        if (msg->getKind() == PeerConnectionThread::SELF_CLOSE_THREAD) {
            // remove this thread. Called when this connection is closed.
            delete msg;
            msg = NULL;
            this->removeThread(thread);
        } else {
            thread->timerExpired(msg);
        }
    } else {
        TCPSrvHostApp::handleMessage(msg);
    }
}

TorrentMetadata const& TrackerApp::getTorrentMetaData(std::string contentName) {
    Enter_Method("getTorrentMetaData(content: %s)", contentName.c_str());
    return this->contents.at(contentName);
}

void TrackerApp::registerEmittedSignals() {
    // register the signal sent when a peer becomes a seeder
    this->seederSignal = registerSignal("TrackerApp_BecameSeeder");
}

