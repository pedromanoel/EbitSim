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

#ifndef __CONNECTIONMANAGER_H__
#define __CONNECTIONMANAGER_H__

#include <omnetpp.h>
#include <TCPSrvHostApp.h>

#include <boost/tuple/tuple_comparison.hpp>
//#include <boost/tuple/tuple.hpp> already included by tuple_comparison.hpp
using boost::tuple;
using boost::make_tuple;
using boost::tie;

#include "PeerEntry.h"

class PeerEntry;
class PeerInfo;
class PeerWireThread;
class SwarmManager;

//! Vector of PeerEntry pointers.
typedef std::vector<PeerEntry const*> PeerEntryPtrVector;
// list of unconnected Peers
typedef std::list<tuple<int, IPvXAddress, int> > UnconnectedList;
// Map of peer entries with peerId as key
typedef std::map<int, PeerEntry> PeerMap;
// Store the limits of a PeerMap (numActive, maxActive, numPassive, maxPassive)
//typedef tuple<int, int, int, int> MapLimits;
// Swarm (numActive, numPassive, PeerMap, UnconnectedList, seeding)
typedef tuple<int, int, PeerMap, UnconnectedList, bool> Swarm;
// map of swarms with infoHash as key
typedef std::map<int, Swarm> SwarmMap;

// Iterators
typedef PeerEntryPtrVector::iterator PeerEntryPtrVectorIt;
typedef UnconnectedList::iterator UnconnectedSetIt;
typedef PeerMap::iterator PeerMapIt;
typedef PeerMap::const_iterator PeerMapConstIt;
typedef SwarmMap::iterator SwarmMapIt;

/**
 * The BitTorrent Client implementation, which acts like a TCP server
 * when receiving connections from other Peers.
 *
 * Besides from that, it is also a TCP client, since it actively connects
 * to other peers.
 */
class BitTorrentClient: public TCPSrvHostApp {
public:
    //! TODO document this
    BitTorrentClient();
    //! TODO document this
    virtual ~BitTorrentClient();

    //!@name Methods used by the Choker
    //@{
    /*!
     * Choke the Peer.
     *
     * @param infoHash[in] The infoHash that identifies the swarm.
     * @param peerId[in] The id of the Peer.
     */
    void chokePeer(int infoHash, int peerId);
    /*!
     * Return a vector of pointers to PeerEntry objects, ordered by their upload
     * rate from the Client.
     *
     * @param infoHash[in] The infoHash that identifies the swarm.
     */
    PeerEntryPtrVector getFastestToUpload(int infoHash);
    /*!
     * Return a vector of pointers to PeerEntry objects, ordered by their
     * download rate to the Client.
     *
     * @param infoHash[in] The infoHash that identifies the swarm.
     */
    PeerEntryPtrVector getFastestToDownload(int infoHash) const;
    /*!
     * Unchoke the Peer.
     *
     * @param infoHash[in] The infoHash that identifies the swarm.
     * @param peerId[in] The id of the Peer.
     */
    void unchokePeer(int infoHash, int peerId);
    //@}

    //!@name Methods used by the ContentManager
    //@{
    /*!
     * Drop the connection with the Peer.
     *
     * @param infoHash[in] The infoHash that identifies the swarm.
     * @param peerId[in] The id of the Peer.
     */
    void closeConnection(int infoHash, int peerId);
    /*!
     * Change the swarm to seed status, warning that this Client is now seeding
     * content to this swarm.
     *
     * @param infoHash[in] The infoHash that identifies the swarm.
     */
    void finishedDownload(int infoHash);
    /*!
     * Show interest in the Peer.
     * @param infoHash[in] The infoHash that identifies the swarm.
     * @param peerId[in] The id of the Peer.
     */
    void peerInteresting(int infoHash, int peerId);
    /*!
     * Show lack of interest in the Peer.
     *
     * @param infoHash[in] The infoHash that identifies the swarm.
     * @param peerId[in] The id of the Peer.
     */
    void peerNotInteresting(int infoHash, int peerId);
    /*!
     * Send HaveMsg to all connected Peers warning that a piece was downloaded.
     *
     * @param infoHash[in] The infoHash that identifies the swarm.
     * @param pieceIndex[in] The index of the Piece that was downloaded.
     */
    void sendHaveMessage(int infoHash, int pieceIndex);
    //@}

