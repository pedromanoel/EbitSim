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

#include "Choker.h"

#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <boost/lexical_cast.hpp>

#include "SwarmManager.h"
#include "BitTorrentClient.h"
#include "PeerStatus.h"

Define_Module(Choker);

namespace {
std::string toStr(int i) {
    return boost::lexical_cast<std::string>(i);
}
}

enum ChokerMessageType {
    ROUND_TIMER
};

Choker::Choker() :
    bitTorrentClient(NULL), roundInterval(0), optimisticRoundRateInLeech(0), optimisticRoundRateInSeed(
        0), numRegular(0), numOptimistic(0), numberOfUploadSlots(0), optimisticCounter(
        0), debugFlag(false), infoHash(-1), localPeerId(-1), roundTimer(
        "RoundTimer"), chokeAlgorithmTimer("ChokeAlgorithmTimer") {
}

Choker::~Choker() {
    cancelEvent(&this->roundTimer);
    cancelEvent(&this->chokeAlgorithmTimer);
    this->printDebugMsg("Deleting Choker module");
}
void Choker::callChokeAlgorithm() {
    Enter_Method("callChokeAlgorithm()");
    // Check if this method was not called more than once in the same event.
    if (!this->chokeAlgorithmTimer.isScheduled()) {
        scheduleAt(simTime(), &this->chokeAlgorithmTimer);
    }
}
void Choker::stopChokerRounds() {
    Enter_Method("stopChoker()");
    this->printDebugMsg("Stop choke rounds");
    cancelEvent(&this->roundTimer);
}

void Choker::addInterestedPeer(int peerId) {
    Enter_Method("addPeer(peer: %d)", peerId);
    if (this->regularSlots.size() < this->numRegular) {
        this->regularSlots.insert(peerId);
        this->bitTorrentClient->unchokePeer(this->infoHash, peerId);

        std::ostringstream out;
        out << "Peer " << peerId;
        out << " unchoked to fill the regular upload slots";
        this->printDebugMsg(out.str());
    } else if (this->optimisticSlots.size() < this->numOptimistic) {
        this->optimisticSlots.insert(peerId);
        this->bitTorrentClient->unchokePeer(this->infoHash, peerId);

        std::ostringstream out;
        out << "Peer " << peerId;
        out << " unchoked to fill the optimistic upload slots";
        this->printDebugMsg(out.str());
    }
    // Start the rounds
    if (!this->roundTimer.isScheduled()) {
        this->printDebugMsg("Start the choke round timer");
        scheduleAt(simTime() + this->roundInterval, &this->roundTimer);
    }
    this->printUploadSlots();
}
void Choker::removePeer(int peerId) {
    Enter_Method("removePeer(peerId: %d)", peerId);

    std::vector<int>::iterator it;

    int count = this->regularSlots.erase(peerId);
    count += this->optimisticSlots.erase(peerId);

    // call the next choke round
    if (count > 0) {
        std::ostringstream out;
        out << "removing peer " << peerId << " from upload slot";
        this->printDebugMsg(out.str());
    }
}

