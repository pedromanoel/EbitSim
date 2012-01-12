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

#include "Statistics.h"

#include <stdexcept>
#include <sstream>

#include "GlobalStatistics.h"

Define_Module( Statistics);

Statistics::Statistics() :
    globalStatistics(NULL), pieceDownloadTime("Piece Download Time"),
    //    pieceDownloadTimeHistogram("Piece Download Time Histogram", 40),
            clientTotalDownloaded("Total Downloaded"), numberOfConnected(
                    "Number Of Connected") {
    //    this->pieceDownloadTimeHistogram.setRangeAuto(100, 1.5);
}

Statistics::~Statistics() {
    // properly dispose of all statistics objects
    //    delete this->pieceDownloadTimeHistogram;
    //    delete this->clientTotalDownloaded;
    //    delete this->numberOfConnected;

    //    {
    //        std::map<std::pair<int, int>, cOutVector*>::iterator it;
    //        it = this->clientTotalDownloadedByPeer.begin();
    //        for (; it != this->clientTotalDownloadedByPeer.end(); ++it) {
    //            delete it->second;
    //        }
    //    }
    {
        std::map<int, cDoubleHistogram*>::iterator it;
        it = this->peerDownloadRate.begin();
        for (; it != this->peerDownloadRate.end(); ++it) {
            delete it->second;
            it->second = NULL;
        }
        it = this->peerUploadRate.begin();
        for (; it != this->peerUploadRate.end(); ++it) {
            delete it->second;
            it->second = NULL;
        }
    }
}

void Statistics::initialize() {
    cModule* globalStatistics =
            simulation.getSystemModule()->getModuleByRelativePath(
                    "globalStatistics");

    if (globalStatistics == NULL) {
        throw std::logic_error("globalStatistics module not found");
    }
    this->globalStatistics = check_and_cast<GlobalStatistics *> (
            globalStatistics);
}

void Statistics::collectPieceDownloadTime(SimTime const& downloadTime) {
    Enter_Method("collectPieceDownloadTime(download time: %f)",
            downloadTime.dbl());

    //	this->pieceDownloadTime[peerId]->collect(downloadTime);
    //    this->pieceDownloadTimeHistogram.collect(downloadTime);
    this->pieceDownloadTime.collect(downloadTime);
    if (this->globalStatistics != NULL) {
        this->globalStatistics->collectPieceDownloadTime(downloadTime);
    }
}
//void Statistics::collectTotalDownloadedByPeer(int clientPeerId,
//        int remotePeerId, int totalDownloaded) {
//    Enter_Method(
//            "collectTotalDownloaded(peerId: %d, remotePeerId: %d, totalDownloaded: %d)",
//            clientPeerId, remotePeerId, totalDownloaded);
//
//    std::pair<int, int> id = std::make_pair(clientPeerId, remotePeerId);
//
//    if (!this->clientTotalDownloadedByPeer.count(id)) {
//        std::ostringstream name;
//        name << "Total Downloaded " << id.first << "-" << id.second;
//        this->clientTotalDownloadedByPeer[id] = new cOutVector(
//                name.str().c_str());
//    }
//
//    this->clientTotalDownloadedByPeer[id]->record(totalDownloaded);
//}
void Statistics::collectTotalDownloaded(int totalDownloaded) {
    Enter_Method("collectTotalDownloaded(totalDownloaded: %d)", totalDownloaded);

    this->clientTotalDownloaded.record(totalDownloaded);
}
void Statistics::collectDownloadRate(int peerId, double downloadRate) {
    Enter_Method("collectDownloadRate(peerId: %d, downloadRate: %f)", peerId,
            downloadRate);
    if (!this->peerDownloadRate.count(peerId)) {
        std::ostringstream name;
        name << "Download Rate " << peerId;

        cDoubleHistogram* hist = new cDoubleHistogram(name.str().c_str(), 20);
        hist->setRangeAuto(100, 1.5);
        this->peerDownloadRate[peerId] = hist;
    }

    this->peerDownloadRate[peerId]->collect(downloadRate);
}
void Statistics::collectUploadRate(int peerId, double uploadRate) {
    Enter_Method("collectUploadRate(peerId: %d, uploadRate: %f)", peerId,
            uploadRate);
    if (!this->peerUploadRate.count(peerId)) {
        std::ostringstream name;
        name << "Upload Rate " << peerId;

        cDoubleHistogram* hist = new cDoubleHistogram(name.str().c_str(), 20);
        hist->setRangeAuto(100, 1.5);
        this->peerUploadRate[peerId] = hist;
    }
    this->peerUploadRate[peerId]->collect(uploadRate);
}
void Statistics::collectNumberOfConnected(int numOfPeers) {
    Enter_Method("collectNumberOfConnected(numOfPeers: %d)", numOfPeers);
    this->numberOfConnected.record(numOfPeers);
}

// protected methods
void Statistics::handleMessage(cMessage *msg) {
    throw cException("This module does not receive messages");
}

void Statistics::finish() {
    {
        std::map<int, cDoubleHistogram*>::iterator it;
        it = this->peerDownloadRate.begin();
        for (; it != this->peerDownloadRate.end(); ++it) {
            it->second->record();
        }
        it = this->peerUploadRate.begin();
        for (; it != this->peerUploadRate.end(); ++it) {
            it->second->record();
        }

        //        if (this->pieceDownloadTimeHistogram.getCount()) {
        //            this->pieceDownloadTimeHistogram.record();
        //        }

        if (this->pieceDownloadTime.getCount()) {
            this->pieceDownloadTime.record();
        }

    }
}
