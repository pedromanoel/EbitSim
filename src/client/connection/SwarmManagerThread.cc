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

#include "SwarmManagerThread.h"

#include <ostream>
#include <vector>

#include "AnnounceRequestMsg_m.h"
#include "BitTorrentClient.h"
#include "SwarmManager.h"
//#include "DataRateCollector.h"TODO

Register_Class(SwarmManagerThread);

SwarmManagerThread::SwarmManagerThread() {
}

SwarmManagerThread::SwarmManagerThread(
        AnnounceRequestMsg const& announceRequest, double refreshInterval,
        bool seeder, IPvXAddress const& trackerAddress, int trackerPort) :
        announceRequest(announceRequest), requestTimer("re-request timeout"), refreshInterval(
                refreshInterval), seeder(seeder), trackerAddress(
                trackerAddress), trackerPort(trackerPort) {
}

SwarmManagerThread::~SwarmManagerThread() {
    cancelEvent(&this->requestTimer);
}
void SwarmManagerThread::sendAnnounce(ANNOUNCE_TYPE announceType) {
    this->announceRequest.setEvent(announceType);

    // if the Peer was not seeding, and an A_COMPLETED announce is being sent
    // then the Peer just became a seeder.
    if (announceType == A_COMPLETED) {
        this->seeder = true;
    }
    // make announce timer occur immediately.
    cancelEvent(&this->requestTimer);
    scheduleAt(simTime(), &this->requestTimer);
}
void SwarmManagerThread::established() {
    // create the HTTP url
    std::ostringstream url;
    url << "infohash=XXXXXXXXXXXXXXXXXXXX"; // 20 chars
    url
            << "?event="
            << this->getEventStr(
                    (ANNOUNCE_TYPE) this->announceRequest.getEvent());
    url << "?numwant=" << this->announceRequest.getNumWant();
    url << "?peer_id=" << this->announceRequest.getPeerId();
    url << "?port=" << this->announceRequest.getPort();
    this->announceRequest.setUrlEncodedRequest(url.str().c_str());
    // set the correct length
    this->announceRequest.setByteLength(
            this->announceRequest.getBaseByteLength() + url.str().size());
    // TODO get rates from somewhere
    //    this->announceRequest.setDownloaded(rates.first);
    //    this->announceRequest.setUploaded(rates.second);

    // send this connection's announce first request.
    this->sock->send(this->announceRequest.dup());
    // change announce type back to the default value.
    this->announceRequest.setEvent(A_NORMAL);
}
void SwarmManagerThread::dataArrived(cMessage *msg, bool urgent) {
    AnnounceResponseMsg* response = check_and_cast<AnnounceResponseMsg*>(msg);
    if (strlen(response->getFailureReason()) > 0) {
        delete msg;
        msg = NULL;
        // Error
        throw std::logic_error("Tracker returned with an Error");
    }
    SwarmManager* swarmManager = static_cast<SwarmManager*>(this->hostmod);

    std::ostringstream out;
    out << " Tracker Client received " << response->getPeersArraySize()
            << " peers from the Tracker.";
    swarmManager->printDebugMsg(out.str());

    // get list of Peers returned by the tracker and tell the ConnectedPeerManager
    // to connect with them.
    std::list<tuple<int, IPvXAddress, int> > trackerPeers;
    for (unsigned int i = 0; i < response->getPeersArraySize(); ++i) {
        PeerInfo& info = response->getPeers(i);
        trackerPeers.push_back(
                make_tuple(info.getPeerId(), info.getIp(), info.getPort()));
    }
    swarmManager->bitTorrentClient->addUnconnectedPeers(
            this->announceRequest.getInfoHash(), trackerPeers);

    delete msg;
    msg = NULL;
}
void SwarmManagerThread::timerExpired(cMessage *timer) {
    // TODO check this out
    if (this->sock->getState() == TCPSocket::CONNECTED
            || this->sock->getState() == TCPSocket::CONNECTING) {
        // avoid a event-driven announce to be called at the same time a
        // periodic is being called
        scheduleAt(simTime() + 1, timer);
    } else {
        // renew the socket in the host's socket map
        TCPSocketMap & socketMap =
                static_cast<SwarmManager*>(this->hostmod)->socketMap;
        socketMap.removeSocket(this->sock);
        this->sock->renewSocket();
        socketMap.addSocket(this->sock);

        SwarmManager* swarmManager = static_cast<SwarmManager*>(this->hostmod);
        this->sock->connect(this->trackerAddress, this->trackerPort);
    }
}
void SwarmManagerThread::closed() {
    // TODO delete socket from map?
    // schedule the next periodic announce message when not seeding
    cancelEvent(&this->requestTimer);
    if (!this->seeder) {
        scheduleAt(simTime() + this->refreshInterval, &this->requestTimer);
    }
}
void SwarmManagerThread::failure(int code) {
    // renew socket and retry to connect
    this->sock->abort();
}

// Private methods
char const* SwarmManagerThread::getEventStr(ANNOUNCE_TYPE type) {
    char const* evStr;
    switch (type) {
    case A_STARTED:
        evStr = "started";
        break;
    case A_STOPPED:
        evStr = "stopped";
        break;
    case A_COMPLETED:
        evStr = "completed";
        break;
    default:
        evStr = "";
        break;
    }
    return evStr;
}
