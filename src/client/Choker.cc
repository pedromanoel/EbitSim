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

#include "SwarmManager.h"
#include "BitTorrentClient.h"
#include "PeerEntry.h"

Define_Module(Choker);

enum ChokerMessageType {
    ROUND_TIMER
};

Choker::Choker() :
        roundInterval(0), optimisticRoundRateInLeech(0), optimisticRoundRateInSeed(
                0), numberOfRegularSlots(0), numberOfOptimisticSlots(0), numberOfUploadSlots(
                0), optimisticCounter(0), debugFlag(false), infoHash(-1), localPeerId(
                -1), roundTimer("RoundTimer"), chokeAlgorithmTimer(
                "ChokeAlgorithmTimer"), bitTorrentClient(NULL) {
}

Choker::~Choker() {
    cancelEvent(&this->roundTimer);
    cancelEvent(&this->chokeAlgorithmTimer);
}
void Choker::callChokeAlgorithm() {
    Enter_Method("callChokeAlgorithm()");
    // check if its not gonna be called already. No need to call it two times.
    if (!this->chokeAlgorithmTimer.isScheduled()) {
        scheduleAt(simTime(), &this->chokeAlgorithmTimer);
    }
}
void Choker::stopChoker() {
    Enter_Method("stopChoker()");
    cancelEvent(&this->roundTimer);
}

void Choker::fillUploadSlots(int peerId) {
    Enter_Method("fillUploadSlots()");

    std::ostringstream out;

    out << "Peer " << peerId;

    if (this->regularUploadSlots.size() < this->numberOfRegularSlots) {
        this->regularUploadSlots.insert(peerId);
        this->bitTorrentClient->unchokePeer(this->infoHash, peerId);

    } else {
        out << " NOT";
    }

    out << " unchoked to fill the upload slots";

    int usedRegularSlots = this->regularUploadSlots.size();
    int totalRegularSlots = this->numberOfRegularSlots;
    out << " Upload slots (" << usedRegularSlots << "/" << totalRegularSlots
            << ")";

    this->printDebugMsg(out.str());

}
void Choker::removePeerInfo(int peerId) {
    Enter_Method("removePeerInfo(peerId: %d)", peerId);

    std::vector<int>::iterator it;

    int count = this->regularUploadSlots.erase(peerId);
    count += this->optimisticUploadSlots.erase(peerId);

    // call the next choke round
    if (count > 0) {
        std::ostringstream out;
        out << "removing peer " << peerId << " from upload slot";
        this->printDebugMsg(out.str());
    }
}

