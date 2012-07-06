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
//
/*
 * PeerWireThreadHandshake.cpp
 *
 *  Created on: May 13, 2010
 *      Author: pevangelista
 */

#include "PeerWireThread.h"

#include <boost/tuple/tuple.hpp>
using boost::make_tuple;
#include <cassert>

#include "BitTorrentClient.h"
#include "Choker.h"
#include "ContentManager.h"
#include "SwarmManager.h"

// ConnectionSM's external transition callers
void PeerWireThread::outgoingPeerWireMsg_ConnectionSM(PeerWireMsg * msg) {
    this->connectionSm.outgoingPeerWireMsg(msg);
}
void PeerWireThread::outgoingPeerWireMsg_ConnectionSM(PeerWireMsgBundle * msg) {
    if (msg != NULL) {
        this->connectionSm.outgoingPeerWireMsg(msg);
    }
}

// Connection State Machine transitions
void PeerWireThread::addConnectedPeer() {
    this->btClient->addConnectedPeer(this->infoHash, this->remotePeerId, this,
            this->activeConnection);
    // start an empty BitField for this connection
    this->contentManager->addEmptyBitField(this->remotePeerId);

    std::ostringstream out;
    out << "Connection established.";
    this->printDebugMsgConnection(out.str());
}
void PeerWireThread::closeLocalConnection() {
    // close the connection
    this->sock->close();
}
void PeerWireThread::stopMachines() {
    this->downloadSm.stopMachine();
    this->uploadSm.stopMachine();
}
BitFieldMsg * PeerWireThread::getBitFieldMsg() {
    return this->contentManager->getClientBitFieldMsg();
}
Handshake * PeerWireThread::getHandshake() {
    if (this->infoHash == -1 || this->remotePeerId == -1) {
        throw std::logic_error(
                "Trying to send a handshake with peerId or infoHash undefined.");
    }

    Handshake * handshake = new Handshake("Handshake");
    handshake->setInfoHash(this->infoHash);
    handshake->setPeerId(this->btClient->localPeerId);
    handshake->setByteLength(handshake->getLength());
    return handshake;
}
KeepAliveMsg * PeerWireThread::getKeepAliveMsg() {
    return new KeepAliveMsg("KeepAliveMsg");
}
void PeerWireThread::renewKeepAliveTimer() {
    this->cancelEvent(&this->keepAliveTimer);
    this->scheduleAt(simTime() + this->btClient->keepAliveInterval,
            &this->keepAliveTimer);
}
void PeerWireThread::renewTimeoutTimer() {
    this->cancelEvent(&this->timeoutTimer);
    this->scheduleAt(simTime() + this->btClient->timeoutInterval,
            &this->timeoutTimer);
}
void PeerWireThread::sendPeerWireMsg(cPacket * msg) {
    // The packet's byte length is not automatically calculated, so it must be
    // set manually.
    if (dynamic_cast<PeerWireMsgBundle*>(msg)) {
        PeerWireMsgBundle* bundle = static_cast<PeerWireMsgBundle*>(msg);
        // Set the size of the packet, then send it. Suppose that the messages
        // inside the bundle have their size correctly set.
        bundle->setByteLength(bundle->getBundle().length());
    } else if (dynamic_cast<PeerWireMsg*>(msg)) {
        PeerWireMsg * peerWireMsg = static_cast<PeerWireMsg*>(msg);
        // Set the real payload length (constant + variable). See PeerWire.msg
        // for more details.
        peerWireMsg->setPayloadLen(
                peerWireMsg->getPayloadLen()
                        + peerWireMsg->getVariablePayloadLen());
        peerWireMsg->setByteLength(
                peerWireMsg->getPayloadLen() + peerWireMsg->getHeaderLen());
    } else {
        throw std::logic_error("Wrong type of message");
    }

    this->btClient->peerWireStatistics(msg, true);

    if (this->getSocket()->getState() != TCPSocket::CONNECTED) {
        // tried to send a message, but the connection is not established.
        // log this event and delete the message
        std::ostringstream out;
        out << "Tried to send message \"" << msg->getName();
        out << "\", but the socket is not connected";
        this->printDebugMsgConnection(out.str());
        delete msg;
    } else {
        std::ostringstream out;
        out << "Message \"" << msg->getName();
        out << "\" sent";
        this->printDebugMsgConnection(out.str());
        // claim ownership over the message since it may not have been created in
        // this module.
        this->btClient->take(msg);
        // send the message to the connected Peer.
        this->getSocket()->send(msg);
    }
}
void PeerWireThread::startHandshakeTimers() {
    this->stopHandshakeTimers();

    this->scheduleAt(simTime() + this->btClient->timeoutInterval,
            &this->timeoutTimer);
    this->scheduleAt(simTime() + this->btClient->keepAliveInterval,
            &this->keepAliveTimer);
}
void PeerWireThread::stopHandshakeTimers() {
    this->cancelEvent(&this->timeoutTimer);
    this->cancelEvent(&this->keepAliveTimer);
}
void PeerWireThread::terminateThread() {
    this->printDebugMsg("Thread removed");
    this->terminating = true;

    // remove the connection from the BitTorrentClient
    if (this->infoHash != -1 && this->remotePeerId != 1) {
        this->btClient->removePeerInfo(this->infoHash, this->remotePeerId,
                this->sock->getConnectionId(), this->activeConnection);
    }

    // remove the peer from the swarm related modules
    if (this->contentManager != NULL) {
        this->contentManager->removePeerInfo(this->remotePeerId);
    }
    if (this->choker != NULL) {
        this->choker->removePeerInfo(this->remotePeerId);
    }

    // cancel all messages that were going to be executed
    while (!this->messageQueue.empty()) {
        cObject * msg = this->messageQueue.pop();
        std::string name = msg->getName();
        std::string out = "Canceled message \"" + name + "\"";
        this->printDebugMsg(out);
        delete msg;
    }
    while (!this->postProcessingAppMsg.empty()) {
        cObject * msg = this->postProcessingAppMsg.pop();
        std::string name = msg->getName();
        std::string out = "Canceled post-message \"" + name + "\"";
        this->printDebugMsg(out);
        delete msg;
    }
//    while (!this->peerWireMessageBuffer.empty()) {
//        std::ostringstream out;
//        cObject * msg = this->peerWireMessageBuffer.pop();
//        out << "Canceled message \"" << msg->getName() << "\"";
//        this->printDebugMsg(out.str());
//        delete msg;
//    }
//    while (!this->applicationMsgQueue.empty()) {
//        std::ostringstream out;
//        cObject * appMsg = this->applicationMsgQueue.pop();
//
//        out << "Canceled application message \"" << appMsg->getName() << "\"";
//        this->printDebugMsg(out.str());
//        delete appMsg;
//    }
}
// transition guards
bool PeerWireThread::checkHandshake(Handshake const& hs) {
    // verify if infoHash is available
    std::pair<Choker*, ContentManager*> modules =
            this->btClient->swarmManager->checkSwarm(hs.getInfoHash());

    // true if no peerId is expected or if the received peerId matches the expected.
    bool validPeerId = (this->remotePeerId == -1)
            || (this->remotePeerId == hs.getPeerId());

    bool canConnect = this->btClient->canConnect(hs.getInfoHash(),
            hs.getPeerId(), this->activeConnection);

    bool swarmExists = modules.first && modules.second && validPeerId;

    if (validPeerId && canConnect && swarmExists) {
        // set this connection's parameters
        this->remotePeerId = hs.getPeerId();
        this->infoHash = hs.getInfoHash();
        // attach the swarm modules to this connection
        this->choker = modules.first;
        this->contentManager = modules.second;
    } else {
        std::ostringstream out;
        out << "Handshake refused";
        if (!validPeerId) {
            out << "(invalid peerId)";
        }
        if (!canConnect) {
            out << "(can't connect)";
        }
        if (!swarmExists) {
            out << "(swarm don't exist)";
        }
        this->printDebugMsgConnection(out.str());
    }

    return validPeerId && canConnect && swarmExists;
}
bool PeerWireThread::isBitFieldEmpty() {
    return this->contentManager->isBitFieldEmpty();
}
