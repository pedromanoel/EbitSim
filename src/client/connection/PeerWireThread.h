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


#ifndef PEERWIRETHREAD_H_
#define PEERWIRETHREAD_H_

#include <cpacketqueue.h>
#include <NotificationBoard.h>
#include <TCPSrvHostApp.h>

#include "Application_m.h"
#include "client/smc/out/ConnectionSM_sm.h"
#include "client/smc/out/DownloadSM_sm.h"
#include "client/smc/out/UploadSM_sm.h"
#include "PeerWire_m.h"

class BitTorrentClient;
class Choker;
class ContentManager;
class PeerWireMsg;
class PeerWireMsgBundle;

class PeerWireThread: public TCPServerThreadBase {
public:
    // own methods
    friend class BitTorrentClient;
    PeerWireThread();
    PeerWireThread(int infoHash, int remotePeerId);
    virtual ~PeerWireThread();
public:
    // parents' methods
    //!@name Implementation of virtual methods from TCPServerThreadBase
    /*!
     * TODO Write something about the TCPServerThreadBase and this implementation.
     */
    //@{
    /*!
     * Called when the connection closes (successful TCP teardown).
     * TODO update doc
     */
    void closed();
    /*!
     * The data packets are identified and the corresponding transition is called in
     * the state machines.
     * TODO update doc
     */
    void dataArrived(cMessage *msg, bool urgent);
    /*! Called when connection is established.
     * TODO update doc
     */
    void established();
    /*!
     * Called when the connection breaks (TCP error).
     * TODO update doc
     */
    void failure(int code);
    /*!
     * Initialize the thread object. Must be called at creation time, or else
     * the thread will not work. The other methods don't check if this method
     * was called.
     */
    void init(TCPSrvHostApp *hostmodule, TCPSocket *socket);
    /*!
     * Called when the connection is closed from the client side. If the
     * connection is established, close it from the server side.
     */
    void peerClosed();
    //! TODO use the status info to abort connections that are not finished yet.
    void statusArrived(TCPStatusInfo* status);
    //! The timers in PeerWireThread send application transitions to the state machines.
    void timerExpired(cMessage *timer);
    //@}
public:
    //!@name ConnectionSM's external transition callers
    //@{
    //! External call the ConnectionSM's outgoingPeerWireMsg transition.
    void outgoingPeerWireMsg_ConnectionSM(PeerWireMsg * msg);
    //! External call the ConnectionSM's outgoingPeerWireMsg transition.
    void outgoingPeerWireMsg_ConnectionSM(PeerWireMsgBundle * msg);
    //@}

    //!@name Connection State Machine methods.
    //@{
    void addConnectedPeer();
    //! Send a BitField message to the Peer.
    BitFieldMsg * getBitFieldMsg();
    //! Send a Handshake to the Peer.
    Handshake * getHandshake();
    //! Send a Keep-Alive message to the Peer.
    KeepAliveMsg * getKeepAliveMsg();
    //! Restart the keep-alive timer.
    void renewKeepAliveTimer();
    //! Restart the timeout timer.
    void renewTimeoutTimer();
    //! Send a PeerWireMsg to the connected Peer.
    void sendPeerWireMsg(cPacket * msg);
    //! Start the timeout and the keep-alive timers.
    void startHandshakeTimers();
    //! Stop the timeout and keep-alive timers.
    void stopHandshakeTimers();
    //! Terminate the thread, removing all information about the peer from the swarm
    void terminateThread();
    //@}

    //!@name Connection State Machine transition guards.
    //@{
    //! Verify if the Peer sent a Handshake with the expected Id and info hash.
    bool checkHandshake(Handshake const& hs);
    //! Return true if this Client's BitField is empty
    bool isBitFieldEmpty();
    //@}

    //!@name Download State Machine methods.
    //@{
    //! Add the BitField from the message to the Peer. Throw an error if the
    //! BitField is not empty.
    void addBitField(const BitFieldMsg & msg);
    //! Calculate the download rate.
    void calculateDownloadRate();
    //! Cancel the requests made.
    void cancelPendingRequests();
    //! Return a new InterestedMsg.
    InterestedMsg * getInterestedMsg();
    //! Return a new NotInterestedMsg.
    NotInterestedMsg * getNotInterestedMsg();
    //! Return a new RequestMsgBundle or NULL, if there are no requests to be made.
    PeerWireMsgBundle * getRequestMsgBundle();
    //! Save the downloaded block and verify the interest in all peers if the piece is completed.
    void processBlock(PieceMsg const& msg);
    //! Update the Peer BitField and verify if it became interesting.
    void processHaveMsg(HaveMsg const& msg);
    //! Restart the download rate timer.
    void renewDownloadRateTimer();
    //! Restart the snubbed timer.
    void renewSnubbedTimer();
    //! Set to true if the Client is snubbed by the Peer.
    void setSnubbed(bool snubbed);
    //! Start the snubbed and the download rate timers.
    void startDownloadTimers();
    //! Stop the snubbed and the download rate timers.
    void stopDownloadTimers();
    //@}

