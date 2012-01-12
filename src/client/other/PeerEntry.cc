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

#include "PeerEntry.h"

// PeerEntry method implementations
PeerEntry::PeerEntry(int peerId, PeerWireThread* thread) :
    peerId(peerId), thread(thread), snubbed(false), interested(false),
            unchoked(false), oldUnchoked(false), timeOfLastUnchoke(0) {
    //    tcpIpConn(TcpIp()), peerId(-1), /*thread(NULL), */snubbed(false),
    //            interested(false), unchoked(false), oldUnchoked(false),
    //            timeOfLastUnchoke(0) {
}
//PeerEntry::PeerEntry(IPvXAddress const& remoteAddr, int remotePort, int peerId) :
//    remoteAddr(remoteAddr), remotePort(remotePort), peerId(peerId), snubbed(
//            false), interested(false), unchoked(false), oldUnchoked(false),
//            timeOfLastUnchoke(0) {
//    tcpIpConn(remoteAddr, remotePort), peerId(peerId), /*thread(NULL), */
//            snubbed(false), interested(false), unchoked(false), oldUnchoked(
//                    false), timeOfLastUnchoke(0) {
//}
//PeerEntry::PeerEntry(IPvXAddress const& localAddr, int localPort,
//        IPvXAddress const& remoteAddr, int remotePort, int peerId/*,
// PeerWireThread* thread*/) :
//    tcpIpConn(localAddr, localPort, remoteAddr, remotePort), peerId(peerId),
//    /*thread(thread), */snubbed(false), interested(false), unchoked(false),
//            oldUnchoked(false), timeOfLastUnchoke(0) {
//}
//PeerEntry::PeerEntry(TcpIp const& tcpIp, int peerId) :
//    tcpIpConn(tcpIp), peerId(peerId), snubbed(false), interested(false),
//            unchoked(false), oldUnchoked(false), timeOfLastUnchoke(0) {
//}
//bool PeerEntry::isInteresting(BitField const& bitField) {
//    return bitField.isBitFieldInteresting(this->bitField);
//}

