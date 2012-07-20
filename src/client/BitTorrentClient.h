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
using boost::tuple;
using boost::make_tuple;
using boost::tie;

#include "PeerStatus.h"

class PeerStatus;
class PeerInfo;
class PeerWireThread;
class SwarmManager;

class Choker;
class ContentManager;

//! Vector of PeerStatus pointers.
typedef std::vector<PeerStatus const*> PeerVector;
//! Tuple with connect information about a Peer. <PeerId, IpAddress, Port>
typedef tuple<int, IPvXAddress, int> PeerConnInfo;
//! List of unconnected Peers
typedef std::list<PeerConnInfo> UnconnectedList;
//! Map of peer entries with peerId as key
typedef std::map<int, PeerStatus> PeerMap;
//! Swarm (numActive, numPassive, PeerMap, UnconnectedList, seeding)
struct Swarm {
    int numActive;
    int numPassive;
    PeerMap peerMap;
    UnconnectedList unconnectedList;
    bool seeding;
    bool closing;
    Choker * choker;
    ContentManager* contentManager;
};
//! map of swarms with infoHash as key
typedef std::map<int, Swarm> SwarmMap;

// Iterators
typedef PeerVector::iterator PeerVectorIt;
typedef UnconnectedList::iterator UnconnectedSetIt;
typedef PeerMap::iterator PeerMapIt;
typedef PeerMap::const_iterator PeerMapConstIt;
typedef SwarmMap::iterator SwarmMapIt;
typedef SwarmMap::const_iterator SwarmMapConstIt;

/**
 * The BitTorrent Client implementation, which acts like a TCP server
 * when receiving connections from other Peers.
 *
 * Besides from that, it is also a TCP client, since it actively connects
 * to other peers.
 */
class BitTorrentClient: public TCPSrvHostApp {
public:
    BitTorrentClient();
    //! Delete the sockets and threads
    virtual ~BitTorrentClient();

    //!@name Methods used by the SwarmManagerThread
    //@{
    //! Add the Peer to the ConnectedPeerManager as an unconnected Peer.
    void addUnconnectedPeers(int infoHash, UnconnectedList & peers);
    //@}

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
     * Return a vector of pointers to PeerStatus objects, ordered by their upload
     * rate from the Client.
     *
     * @param infoHash[in] The infoHash that identifies the swarm.
     */
    PeerVector getFastestToUpload(int infoHash) const;
    /*!
     * Return a vector of pointers to PeerStatus objects, ordered by their
     * download rate to the Client.
     *
     * @param infoHash[in] The infoHash that identifies the swarm.
     */
    PeerVector getFastestToDownload(int infoHash) const;
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
    void closeConnection(int infoHash, int peerId) const;
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
    void peerInteresting(int infoHash, int peerId) const;
    /*!
     * Show lack of interest in the Peer.
     *
     * @param infoHash[in] The infoHash that identifies the swarm.
     * @param peerId[in] The id of the Peer.
     */
    void peerNotInteresting(int infoHash, int peerId) const;
    /*!
     * Send HaveMsg to all connected Peers warning that a piece was downloaded.
     *
     * @param infoHash[in] The infoHash that identifies the swarm.
     * @param pieceIndex[in] The index of the Piece that was downloaded.
     */
    void sendHaveMessages(int infoHash, int pieceIndex) const;
    /*!
     * Tell the corresponding thread that there is a piece available for sending.
     *
     * @param infoHash[in] The infoHash that identifies the swarm.
     * @param peerId[in] The id of the Peer.
     */
    void sendPieceMessage(int infoHash, int peerId) const;
    /*!
     * Update the upload rate of the Peer.
     *
     * @param infoHash[in] The infoHash that identifies the swarm.
     * @param peerId[in] The id of the Peer.
     * @param totalDownloaded[in] The total of bytes downloaded to the peer until
     * this call.
     * @return The calculated download rate.
     */
    double updateDownloadRate(int infoHash, int peerId,
        unsigned long totalDownloaded);
    /*!
     * Update the upload rate of the Peer.
     *
     * @param infoHash[in] The infoHash that identifies the swarm.
     * @param peerId[in] The id of the Peer.
     * @param totalUploaded[in] The total of bytes uploaded to the peer until
     * this call.
     * @return The calculated upload rate.
     */
    double updateUploadRate(int infoHash, int peerId,
        unsigned long totalUploaded);
    //@}

