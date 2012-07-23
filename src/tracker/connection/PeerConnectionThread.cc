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
 * TrackerThread.cc
 *
 *  Created on: Mar 29, 2010
 *      Author: pevangelista
 */

#include "PeerConnectionThread.h"

#include <cstring>
#include <string>
#include <cstdio>//TODO debug
#include <algorithm>

#include "AnnounceRequestMsg_m.h"
#include "AnnounceResponseMsg_m.h"
#include "TrackerApp.h"

Register_Class(PeerConnectionThread);

PeerConnectionThread::PeerConnectionThread() :
        trackerApp(NULL), httpResponseHeaderSize(HTTP_RESPONSE_HEADER_SIZE) {
    // TODO Auto-generated constructor stub

}

PeerConnectionThread::~PeerConnectionThread() {
    delete this->sock;
}
void PeerConnectionThread::closed() {
    // schedule message that will delete the thread
    // don't try to delete the thread from inside itself, because of
    // memory access errors.
    cMessage* appMessage;
    appMessage = new cMessage("Close Thread");
    appMessage->setKind(SELF_CLOSE_THREAD);
    this->scheduleAt(simTime(), appMessage);
}
void PeerConnectionThread::processDataArrived(cMessage *msg) {
    if (dynamic_cast<AnnounceRequestMsg*>(msg)) {
        AnnounceResponseMsg* response = this->makeResponse(
                static_cast<AnnounceRequestMsg*>(msg));
        this->sock->send(response);
        // Server closes the connection right after the response.
        this->sock->close();
    } else {
        std::ostringstream out;
        out << "Message '" << msg->getName()
                << "' has the wrong type of message.";
        this->printDebugMsgConnection(out.str());
        throw std::logic_error(out.str().c_str());
    }
}
void PeerConnectionThread::printDebugMsgConnection(std::string s) {
    std::ostringstream out;
    out << "(ThreadConnection) connId " << this->sock->getConnectionId() << "(";
    out << this->sock->getLocalAddress() << ":";
    out << this->sock->getLocalPort() << " <=> ";
    out << this->sock->getRemoteAddress() << ":";
    out << this->sock->getRemotePort() << ")";

    this->trackerApp->printDebugMsg(out.str());
}
void PeerConnectionThread::dataArrived(cMessage *msg, bool urgent) {
    msg->setKind(SELF_PROCESS_TIME);
    scheduleAt(simTime() + this->trackerApp->par("processTime"), msg);
}
void PeerConnectionThread::established() {
    // Do nothing
}
void PeerConnectionThread::failure(int code) {
}
void PeerConnectionThread::peerClosed() {
    if (this->sock->getState() == TCPSocket::CONNECTED) {
        this->sock->close();
    }
}
void PeerConnectionThread::statusArrived(TCPStatusInfo *status) {
}
void PeerConnectionThread::timerExpired(cMessage *timer) {
    if (timer->getKind() == SELF_PROCESS_TIME) {
        this->processDataArrived(timer);
    } else {
        delete timer;
        timer = NULL;
        throw std::logic_error("Tracker doesn't process timers.");
    }
}

void PeerConnectionThread::init(TCPSrvHostApp* hostmodule, TCPSocket* socket) {
    TCPServerThreadBase::init(hostmodule, socket);
    this->trackerApp = check_and_cast<TrackerApp*>(this->hostmod);
}
AnnounceResponseMsg* PeerConnectionThread::makeResponse(
        AnnounceRequestMsg* announceRequestMsg) {
    // collect all information from the request
    int infoHash = announceRequestMsg->getInfoHash();
    double downloaded = announceRequestMsg->getDownloaded(); // FIXME not used
    AnnounceType event = (AnnounceType) announceRequestMsg->getEvent();
    unsigned int numWant = announceRequestMsg->getNumWant();
    int peerId = announceRequestMsg->getPeerId();
    unsigned int port = announceRequestMsg->getPort();
    double uploaded = announceRequestMsg->getUploaded(); // FIXME not used

    // consume the message
    delete announceRequestMsg;
    announceRequestMsg = NULL;

    AnnounceResponseMsg* response = new AnnounceResponseMsg("Response");

    // Test if Tracker has this content
    std::map<int, SwarmPeerList>::iterator it = this->trackerApp->swarms.find(
            infoHash);

    if (it == this->trackerApp->swarms.end()) {
        // can't respond to a peer if it not on the list
        std::string errorMessage(
                "Swarm with the given infoHash not present in this tracker");
        response->setFailureReason(errorMessage.c_str());
        response->setByteLength(errorMessage.size() + httpResponseHeaderSize);
    } else {
        SwarmPeerList & swarmPeerSet = it->second;

        // create a PeerInfo object
        PeerInfo peerInfo(peerId, this->getSocket()->getRemoteAddress(), port);
        // iterator to the first element equal to or greater than peerInfo,
        // or iterator to end if no such element exists
        SwarmPeerList::iterator lb = swarmPeerSet.lower_bound(peerInfo);
        bool elementInSet = lb != swarmPeerSet.end() && *lb == peerInfo;

        if (event == A_STARTED && elementInSet) {
            // can't add a peer if it is already on the set
            std::string errorMessage(
                    "Swarm already have a peer with the given peerId");
            response->setFailureReason(errorMessage.c_str());
            response->setByteLength(
                    errorMessage.size() + httpResponseHeaderSize);
        } else if (event != A_STARTED && !elementInSet) {
            // can't respond to a peer if it not on the list
            std::string errorMessage(
                    "Swarm don't have the peer with the given peerId");
            response->setFailureReason(errorMessage.c_str());
            response->setByteLength(
                    errorMessage.size() + httpResponseHeaderSize);
        } else {
            switch (event) {
            case A_STARTED:
                // insert with hint
                swarmPeerSet.insert(lb, peerInfo);
                break;
            case A_STOPPED:
                // erase from swarm
                swarmPeerSet.erase(peerInfo);
                break;
            case A_COMPLETED:
                // empty
            case A_NORMAL:
                this->trackerApp->updatePeerStatus(peerInfo, infoHash, event);
                break;
            }
            // get return list
            std::vector<PeerInfo const*> peers =
                    this->trackerApp->getListOfPeers(peerInfo, infoHash,
                            numWant);

            // TODO use these
            //response->setInterval(this->trackerApp->interval);
            //response->setMinInterval(this->trackerApp->minInterval);
            //response->setTrackerId(this->trackerApp->trackerId);
            //response->setComplete(this->trackerApp->completeDownloads);
            //response->setIncomplete(this->trackerApp->incompleteDownloads);
            response->setPeersArraySize(peers.size());

            std::ostringstream out;
            out << "Tracker returned peers (";

            // the bencoded response will be written to this string stream.
            // The size of the response is the size of the bencoded response
            std::ostringstream bencodedResponse;
            // start of the response dictionary and list of peers
            bencodedResponse << "d5:peersl";
            for (unsigned int i = 0; i < peers.size(); ++i) {
                out << peers[i]->getPeerId() << ", ";
                // start of the peer dictionary
                bencodedResponse << peers[i]->getBencodedInfo();
                response->setPeers(i, *peers[i]);
            }
            out << ")";
            this->trackerApp->printDebugMsg(out.str());

            // end of the list of peers and response dictionary
            bencodedResponse << "ee";
            response->setBencodedResponse(bencodedResponse.str().c_str());
            // the size of the bencoded response plus the size of the http header
            response->setByteLength(
                    this->httpResponseHeaderSize
                            + bencodedResponse.str().length());
        }
    }
    return response;
}