std::string PeerEntry::str() const {
    std::ostringstream out;
    //    out << "id: " << this->peerId << "\n";
    //    out << "addr: " << this->remoteAddr.str() << ":" << this->remotePort << "\n";
    //    out << "conn: " << this->tcpIpConn.str() << "\n";
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

// Main attributes.
//void PeerEntry::setLocalAddress(IPvXAddress const& localIp, int localPort) {
//    if (!this->tcpIpConn.isConnected()) {
//        this->tcpIpConn.setOrigAddress(localIp, localPort);
//    }
//}
//IPvXAddress const& PeerEntry::getRemoteIp() const {
////    return this->tcpIpConn.getDestIp();
//    return this->remoteAddr;
//}
//int PeerEntry::getRemotePort() const {
////    return this->tcpIpConn.getDestPort();
//    return this->remotePort;
//}
//void PeerEntry::setTcpIp(TcpIp const& tcpIp) {
//    this->tcpIpConn = tcpIp;
//}
//TcpIp const& PeerEntry::getTcpIp() const {
//    return this->tcpIpConn;
//}
//void PeerEntry::setPeerId(int peerId) {
//    if (this->peerId == -1) {
//        this->peerId = peerId;
//    }
//}
int PeerEntry::getPeerId() const {
    return this->peerId;
}
//void PeerEntry::setThread(PeerWireThread* thread) {
//	this->thread = thread;
//}
PeerWireThread* PeerEntry::getThread() const {
    return this->thread;
}
//void PeerEntry::setBitField(BitField const& bitField) {
//    this->bitField = bitField;
//}

// Attributes used when sorting.
void PeerEntry::setSnubbed(bool snubbed) {
    this->snubbed = snubbed;
}
bool PeerEntry::isSnubbed() const {
    return this->snubbed;
}
void PeerEntry::setInterested(bool interested) {
    this->interested = interested;
}
bool PeerEntry::isInterested() const {
    return this->interested;
}
void PeerEntry::setUnchoked(bool unchoked, simtime_t const& now) {
    if (unchoked) {
        this->timeOfLastUnchoke = now;
    }
    this->unchoked = unchoked;
}
bool PeerEntry::isUnchoked() const {
    return this->unchoked;
}
void PeerEntry::setOldUnchoked(bool oldUnchoked) {
    this->oldUnchoked = oldUnchoked;
}
//void PeerEntry::setPendingRequests(bool pendingRequests) {
//    this->pendindRequests = pendingRequests;
//}
//void PeerEntry::setBlockDownload(int index) {
//    this->bitField.addPiece(index);
//}
void PeerEntry::setBytesDownloaded(double now, int bytesDownloaded) {
    this->downloadDataRate.collect(now, bytesDownloaded);
}
void PeerEntry::setBytesUploaded(double now, int bytesUploaded) {
    this->uploadDataRate.collect(now, bytesUploaded);
}
double PeerEntry::getDownloadRate() const {
    return this->downloadDataRate.getLastDataRate();
}
double PeerEntry::getUploadRate() const {
    return this->uploadDataRate.getLastDataRate();
}

/*!
 * The PeerEntry objects are compared first by their dest address. If
 * the orig address is set and the dest addresses are equal, they are
 * compared by their orig address.
 */
//bool PeerEntry::operator<(PeerEntry const& peer) const {
////    return this->tcpIpConn < peer.tcpIpConn;
//    return this->remoteAddr < peer.remoteAddr;
//}
/*!
 * The PeerEntry objects are compared first by their dest address. If
 * the orig address is set and the dest addresses are equal, they are
 * compared by their orig address.
 */
//bool PeerEntry::operator>(PeerEntry const& peer) const {
////    return this->tcpIpConn > peer.tcpIpConn;
//    return !this->operator<(peer) && !this->operator ==(peer);
//}
/*!
 * The PeerEntry objects are compared first by their dest address.
 * If the orig address is set, they are also compared by their orig
 * address.
 */
//bool PeerEntry::operator==(PeerEntry const& peer) const {
////    return this->tcpIpConn == peer.tcpIpConn;
//    return this->remoteAddr == peer.remoteAddr;
//}
/*!
 * The PeerEntry objects are compared first by their dest address.
 * If the orig address is set, they are also compared by their orig
 * address.
 */
//bool PeerEntry::operator!=(PeerEntry const& peer) const {
////    return this->tcpIpConn != peer.tcpIpConn;
//    return this->remoteAddr != peer.remoteAddr;
//}

//Sorting methods
/*!
 * Only interested and not snubbed (sent at least one piece in the last 30 seconds)
 * are considered. That means that if one of the PeerEntry'es is not interested or snubbed,
 * than it will come after the other.
 * lhs and rhs are compared based on the download rate from the Client's point of view.
 */
bool PeerEntry::sortByDownloadRate(const PeerEntry* lhs, const PeerEntry* rhs) {

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
 * if one of the PeerEntry objects is choked or not interested, than it will come
 * after the other.
 * The PeerEntry lhs and rhs are compared by the following criteria, in the order
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
bool PeerEntry::sortByUploadRate(const PeerEntry* lhs, const PeerEntry* rhs) {
    // verify if lhs is unchoked and interested. If not, rhs comes first.
    if (!lhs->interested || !lhs->unchoked) {
        return false;
    }
    // verify if rhs is unchoked and interested. If not, lhs comes first.
    if (!rhs->interested || !rhs->unchoked) {
        return true;
    }

    // verify if the peers where unchoked in the last 20 seconds.
    //    bool lhsRecent, rhsRecent;
    //    simtime_t now = simTime(); // get the simulation time.
    //    lhsRecent = ((now - lhs->timeOfLastUnchoke) < 20.0);
    //    rhsRecent = ((now - rhs->timeOfLastUnchoke) < 20.0);

    // verify if lhs is recently unchoked or has pending requests and rhs not.
    // if so, lhs comes first.
    if ((!lhs->oldUnchoked/* || lhs->pendindRequests*/) ^ (!rhs->oldUnchoked
    /*|| rhs->pendindRequests*/)) { // xor
        return (!lhs->oldUnchoked/* || lhs->pendindRequests*/);
    }

    // verify if lhs was unchoked after rhs. If so, lhs comes first.
    if (!lhs->oldUnchoked/* || lhs->pendindRequests*/) {// if true, it means lhs and rhs are recently unchoked or has pending requests.
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
 * Print information about this PeerEntry.
 *
 * Used by the WATCH_MAP macro defined by the OMNeT++ API.
 */
std::ostream& operator<<(std::ostream& os, PeerEntry const& peer) {
    os << peer.str().c_str();
    return os;
}
