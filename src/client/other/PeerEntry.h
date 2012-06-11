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
 * PeerEntr.h
 *
 *  Created on: Feb 2, 2010
 *      Author: pevangelista
 */

#ifndef PEERENTR_H_
#define PEERENTR_H_

#include <omnetpp.h>
#include <ostream>
#include <string>
#include "DataRateRollingAverage.h"

class PeerWireThread;

/*!
 * Representation of a BitTorrent Peer.
 *
 * Each Peer should be uniquely identified by its Id string, but for BitTorrent the Id doesn't matter,
 * since each Peer has a unique IP address and port. This means that the peerId can be left empty, and it
 * is not used to find a Peer in the ConnectedPeerManager.
 *
 * A connected Peer must have a pointer to the corresponding PeerWireThread.
 */
class PeerEntry {
public:
    PeerEntry(int peerId, PeerWireThread* thread);

    //! Return a string representation of this PeerEntry object.
    std::string str() const;

    //!@name PeerEntry main attributes.
    //@{
    //! Return the Id of the PeerEntry.
    int getPeerId() const;
    //! Return pointer to the thread opened when this Peer connected with the client.
    PeerWireThread* getThread() const;
    //@}

    //!@name Setters for the attributes used when sorting the PeerEntry'es in the ConnectedPeerManager.
    //@{
    //! Set to true if the Peer is snubbing the Client (did not answer any piece requests in the last minute).
    void setSnubbed(bool snubbed);
    //! Return true if the Peer is snubbing the Client (did not answer any piece requests in the last minute).
    bool isSnubbed() const;
    //! Set to true if the Peer showed interest in the Client.
    void setInterested(bool interested);
    //! Return true if the Peer is interested in the Client.
    bool isInterested() const;
    //! Set to true if the Peer was unchoked by the Client and the time the Peer was unchoked.
    void setUnchoked(bool unchoked, simtime_t const& now);
    //! Return true if the Peer was unchoked by the Client.
    bool isUnchoked() const;
    //! Set to true if the Peer was not recently unchoked.
    void setOldUnchoked(bool oldUnchoked);
    //! Set how many bytes arrived FROM the Peer since the last call.
    void setBytesDownloaded(double now, int bytesDownloaded);
    //! Set how many bytes were sent TO the Peer since the last call.
    void setBytesUploaded(double now, int bytesUploaded);
    //! Return the download rate that was collected last.
    double getDownloadRate() const;
    //! Return the upload rate that was collected last.
    double getUploadRate() const;
    //@}

    //!@name Static methods used to sort a list of PeerEntry'es
    //@{
    //! Return true if lhs comes before rhs.
    static bool sortByDownloadRate(const PeerEntry* lhs, const PeerEntry* rhs);
    //! Return true if lhs comes before rhs.
    static bool sortByUploadRate(const PeerEntry* lhs, const PeerEntry* rhs);
    //@}
private:
    /*!
     * The peerId of this peer. The specification says the peerId is a 20-byte string,
     * but to simplify the code development, the peerId of the module is used instead,
     * since it is guaranteed to be unique.
     */
    int peerId;
    //! Pointer to the thread created when the Peer connected with the client.
    PeerWireThread* thread;

    //! True if the Peer is snubbed.
    bool snubbed;
    //! True if the Peer is interested in the Client.
    bool interested;
    //! True if the Peer is unchoked by the Client.
    bool unchoked;
    //! False if the Peer was recently unchoked by the Client.
    bool oldUnchoked;
    //! The time the last unchoke occurred.
    simtime_t timeOfLastUnchoke;

    //! Used to calculate the upload data rate
    DataRateRollingAverage uploadDataRate;
    //! Used to calculate the download data rate
    DataRateRollingAverage downloadDataRate;
};

//! Output operator used by the WATCH_SET macro to print info about the PeerEntry.
std::ostream& operator<<(std::ostream& os, PeerEntry const& peer);

#endif /* PEERENTR_H_ */

