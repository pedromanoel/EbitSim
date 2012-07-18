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
 * PeerEntr.cc
 *
 *  Created on: Feb 2, 2010
 *      Author: pevangelista
 */

#include "PeerStatus.h"

// PeerStatus method implementations
PeerStatus::PeerStatus(int peerId, PeerWireThread* thread) :
        peerId(peerId), thread(thread), snubbed(false), interested(false),
            unchoked(false), oldUnchoked(false), timeOfLastUnchoke(0) {
}

std::string PeerStatus::str() const {
    std::ostringstream out;
    out << "obj address: " << (void *) this << "\n";

    out << (this->snubbed ? "snubbed" : "not snubbed") << "\n";
    out << (this->interested ? "interested" : "not interested") << "\n";
    out << (this->unchoked ? "unchoked" : "choked") << "\n";
    out << (this->oldUnchoked ? "old unchoked" : "recently unchoked") << "\n";

    out << "last unchoke: " << this->timeOfLastUnchoke << "s\n";
    out << "download rate: " << this->downloadDataRate.getDataRateAverage()
        << "kpbs, upload rate: ";
    out << this->uploadDataRate.getDataRateAverage() << "bps";

    return out.str();
}

int PeerStatus::getPeerId() const {
    return this->peerId;
}
PeerWireThread* PeerStatus::getThread() const {
    return this->thread;
}

// Attributes used when sorting.
void PeerStatus::setSnubbed(bool snubbed) {
    this->snubbed = snubbed;
}
bool PeerStatus::isSnubbed() const {
    return this->snubbed;
}
void PeerStatus::setInterested(bool interested) {
    this->interested = interested;
}
bool PeerStatus::isInterested() const {
    return this->interested;
}
void PeerStatus::setUnchoked(bool unchoked, simtime_t const& now) {
    if (unchoked) {
        this->timeOfLastUnchoke = now;
    }
    this->unchoked = unchoked;
}
bool PeerStatus::isUnchoked() const {
    return this->unchoked;
}
void PeerStatus::setOldUnchoked(bool oldUnchoked) {
    this->oldUnchoked = oldUnchoked;
}
void PeerStatus::setBytesDownloaded(double now, unsigned long bytesDownloaded) {
    this->downloadDataRate.collect(now, bytesDownloaded);
}
void PeerStatus::setBytesUploaded(double now, unsigned long bytesUploaded) {
    this->uploadDataRate.collect(now, bytesUploaded);
}
double PeerStatus::getDownloadRate() const {
    return this->downloadDataRate.getDataRateAverage();
}
double PeerStatus::getUploadRate() const {
    return this->uploadDataRate.getDataRateAverage();
}

//Sorting methods
/*!
 * Only interested and not snubbed (sent at least one piece in the last 30 seconds)
 * are considered. That means that if one of the PeerStatus'es is not interested or snubbed,
 * than it will come after the other.
 * lhs and rhs are compared based on the download rate from the Client's point of view.
 */
bool PeerStatus::sortByDownloadRate(const PeerStatus* lhs, const PeerStatus* rhs) {

    // verify if lhs is interested and not snubbed. If not, rhs comes first.
    if (!lhs->interested || lhs->snubbed) {
        return false;
    }
    // verify if rhs is interested and not snubbed. If not, lhs comes first.
    if (!rhs->interested || rhs->snubbed) {
        return true;
    }
    return lhs->downloadDataRate.getDataRateAverage()
        > rhs->downloadDataRate.getDataRateAverage();
}
/*!
 * Only peers that are unchoked and interested are considered. That means that
 * if one of the PeerStatus objects is choked or not interested, than it will come
 * after the other.
 * The PeerStatus lhs and rhs are compared by the following criteria, in the order
 * presented. If lhs is equal to rhs in one of the criteria, then the following is
 * used, until lhs can be determined to be before or after rhs.
 * <ol>
 *      <li> Unchoked and interested comes first.
 *      <li> Recently unchoked (less than 20 second ago).
 *      <li> Most recently unchoked comes first.
 *      <li> Highest upload rate, from the Client's point of view, comes first.
 * </ol>
 *
 * The algorithm orders peers according to the time they were last unchoked (most recently
 * unchoked first) for all peers that were unchoked recently (less than 20 seconds ago) or that
 * have pending requests. The upload rate is used to decide between peers with the same last unchoked
 * time, giving priority to the highest upload. All other peers are ordered according to their
 * upload rate, giving priority to the highest upload.
 */
bool PeerStatus::sortByUploadRate(const PeerStatus* lhs, const PeerStatus* rhs) {
    // verify if lhs is unchoked and interested. If not, rhs comes first.
    if (!lhs->interested || !lhs->unchoked) {
        return false;
    }
    // verify if rhs is unchoked and interested. If not, lhs comes first.
    if (!rhs->interested || !rhs->unchoked) {
        return true;
    }

    // verify if lhs is recently unchoked or has pending requests and rhs not.
    // if so, lhs comes first.
    if ((!lhs->oldUnchoked) ^ (!rhs->oldUnchoked)) { // xor
        return (!lhs->oldUnchoked);
    }

    // verify if lhs was unchoked after rhs. If so, lhs comes first.
    if (!lhs->oldUnchoked) { // if true, it means lhs and rhs are recently unchoked or has pending requests.
        if (lhs->timeOfLastUnchoke < rhs->timeOfLastUnchoke) {
            return true;
        } else if (lhs->timeOfLastUnchoke > rhs->timeOfLastUnchoke) {
            return false;
        }
    }
    // both have the same unchoke time or are not recently unchoked. Order by upload rate.
    return lhs->uploadDataRate.getDataRateAverage()
        > rhs->uploadDataRate.getDataRateAverage();
}

/*!
 * Print information about this PeerStatus.
 */
std::ostream& operator<<(std::ostream& os, PeerStatus const& peer) {
    os << peer.str().c_str();
    return os;
}