void Choker::chokeAlgorithm(bool optimisticRound) {
    Enter_Method("callChokeAlgorithm(%s)",
            optimisticRound ? "optimistic" : "regular");
    PeerEntryPtrVector orderedPeers;

    this->printDebugMsg("Calling Choke Algorithm");

    if (par("seeder").boolValue()) {
        // order the peers by upload
        orderedPeers = this->bitTorrentClient->getFastestToUpload(
                this->infoHash);
    } else {
        // order the peers by download
        orderedPeers = this->bitTorrentClient->getFastestToDownload(
                this->infoHash);
    }

    // choke/unchoke the peers.
    if (!orderedPeers.empty()) {
        PeerEntryPtrVectorIt begin = orderedPeers.begin();
        PeerEntryPtrVectorIt end = orderedPeers.end();

        this->regularUploadSlots.clear();

        // unchoke the fastest peers and put the interested ones in the regular upload slots
        // run until all of the upload slots are occupied or until there are no more
        // peers to unchoke
        for (;
                begin != end
                        && this->regularUploadSlots.size()
                                < this->numberOfRegularSlots; ++begin) {
            PeerEntry const* peer = (*begin);
            int peerId = peer->getPeerId();

            // don't consider optimistically unchoked peers
            if (!this->optimisticUploadSlots.count(peerId)) {
                // remove snubbed connections from the active slots
                if (peer->isSnubbed()) {
                    this->bitTorrentClient->chokePeer(this->infoHash, peerId);
                } else {
                    // only interested peers can occupy upload slots
                    if (peer->isInterested()) {
                        this->regularUploadSlots.insert(peerId);
                    }

                    // Unchoke all peers from the front of the vector until the upload
                    // slots are filled or until the list ends. Fast Peers that are not
                    // interested in the Client will also be unchoked. If they become
                    // interested, the downloader with the worst upload rate gets choked
                    // at the next choke round.
                    this->bitTorrentClient->unchokePeer(this->infoHash, peerId);
                }
            }
        }

        // fill the optimistic upload slots with interested peers
        if (optimisticRound) {
            this->optimisticUploadSlots.clear();
            // Shuffle the remaining Peers, since optimistic unchokes are random.
            std::random_shuffle(begin, end, intrand);
            // run until all of the upload slots are occupied or until there are no
            // more peers to unchoke
            for (;
                    begin != end
                            && this->optimisticUploadSlots.size()
                                    < this->numberOfOptimisticSlots; ++begin) {
                PeerEntry const* peer = (*begin);
                int peerId = peer->getPeerId();

                // The top interested Peers that are not snubbed are optimistically unchoked
                if (peer->isInterested()) {
                    if (peer->isSnubbed()) {
                        this->bitTorrentClient->chokePeer(this->infoHash,
                                peerId);
                    } else {
                        this->optimisticUploadSlots.insert(peerId);
                        this->bitTorrentClient->unchokePeer(this->infoHash,
                                peerId);
                    }
                }
            }
        }

        // Since only the peers at the upload slots can be interested and unchoked
        // at the same time, choke all remainining interested Peers. Peers that are
        // not interested don't need to be choked, since they will not try to upload
        // from the Client.
        for (; begin != end; ++begin) {
            PeerEntry const* peer = (*begin);
            if (peer->isInterested()) {
                this->bitTorrentClient->chokePeer(this->infoHash,
                        peer->getPeerId());
            }
        }

        {
            std::ostringstream out;
            std::ostream_iterator<int> out_it(out, " ");
            out << "regular upload slots: ";
            std::copy(this->regularUploadSlots.begin(),
                    this->regularUploadSlots.end(), out_it);
            out << ", optimistic upload slots: ";
            std::copy(this->optimisticUploadSlots.begin(),
                    this->optimisticUploadSlots.end(), out_it);
            this->printDebugMsg(out.str());
        }
    }
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

    // will throw an error if this module is not owned by a SwarmManager
    SwarmManager * swarmManager = check_and_cast<SwarmManager*>(
            getParentModule());
    this->localPeerId =
            swarmManager->getParentModule()->getParentModule()->getId();

    cModule *bitTorrentClient = swarmManager->getParentModule()->getSubmodule(
            "bitTorrentClient");

    if (bitTorrentClient == NULL) {
        throw cException("BitTorrentClient module not found");
    }
    this->bitTorrentClient = check_and_cast<BitTorrentClient*>(
            bitTorrentClient);

    this->roundInterval = par("roundInterval");
    this->optimisticRoundRateInLeech =
            par("optimisticRoundRateInLeech").longValue();
    this->optimisticRoundRateInSeed =
            par("optimisticRoundRateInSeed").longValue();

    this->numberOfRegularSlots = par("numberOfRegularSlots").longValue();
    this->numberOfOptimisticSlots = par("numberOfOptimisticSlots").longValue();
    this->numberOfUploadSlots = this->numberOfRegularSlots
            + this->numberOfOptimisticSlots;
    this->infoHash = par("infoHash").longValue();

    scheduleAt(simTime() + this->roundInterval, &this->roundTimer);

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

        std::ostringstream out;

        if (optimisticRound) {
            out << "Optimistic ";
        } else {
            out << "Normal ";
        }

        this->chokeAlgorithm(optimisticRound);

        out << " choke round";
        this->printDebugMsg(out.str());

        // re-eschedule the choke round timer
        scheduleAt(simTime() + this->roundInterval, &this->roundTimer);
        ++this->optimisticCounter;
    } else if (msg->isSelfMessage() && msg == &this->chokeAlgorithmTimer) {
        // choke/unchoke the peers.
        this->chokeAlgorithm();
    } else {
        this->printDebugMsg("Whaaaa?");
    }
}