    //!@name Methods used by the SwarmManager
    //@{
    //! Return the id of this peer.
    int getLocalPeerId() const;
    /*!
     * Create the swarm.
     * A swarm is a list of Peers that have the same content.
     *
     * @param infoHash[in] The infoHash that identifies the swarm.
     * @param newSwarmSeeding[in] True if this Client is seeding for this swarm.
     */
    void createSwarm(int infoHash, int numOfPieces, int numOfSubPieces,
        int subPieceSize, bool newSwarmSeeding);
    /*!
     * Delete the swarm from the Client.
     *
     * @param infoHash[in] The infoHash that identifies the swarm.
     */
    void deleteSwarm(int infoHash);
    //@}
private:
    /*!
     * Declare PeerWireThread a friend of  BitTorrentClient, since their
     * behavior are intimately connected.
     */
    friend class PeerWireThread;
    //!@name Methods used by the PeerWireThread
    //@{
    /*!
     * Tell the ConnectedPeerManager that a Peer established a TCP connection
     * with the client.
     * Return true if the connection can be established.
     */
    void addPeerToSwarm(int infoHash, int peerId, PeerWireThread* thread,
        bool active);
    /*!
     * Return true if the Peer can connect with the Client. The connection will
     * not be possible if there are no connection slots available or if the Peer
     * is already connected.
     */
    bool canConnect(int infoHash, int peerId, bool active) const;
    /*!
     * Return the choker and content manager to this swarm.
     *
     * @param infoHash The infoHash of the desired swarm.
     * @return A pair with the Choker and ContentManager pointers. Both are set
     * to NULL if there is no swarm with the passed infoHash.
     */
    std::pair<Choker*, ContentManager*> checkSwarm(int infoHash);
    /*!
     * Schedule the start of processing for the next Thread with messages
     * waiting.
     */
    void processNextThread();
    //! Remove this peer from the ConnectedPeerManager.
    void removePeerFromSwarm(int infoHash, int peerId, int connId, bool active);
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
    //! Pointer to the thread currently utilizing the processor.
    std::set<PeerWireThread*>::iterator threadInProcessingIt;
    //!
    cMessage endOfProcessingTimer;
    //! Processing time histogram, used to generate values for the simulation
    cDoubleHistogram doubleProcessingTimeHist;
    //@}

    //! Contains the list of swarms mapped by their infoHash
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
    //! Set to true to print debug messages.
    bool debugFlag;
    //! Set to true to print debug messages for the swarm modules.
    bool subModulesDebugFlag;
    //! TODO document this
    int globalNumberOfPeers;
    //! TODO document this
    int numActiveConn;
    //! TODO document this
    int numPassiveConn;
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
    // Private Methods
    //! TODO document this
    void attemptActiveConnections(Swarm & swarm, int infoHash);
    //! Open a TCP connection with this Peer.
    void connect(int infoHash, const PeerConnInfo & peer);
    //! Close the server socket so that other Peers cannot connect with the Client.
    //    void closeListeningSocket();
    //! Emit a signal corresponding to reception of a message with the passed message id.
    void emitReceivedSignal(int messageId);
    //! Emit a signal corresponding to sending of a message with the passed message id.
    void emitSentSignal(int messageId);
    //! Return a reference to the Swarm with infoHash.
    Swarm & getSwarm(int infoHash);
    //! Return a const reference to the Swarm with infoHash.
    const Swarm & getSwarm(int infoHash) const;
    //! Return a reference to the Peer with peerId inside the Swarm with infoHash.
    PeerStatus & getPeerStatus(int infoHash, int peerId);
    //! Return a const reference to the Peer with peerId inside the Swarm with infoHash.
    const PeerStatus & getPeerStatus(int infoHash, int peerId) const;
    //! Open the server socket so that other Peers can connect with the Client.
    //    void openListeningSocket();
    /*!
     * Will emit signals for statistical purposes.
     * If sending is true, the signal emitted is for messages being sent.
     */
    void peerWireStatistics(const cMessage *msg, bool sending);
    //! Print a debug message to the passed ostream, which defaults to clog.
    void printDebugMsg(std::string s) const;
    void printDebugMsgConnections(std::string methodName, int infoHash, const Swarm & swarm) const;
    //! TODO document this
    void removeThread(PeerWireThread *thread);
    //!@name Signal registration and subscription methods
    //@{
    //! Register all signals this module is going to emit.
    void registerEmittedSignals();
    //@}

protected:
    //! Initialization performed in four stages.
    virtual int numInitStages() const;
    //! Record statistics
    virtual void finish();
    //! Open a port for incoming peer connections. Already defined in TCPSrvHostApp
    virtual void initialize(int stage);
    /*!
     * This method is a copy of the TCPSrvHostApp::handleMessage method, but with
     * one major difference: It treats self messages from the BitTorrentClient module
     * itself.
     */
    virtual void handleMessage(cMessage* msg);
};

#endif
