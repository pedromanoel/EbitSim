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
 * PeerInfo.cc
 *
 *  Created on: Mar 31, 2010
 *      Author: pevangelista
 */

#include "PeerInfo.h"

PeerInfo::PeerInfo() :
        peerId(-1), port(-1) {
}

PeerInfo::PeerInfo(int peerId, IPvXAddress const& ip, int port) :
        peerId(peerId), ip(ip), port(port), status(A_NORMAL) {
}

PeerInfo::~PeerInfo() {
}

/*!
 * The bencoded info is used to calculate the Tracker response message size.
 * The peer_id returned is not the actual Peer id, but a fixed 20-bytes string.
 * This is a simplification to avoid having to create a peer id string following
 * the specification, which would not be useful or necessary.
 */
std::string PeerInfo::getBencodedInfo() const {
    std::ostringstream bencodedResponse;
    bencodedResponse << "d7:peer_id20:_notTheActualPeerId_";
    bencodedResponse << "2:ip" << this->ip.str().length() << ":" << this->ip;
    bencodedResponse << "4:porti" << this->port << "ee";

    return bencodedResponse.str();
}

int PeerInfo::getPeerId() const {
    return this->peerId;
}
IPvXAddress const& PeerInfo::getIp() const {
    return this->ip;
}
int PeerInfo::getPort() const {
    return this->port;
}
void PeerInfo::setStatus(AnnounceType status) {
    this->status = status;
}
AnnounceType PeerInfo::getStatus() {
    return status;
}

bool PeerInfo::operator==(PeerInfo const& peerInfo) const {
    return ((this->peerId == peerInfo.peerId) && (this->ip == peerInfo.ip)
            && (this->port == peerInfo.port));
}
bool PeerInfo::operator!=(PeerInfo const& peerInfo) const {
    return !this->operator==(peerInfo);
}
bool PeerInfo::operator<(PeerInfo const& peerInfo) const {
    return this->peerId < peerInfo.peerId || // first is less
            (!(this->peerId < peerInfo.peerId) && this->ip < peerInfo.ip) || // first is equal, second is less
            (!(this->peerId < peerInfo.peerId) && !(this->ip < peerInfo.ip)
                    && this->port < peerInfo.port); // first and second are equal, third is less
}

std::ostream& operator<<(std::ostream& out, PeerInfo const& peerInfo) {
    out << "Peer id: \"" << peerInfo.getPeerId();
    out << "\", Ip: " << peerInfo.getIp();
    out << ", Port: " << peerInfo.getPort();
    return out;
}