    //!@name Methods used by the SwarmManager
    //@{
    /*!
     * Create the swarm.
     * A swarm is a list of Peers that have the same content.
     *
     * @param infoHash[in] The infoHash that identifies the swarm.
     * @param newSwarmSeeding[in] True if this Client is seeding for this swarm.
     */
    void addSwarm(int infoHash, bool newSwarmSeeding);
    /*!
     * Delete the swarm from the Client.
     *
     * @param infoHash[in] The infoHash that identifies the swarm.
     */
    void removeSwarm(int infoHash);
    //@}
private:
    /*!
     * Declare PeerWireThread a friend of  BitTorrentClient, since their
     * behavior are intimately connected.
     */
    friend class PeerWireThread;
    friend class SwarmManagerThread;

    //!@name Methods used by the SwarmManagerThread
    //@{
    //! Add the Peer to the ConnectedPeerManager as an unconnected Peer.
    void addUnconnectedPeers(int infoHash,
            std::list<tuple<int, IPvXAddress, int> > & peers);

    //!@name Methods used by the PeerWireThread
    //@{
    /*!
     * Tell the ConnectedPeerManager that a Peer established a TCP connection
     * with the client.
     * Return true if the connection can be established.
     */
    void addConnectedPeer(int infoHash, int peerId, PeerWireThread* thread,
            bool active);
    //! Calculate the download rate for the Peer with the passed peerId.
    void calculateDownloadRate(int infoHash, int peerId);
    //! Calculate the upload rate for the Peer with the passed peerId.
    void calculateUploadRate(int infoHash, int peerId);
    /*!
     * Return true if the Peer can connect with the Client. The connection will
     * not be possible if there are no connection slots available or if the Peer
     * is already connected.
     */
    bool canConnect(int infoHash, int peerId, bool active) const;
    /*!
     * Schedule the start of processing for the next Thread with messages
     * waiting.
     */
    void processNextThread();
    //! Remove this peer from the ConnectedPeerManager.
    void removePeerInfo(int infoHash, int peerId, int connId, bool active);
    //! Set to true if the Peer is interested in the Client.
    void setInterested(bool interested, int infoHash, int peerId);
    //! Set to true if the peer was not recently unchoked.
    void setOldUnchoked(bool oldUnchoke, int infoHash, int peerId);
    //! Set to true if the Client is snubbed by the Peer.
    void setSnubbed(bool snubbed, int infoHash, int peerId);
    //@}
private:
    //!@name Pointers to other modules
    //@{
    SwarmManager *swarmManager;
    //@}
    //!@name Thread processing
    //@{
    //! Contain all threads that requested the processor while it was busy.
    std::set<PeerWireThread*> waitingThreads;
    //@}
    //! Pointer to the thread currently utilizing the processor.
    std::set<PeerWireThread*>::iterator threadInProcessingIt;
    //!
    cMessage endOfProcessingTimer;
    //    cMessage processNextThreadTimer;
    //! Processing time histogram, used to generate values for the simulation
    cDoubleHistogram doubleProcessingTimeHist;
    /*!
     * This data structure is a map of Swarms, where a Swarm is the tuple of
     * number of active connections, number of passive connections, map of Peers
     * and list of unconnected Peers.
     * Structure:
     * SwarmMap => {infoHash: Swarm}
     * Swarm => {peerId: (numOfActive, numOfPassive, PeerMap, UnconnectedList)}
     * PeerMap => {peerId: PeerEntry}
     * UnconnectedList => [(peerId, IPvXAddress, port)]
     */
    SwarmMap swarmMap;
    /*!
     * Set with all active connections established or attempted by this Peer.
     * Peers are only removed from this list when the TCP connection closes.
     *
     * If there are connection slots available, the Client will actively connect
     * with other Peers in two situations:
     * 1) when an announce response arrives, or
     * 2) an active connection is closed.
     *
     * Structure: [(infoHash, peerId),]
     */
    std::set<std::pair<int, int> > activeConnectedPeers;
    /*!
     * Set with all threads created in the BitTorrentClient. Used when this
     * module is deleted at the end of the simulation to delete all unclosed
     * threads.
     */
    std::set<PeerWireThread*> allThreads;
    //!@name Timer intervals
    //@{
    //! The time, in seconds, to occur an snubbed timeout.
    simtime_t snubbedInterval;
    //! The time, in seconds, to occur a message timeout.
    simtime_t timeoutInterval;
    //! The time, in seconds, to occur a keep-alive timeout.
    simtime_t keepAliveInterval;
    //! The time, in seconds, to occur a download rate timeout.
    simtime_t downloadRateInterval;
    //! The time, in seconds, to occur a upload rate timeout.
    simtime_t uploadRateInterval;
    //@}
    //! The IP address of this Client.
    IPvXAddress localIp;
    //! The port of this Client.
    int localPort;
    //! The peerId of this Client.
    int localPeerId;
    // Set to true to print debug messages
    bool debugFlag;
    //! TODO document this
    int globalNumberOfPeers;
    //! TODO document this
    int numberOfActivePeers;
    //! TODO document this
    int numberOfPassivePeers;
    //!@name Signals and helper variables
    //@{
    unsigned int prevNumUnconnected;
    unsigned int prevNumConnected;
    simsignal_t numUnconnected_Signal;
    simsignal_t numConnected_Signal;