    //!@name Upload State Machine methods
    //@{
    //! Cancel the request made by the Peer.
    void cancelPiece(CancelMsg const& msg);
    //! Calculate the upload rate.
    void calculateUploadRate();
    //! Tell the Choker execute the next choke round now.
    void callChokeAlgorithm();
    //! While there are available slots, choked peers that become interested
    //! will be unchoked.
    void fillUploadSlots();
    //! Return a new ChokeMsg.
    ChokeMsg * getChokeMsg();
    //! Return a new PieceMsg for the RequestMsg received.
    PieceMsg * getPieceMsg(RequestMsg const& msg);
    //! Return a new UnchokeMsg.
    UnchokeMsg * getUnchokeMsg();
    //! Restart the upload rate timer.
    void renewUploadRateTimer();
    //! Set to true if the Peer is interested in the Client.
    void setInterested(bool interested);
    //! Start the old unchoke and the upload rate timers.
    void startUploadTimers();
    //! Stop the old unchoke and the upload rate timers.
    void stopUploadTimers();
    //@}
    //! Print a debug message with information about the thread.
    void printDebugMsg(std::string s);
    void printDebugMsgUpload(std::string s);
    void printDebugMsgDownload(std::string s);
    void printDebugMsgConnection(std::string s);
private:
    // attributes
    /*!
     * @name BitTorrent State Machines
     */
    //@{
    ConnectionSMContext connectionSm;
    DownloadSMContext downloadSm;
    UploadSMContext uploadSm;
    //@}
    //! True if this thread was created because of an active connection.
    bool activeConnection;
    //! Pointer to the host, cast to the BitTorrentClient class.
    BitTorrentClient *btClient;
    Choker *choker;
    ContentManager *contentManager;
    /*!
     * The peerId of the Peer, which can be set in two moments: at the
     * connection, if the Tracker specified it, or at the arrival of the Handshake.
     */
    int infoHash;
    int remotePeerId;
    /*!
     * True when this thread is terminated.
     * Will result in the deletion of the thread when the finishProcessing()
     * method is called.
     */
    bool terminated;

    //!@name Message processing
    //@{
    bool processing;
    /*!
     * Self-message used to signal the end of processing of the message at the
     * top of the message queue.
     */
//    cMessage endOfProcessingTimer;
    /*!
     * Queue of messages that are waiting to be processed.
     */
    cQueue peerWireMessageBuffer;
    /*!
     * Queue of application messages generated during a state machine transition.
     * These messages will be executed right after a PeerWireMsg is processed,
     * and before the next PeerWireMsg starts its processing. Other threads can
     * insert application messages in this thread by calling sendApplicationMsg.
     */
    cQueue applicationMsgQueue;
    /*!
     * Pointer to the message being processed at the moment or NULL if no
     * processing is being made.
     */
//    cMessage * messageInProcessing;
    //@}

    //!@name Self-messages
    //@{
    //! Self-message used to calculate the download rate.
//    cMessage *downloadRateTimer;
    cMessage downloadRateTimer;
    //! Self-message used to create keep-alives.
//    cMessage *keepAliveTimer;
    cMessage keepAliveTimer;
    /*!
     * Self-message used to signal the deletion of this thread.
     */
//    cMessage selfThreadDeleteTimer;
    //! Self-message used to verify if the peer was recently unchoked.
    //    cMessage* oldUnchokeTimer;
    //! Self-message used to verify if the connection is snubbed.
//    cMessage *snubbedTimer;
    cMessage snubbedTimer;
    //! Self-message used to timeout the connection.
//    cMessage *timeoutTimer;
    cMessage timeoutTimer;
    //! Self-message used to calculate the upload rate.
//    cMessage *uploadRateTimer;
    cMessage uploadRateTimer;
    //@}
private:
    //!@name Thread processing
    //@{
    /*!
     * Will finish the processing of this thread.
     */
    void finishProcessing();
    /*!
     * Return true if this Thread has ApplicationMsg or PeerWireMsg waiting to
     * be processed.
     */
    bool hasMessagesToProcess();
    /*!
     * Return true if this Thread is currently processing a PeerWireMsg.
     */
    bool isProcessing();
    /*!
     * Will execute all ApplicationMsg currently queued, then will execute the
     * PeerWireMsg at the top of the queue, scheduling the end of processing.
     * At the end of processing (in the timerExpired method), tell the
     * BitTorrentClient to process the next thread.
     */
    simtime_t processThread();
    //@}

    /*!
     * Will queue the ApplicationMsg if the processor is busy, or will execute
     * it immediately if not.
     */
    void computeApplicationMsg(ApplicationMsg* msg);
    //! Issue a transition to one of the state machines.
    void issueTransition(cMessage const* msg);
    //! The Client was not recently unchoked.
//    void oldUnchoked();
    //! The Client is snubbed by the Peer.
//    void peerSnubbed();
    //! Send a message from the Application to the state machines.
    void sendApplicationMessage(int kind);
    //! Set the size of the packet, then send it.
//    void sendPeerWireMsg(cPacket* msg);
//    void sendPeerWireMsg(PeerWireMsg* msg);
    //! Set the size of the packet, then send it.
//    void sendPeerWireMsg(PeerWireMsgBundle* msg);
};

#endif /* PEERWIRETHREAD_H_ */
