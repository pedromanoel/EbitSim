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

/*
 * PeerWireThreadDownload.cc
 *
 *  Created on: May 13, 2010
 *      Author: pevangelista
 */

#include "PeerWireThread.h"

#include <boost/lexical_cast.hpp>

#include "BitTorrentClient.h"
#include "ContentManager.h"
#include "PeerWire_m.h"
#include "PeerWireMsgBundle_m.h"

// Download State Machine methods
void PeerWireThread::addBitField(BitFieldMsg const& msg) {
    assert(this->contentManager);
    this->contentManager->addPeerBitField(msg.getBitField(),
        this->remotePeerId);
}
void PeerWireThread::calculateDownloadRate() {
    assert(this->contentManager);
    unsigned long totalDownloaded = this->contentManager->getTotalDownloaded(
        this->remotePeerId);
    double downRate = this->btClient->updateDownloadRate(this->infoHash,
        this->remotePeerId, totalDownloaded);
#ifdef DEBUG_MSG
    std::string out = "Download rate: "
        + boost::lexical_cast<std::string>(downRate);
    this->printDebugMsg(out);
#endif
}
void PeerWireThread::cancelDownloadRequests() {
    assert(this->contentManager);
    this->contentManager->cancelDownloadRequests(this->remotePeerId);
}
InterestedMsg * PeerWireThread::getInterestedMsg() {
    return new InterestedMsg("InterestedMsg");
}
NotInterestedMsg * PeerWireThread::getNotInterestedMsg() {
    return new NotInterestedMsg("NotInterestedMsg");

}
PeerWireMsgBundle * PeerWireThread::getRequestMsgBundle() {
    assert(this->contentManager);
    PeerWireMsgBundle * bundle = this->contentManager->getNextRequestBundle(
        this->remotePeerId);
#ifdef DEBUG_MSG
    if (bundle == NULL) {
        this->printDebugMsg("Didn't return a Bundle");
    }
#endif
    return bundle;
}
void PeerWireThread::processBlock(PieceMsg const& msg) {
    assert(this->contentManager);
    this->contentManager->processBlock(this->remotePeerId, msg.getIndex(),
        msg.getBegin(), msg.getBlockSize());
}
void PeerWireThread::processHaveMsg(HaveMsg const& msg) {
    assert(this->contentManager);
    this->contentManager->processHaveMsg(msg.getIndex(), this->remotePeerId);
}
void PeerWireThread::renewDownloadRateTimer() {
    cancelEvent(&this->downloadRateTimer);
    scheduleAt(simTime() + this->btClient->downloadRateInterval,
        &this->downloadRateTimer);
}
void PeerWireThread::renewSnubbedTimer() {
    cancelEvent(&this->snubbedTimer);
    scheduleAt(simTime() + this->btClient->snubbedInterval,
        &this->snubbedTimer);
}
void PeerWireThread::setSnubbed(bool snubbed) {
#ifdef DEBUG_MSG
    if (snubbed) {
        this->printDebugMsg("Peer snubbed");
    } else {
        this->printDebugMsg("Peer not snubbed");
    }
#endif
    // snubbed starts as false and is set to true if the timeout occurs
    this->btClient->setSnubbed(snubbed, this->infoHash, this->remotePeerId);
}
void PeerWireThread::startDownloadTimers() {
    this->stopDownloadTimers();

    scheduleAt(simTime() + this->btClient->snubbedInterval,
        &this->snubbedTimer);
    scheduleAt(simTime() + this->btClient->downloadRateInterval,
        &this->downloadRateTimer);
}
void PeerWireThread::stopDownloadTimers() {
    cancelEvent(&this->snubbedTimer);
    cancelEvent(&this->downloadRateTimer);
}