    simsignal_t processingTime_Signal;

    simsignal_t peerWireBytesSent_Signal;
    simsignal_t peerWireBytesReceived_Signal;
    simsignal_t contentBytesSent_Signal;
    simsignal_t contentBytesReceived_Signal;

    simsignal_t bitFieldSent_Signal;
    simsignal_t bitFieldReceived_Signal;
    simsignal_t cancelSent_Signal;
    simsignal_t cancelReceived_Signal;
    simsignal_t chokeSent_Signal;
    simsignal_t chokeReceived_Signal;
    simsignal_t handshakeSent_Signal;
    simsignal_t handshakeReceived_Signal;
    simsignal_t haveSent_Signal;
    simsignal_t haveReceived_Signal;
    simsignal_t interestedSent_Signal;
    simsignal_t interestedReceived_Signal;
    simsignal_t keepAliveSent_Signal;
    simsignal_t keepAliveReceived_Signal;
    simsignal_t notInterestedSent_Signal;
    simsignal_t notInterestedReceived_Signal;
    simsignal_t pieceSent_Signal;
    simsignal_t pieceReceived_Signal;
    simsignal_t requestSent_Signal;
    simsignal_t requestReceived_Signal;
    simsignal_t unchokeSent_Signal;
    simsignal_t unchokeReceived_Signal;
    //@}
private:
    //! TODO document this
    void attemptActiveConnections(Swarm & swarm, int infoHash);
    //! Open a TCP connection with this Peer.
    void connect(int infoHash, const tuple<int, IPvXAddress, int> & peer);
    //! Close the server socket so that other Peers cannot connect with the Client.
    void closeListeningSocket();
    //! Emit a signal corresponding to reception of a message with the passed message id.
    void emitReceivedSignal(int messageId);
    //! Emit a signal corresponding to sending of a message with the passed message id.
    void emitSentSignal(int messageId);
    //! TODO document this
    Swarm & getSwarm(int infoHash);
    const Swarm & getSwarm(int infoHash) const;
    //! TODO document this
    PeerEntry & getPeerEntry(int infoHash, int peerId);
    //! Open the server socket so that other Peers can connect with the Client.
    void openListeningSocket();
    /*!
     * Will emit signals for statistical purposes.
     * If sending is true, the signal emitted is for messages being sent.
     */
    void peerWireStatistics(cMessage const*msg, bool sending);
    //! Print a debug message to the passed ostream, which defaults to clog.
    void printDebugMsg(std::string s) const;
    void printDebugMsgConnections(std::string methodName, int infoHash,
            Swarm const& swarm) const;
    //! TODO document this
    void removeThread(PeerWireThread *thread);
    //!@name Signal registration and subscription methods
    //@{
    //! Register all signals this module is going to emit.
    void registerEmittedSignals();
    //! Subscribe to signals.
    void subscribeToSignals();
    //@}

protected:
    //! Initialization performed in four stages.
    virtual int numInitStages() const;
    //! Record statistics
    virtual void finish();
    //! Open a port for incoming peer connections. Already defined in TCPSrvHostApp
    virtual void initialize(int stage);
    //! Must check if the Client can still open passive connections.
    virtual void handleMessage(cMessage* msg);
};

#endif
