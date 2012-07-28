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

#ifndef __CHOKER_H__
#define __CHOKER_H__

#include <omnetpp.h>
#include <vector>

class BitTorrentClient;
class PeerStatus;

/**
 * The Choker chokes and unchokes Peers connected to the Client.
 * The choking is performed in rounds, which occurs every
 * ten seconds, when a Peer leaves the peer set or when an
 * unchoked peer becomes changes interest.
 * The peer selection algorithm changes depending on the state
 * of the BitTorrentApp (that is, leecher and seeder states).
 *
 * The algorithm was acquired from (U. Guillaume and M. Pietro,
 * "Understanding BitTorrent: An Experimental Perspective",
 * Technical report, INRIA Sophia Antipolis / Institut Eurecom, November 2005.)
 *
 * In leecher state, the following algorithm is used to select peers
 * to unchoke:
 * <ol>
 *      <li> At the beginning of every three rounds, i.e., every 30 seconds,
 *           the algorithm chooses one peer at random that is choked and interested.
 *           This is called the optimistic unchoked peer.
 *      <li> The algorithm orders peers that are interested and not snubbed (a peer that
 *           has not send any block in the last 30 seconds is called snubbed) according
 *           to their download rate (to the Client).
 *      <li> The three fastest peers are unchoked.
 *      <li> If the planned optimistic unchoke is not part of the three fastest peers, it
 *           is unchoked and the round is completed.
 *      <li> If the planned optimistic unchoke is part of the three fastest peers, another
 *           planned optimistic unchoke is choken at random.
 *      <li>
 *          <ul>
 *              <li> If the peer is interested, it is unchoked and the round is completed.
 *              <li> If the peer is not interested, it is unchoked and a new planned
 *              optimistic unchoke is chosen at random. The previous step is repeated with
 *              the new planned optimistic unchoke. The consequence of this behavior is that
 *              there can be more than four unchoked peers, but only four can be unchoked
 *              AND interested. This way, the active peer set can be computed as soon as
 *              one of the unchoked and not interested peers becomes interested.
 *          </ul>
 * </ol>
 *
 * In seeder state, the following algorithm is used to select peers to unchoke:
 * <ol>
 *      <li> The algorithm selects all peers that were recently unchoked (less than 20 seconds ago)
 *      or that have pending requests for blocks and orders them according to the time
 *      they were last unchoked. If two peers were unchoked at the same time, they are ordered
 *      by their upload rate (to the Client), giving priority to the highest upload.
 *      <li> The algorithm orders the other peers according to their upload rate
 *      (to the Client), giving priority to the highest upload, and puts them after the peers
 *      ordered in the previous step.
 *      <li> During two out of three rounds, the algorithm keeps unchoked the first 3 peers,
 *      and unchoke another interested peer at random. For the third round, the algorithm
 *      keeps unchoked the first four peers.
 * </ol>
 *
 * When there are not enough connected peers to fill the upload slots, all of them
 * start unchoked. On the other hand, if there are more peers than there are upload
 * slots, they will start choked. In this way, the first peers to connect will
 * start downloading as soon as it becomes interested in the Client.
 */
class Choker: public cSimpleModule {
public:
    Choker();
    virtual ~Choker();
    void callChokeAlgorithm();
    void stopChokerRounds();
    /*!
     * If there are upload slots available, unchoke the passed Peer. First use
     * the regular slots, then the optimistic slots. When an optimistic slots is
     * used, start choke round timer. This method is called to fill up the
     * upload slots while there aren't enough connections to justify choking.
     *
     * @param peerId An interested but choked Peer.
     */
    void addInterestedPeer(int peerId);
    void removePeer(int peerId);
private:
    //!@name Pointers to other modules
    //@{
    BitTorrentClient *bitTorrentClient;
    //@}

    simtime_t roundInterval;
    unsigned int optimisticRoundRateInLeech;
    unsigned int optimisticRoundRateInSeed;
    unsigned int numRegular;
    unsigned int numOptimistic;
    unsigned int numberOfUploadSlots;
    long optimisticCounter;

    //! Set to true to print debug messages
    bool debugFlag;
    //! The infoHash of the swarm this ContentManager is serving.
    int infoHash;
    //! The peerId of this Client.
    int localPeerId;

    //!@name Upload slot vectors.
    //! A value of -1 represents an empty slot.
    //@{
    std::set<int> regularSlots;
    std::set<int> optimisticSlots;
    //@}

    cMessage roundTimer;
    //! Used to avoid multiple calls to the choke algorithm in the same event.
    cMessage chokeAlgorithmTimer;
private:
    void chokeAlgorithm(bool optimisticRound = false);
    typedef std::vector<PeerStatus const*>::iterator PeerVectorIt;
    /*!
     * Fill the regular slots with the fastest interested peers that are not
     * optimistically unchoked.
     *
     * Will increment the iterator until (it == end) or until the upload slots
     * are full.
     */
    void regularUnchoke(PeerVectorIt & it, PeerVectorIt const& end);
    /*!
     * Fill the optimistic upload slots with randomly selected interested peers.
     *
     * Will increment the iterator until (it == end) or until the optimistic
     * slots are full.
     */
    void optimisticUnchoke(PeerVectorIt & it, PeerVectorIt & end);
    void printUploadSlots();
    //! Print a debug message to clog.
    void printDebugMsg(std::string s);
    void updateStatusString();
protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

#endif
