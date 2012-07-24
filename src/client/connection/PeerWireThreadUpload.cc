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
 * PeerWireThreadUpload.cc
 *
 *  Created on: May 13, 2010
 *      Author: pevangelista
 */

#include "PeerWireThread.h"

#include <boost/lexical_cast.hpp>

#include "BitTorrentClient.h"
#include "ContentManager.h"
#include "Choker.h"
#include "PeerWire_m.h"

// Upload State Machine methods
void PeerWireThread::cancelPiece(CancelMsg const& msg) {
    //TODO
    throw std::logic_error("Cancel not implemented");
}
void PeerWireThread::cancelUploadRequests() {
    assert(this->contentManager);
    this->contentManager->cancelUploadRequests(this->remotePeerId);
}
void PeerWireThread::calculateUploadRate() {
    assert(this->contentManager);
    unsigned long totalUploaded = this->contentManager->getTotalUploaded(
        this->remotePeerId);
    double upRate = this->btClient->updateUploadRate(this->infoHash,
        this->remotePeerId, totalUploaded);

    std::string out = "Upload rate: "
        + boost::lexical_cast<std::string>(upRate);
    this->printDebugMsgUpload(out);
}
void PeerWireThread::callChokeAlgorithm() {
    // The choke algorithm is called when the upload machine stops, even when
    // it stopped because the swarm has been deleted
    assert(this->choker);
    this->choker->callChokeAlgorithm();
}
void PeerWireThread::addPeerToChoker() {
    assert(this->choker);
    this->choker->addInterestedPeer(this->remotePeerId);
}
ChokeMsg * PeerWireThread::getChokeMsg() {
    return new ChokeMsg("ChokeMsg");
}
void PeerWireThread::requestPieceMsg(RequestMsg const& msg) {
    assert(this->contentManager);
    // the Peer must ask for pieces that the
    this->contentManager->requestPieceMsg(this->remotePeerId, msg.getIndex(),
        msg.getBegin(), msg.getReqLength());
}
PieceMsg * PeerWireThread::getPieceMsg() {
    assert(this->contentManager);
    // the Peer must ask for pieces that the
    PieceMsg * pieceMsg = this->contentManager->getPieceMsg(this->remotePeerId);
    return pieceMsg;
}
UnchokeMsg * PeerWireThread::getUnchokeMsg() {
    return new UnchokeMsg("UnchokeMsg");
}
void PeerWireThread::renewUploadRateTimer() {
    cancelEvent(&this->uploadRateTimer);

    scheduleAt(simTime() + this->btClient->uploadRateInterval,
        &this->uploadRateTimer);
}
void PeerWireThread::setInterested(bool interested) {
    this->btClient->setInterested(interested, this->infoHash,
        this->remotePeerId);
}
void PeerWireThread::startUploadTimers() {
    this->stopUploadTimers();
    scheduleAt(simTime() + this->btClient->uploadRateInterval,
        &this->uploadRateTimer);
}
void PeerWireThread::stopUploadTimers() {
    cancelEvent(&this->uploadRateTimer);
}