void Choker::chokeAlgorithm(bool optimisticRound) {
    // order the peers differently depending on the client's state
    PeerVector orderedPeers =
        par("seeder").boolValue() ?
            this->bitTorrentClient->getFastestToUpload(this->infoHash) :
            this->bitTorrentClient->getFastestToDownload(this->infoHash);

    // Do nothing if the list is empty
    if (orderedPeers.empty()) {
        return;
    }
    // choke/unchoke the peers.
    PeerVectorIt it = orderedPeers.begin();
    PeerVectorIt end = orderedPeers.end();

    regularUnchoke(it, end);
    if (optimisticRound || this->optimisticSlots.empty()) {
        optimisticUnchoke(it, end);
    }

    // All other interested peers are choked, except for the optimistic unchoke
    for (; it != end; ++it) {
        PeerStatus const* peer = *it;
        int peerId = peer->getPeerId();
        if (!this->optimisticSlots.count(peerId) && peer->isInterested()) {
            this->bitTorrentClient->chokePeer(this->infoHash, peerId);
            std::string out = "Choke peer " + toStr(peerId);
            this->printDebugMsg(out);
        }
    }
    this->printUploadSlots();
}
void Choker::regularUnchoke(PeerVectorIt & it, PeerVectorIt const& end) {
    this->regularSlots.clear();
    std::string out = "Regular unchokes - ";
    for (; it != end && this->regularSlots.size() < this->numRegular; ++it) {
        PeerStatus const* peer = (*it);
        int peerId = peer->getPeerId();

        // don't consider optimistically unchoked peers
        if (this->optimisticSlots.count(peerId)) {
            continue;
        }
        // remove snubbed connections from the active slots
        if (peer->isSnubbed()) {
            this->bitTorrentClient->chokePeer(this->infoHash, peerId);
            out += "choke_snubbed(" + toStr(peerId) + ") ";
        } else {
            // only interested peers can occupy upload slots
            if (peer->isInterested()) {
                this->regularSlots.insert(peerId);
                out += "interested(" + toStr(peerId) + ") ";
            } else {
                out += "not-interested(" + toStr(peerId) + ") ";
            }

            // Unchoke the fastest peers until the regular slots are
            // filled. When the not interested peers become interested,
            // a choke round will be issued.
            this->bitTorrentClient->unchokePeer(this->infoHash, peerId);
        }
    }
    this->printDebugMsg(out);
}
void Choker::optimisticUnchoke(PeerVectorIt & it, PeerVectorIt & end) {
    this->optimisticSlots.clear();
    // Shuffle the remaining Peers, since optimistic unchokes are random.
    std::random_shuffle(it, end, intrand);
    // run until all of the upload slots are occupied or until there are
    // no more peers to unchoke
    std::string out = "Optimistic unchokes - ";
    for (; it != end && this->optimisticSlots.size() < this->numOptimistic;
        ++it) {
        PeerStatus const* peer = (*it);
        int peerId = peer->getPeerId();

        // The top interested Peers that are not snubbed are optimistically unchoked
        if (peer->isInterested()) {
            if (peer->isSnubbed()) {
                this->bitTorrentClient->chokePeer(this->infoHash, peerId);
                out += "choke_snubbed(" + toStr(peerId) + ") ";
            } else {
                this->optimisticSlots.insert(peerId);
                this->bitTorrentClient->unchokePeer(this->infoHash, peerId);
                out += "unchoke(" + toStr(peerId) + ") ";
            }
        }
    }
    this->printDebugMsg(out);
}
void Choker::printUploadSlots() {
    std::ostringstream out;
    std::ostream_iterator<int> out_it(out, " ");
    out << "regular upload slots: ";
    std::copy(this->regularSlots.begin(), this->regularSlots.end(), out_it);
    out << ", optimistic upload slots: ";
    std::copy(this->optimisticSlots.begin(), this->optimisticSlots.end(),
        out_it);
    this->printDebugMsg(out.str());
}
void Choker::printDebugMsg(std::string s) {
    if (this->debugFlag) {
        // debug "header"
        std::cerr << simulation.getEventNumber() << " (T=";
        std::cerr << simulation.getSimTime() << ")(Choker) - ";
        std::cerr << "Peer " << this->localPeerId;
        std::cerr << ": infoHash " << this->infoHash << " - ";
        std::cerr << s << "\n";
    }
}
void Choker::updateStatusString() {
    //    if (ev.isGUI()) {
    //        std::ostringstream out_status;
    //        getDisplayString().setTagArg("t", 0, out_status.str().c_str());
    //    }
}

void Choker::initialize() {
    // will throw an error if this module is not owned by a BitTorrentclient
    this->bitTorrentClient = check_and_cast<BitTorrentClient*>(
        getParentModule());
    this->localPeerId = this->bitTorrentClient->getLocalPeerId();

    this->roundInterval = par("roundInterval");
    this->optimisticRoundRateInLeech =
        par("optimisticRoundRateInLeech").longValue();
    this->optimisticRoundRateInSeed =
        par("optimisticRoundRateInSeed").longValue();

    this->numRegular = par("numberOfRegularSlots").longValue();
    this->numOptimistic = par("numberOfOptimisticSlots").longValue();
    this->numberOfUploadSlots = this->numRegular + this->numOptimistic;
    this->infoHash = par("infoHash").longValue();

//    scheduleAt(simTime() + this->roundInterval, &this->roundTimer);

    this->debugFlag = par("debugFlag").boolValue();
}

void Choker::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage() && msg == &this->roundTimer) {
        bool optimisticRound;

        if (par("seeder").boolValue()) {
            // every 'optimisticRoundRateInSeed' rounds, do not optimistic unchoke
            optimisticRound = !!(this->optimisticCounter
                % this->optimisticRoundRateInSeed);
        } else {
            // Every 'optimisticRoundRateInLeech' rounds, optimistic unchoke
            optimisticRound = !(this->optimisticCounter
                % this->optimisticRoundRateInLeech);
        }

        std::string out = (optimisticRound ? "Optimistic " : "Normal ")
            + std::string("choke round");
        this->printDebugMsg(out);

        this->chokeAlgorithm(optimisticRound);

        // re-eschedule the choke round timer
        scheduleAt(simTime() + this->roundInterval, &this->roundTimer);
        ++this->optimisticCounter;
    } else if (msg->isSelfMessage() && msg == &this->chokeAlgorithmTimer) {
        // Call the choke algorithm without changing the round counter.
        this->printDebugMsg("Extra choke call");
        this->chokeAlgorithm();
    } else {
        this->printDebugMsg("Whaaaa?");
    }
}
