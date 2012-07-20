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
#include <TCPSrvHostApp.h>

#include "client/smc/out/ConnectionSM_sm.h"
#include "client/smc/out/DownloadSM_sm.h"
#include "client/smc/out/UploadSM_sm.h"
#include "PeerWireMsgBundle_m.h"
#include "PeerWire_m.h"

class BitTorrentClient;
class Choker;
class ContentManager;

class PeerWireThread: public TCPServerThreadBase {
public:
    // own methods
    friend class BitTorrentClient;
    PeerWireThread(int infoHash, int remotePeerId);
    virtual ~PeerWireThread();
public:
    // parents' methods
    //!@name Implementation of the virtual methods from TCPServerThreadBase
    //@{
    //! Send an application message warning about the TCP connection closing.
    void closed();
    //! Insert the message in the queue for later processing.
    void dataArrived(cMessage *msg, bool urgent);
    //! Initiate the Handshake procedure in the Connection State Machine.
    void established();
    //! Shouldn't happen, so throw an error.
    void failure(int code);
    //! Call parent ctor and cast hostmodule to BitTorrentApp* as a shortcut.
    void init(TCPSrvHostApp *hostmodule, TCPSocket *socket);
    //! Send an application message about the Peer closing the TCP connection.
    void peerClosed();
    /*!
     * Send an application message corresponding to the expired timer.
     *
     * Timer messages are not deleted because they are constantly being
     * re-scheduled.
     * @param timer
     */
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
    void connected();
    //! Close the TCP connection from the client's side.
    void closeLocalConnection();
    //! Start the Download and Upload machines
    void startMachines();
    //! Stop the Download and Upload machines
    void stopMachines();
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
    void removeFromSwarm();
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
    void cancelDownloadRequests();
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
    //! Cancel all the requestes made by this Peer.
    void cancelUploadRequests();
    //! Calculate the upload rate.
    void calculateUploadRate();
    //! Tell the Choker execute the next choke round now.
    void callChokeAlgorithm();
    //! While there are available slots, choked peers that become interested
    //! will be unchoked.
    void addPeerToChoker();
    //! Return a new ChokeMsg.
    ChokeMsg * getChokeMsg();
    //! Make a piece request to the ContentManager.
    void requestPieceMsg(RequestMsg const& msg);
    //! Get the previously requested piece from the ContentManager.
    PieceMsg * getPieceMsg();
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
    std::string getThreadId();
    void printDebugMsg(std::string s);
    void printDebugMsgUpload(std::string s);
    void printDebugMsgDownload(std::string s);
    void printDebugMsgConnection(std::string s);
private:
    //!@name BitTorrent State Machines
    //@{
    ConnectionSMContext connectionSm;
    DownloadSMContext downloadSm;
    UploadSMContext uploadSm;
    //@}

    //! Pointer to the host, cast to the BitTorrentClient class.
    BitTorrentClient *btClient;

    //!@name Swarm-related modules
    //@{
    Choker *choker; //! The Swarm's Choker module
    ContentManager *contentManager; //! The Swarm's Content Manager module
    //@}


    // attributes
    //! True if this thread was created because of an active connection.
    bool activeConnection;
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
    bool terminating;

    //!@name Message processing
    //@{
    bool busy;
    /*!
     * Queue with all the messages to be processed. ApplicationMsgs are
     * processed instantly, while the PeerWireMsg are processed with a timeout
     * timer. The processing timer also serves as the time slice for this thread.
     */
    cQueue messageQueue;
    /*!
     * Queue for messages produced during the processing of the current message.
     */
    cQueue postProcessingAppMsg;
    //@}

    //!@name Self-messages
    //@{
    //! Self-message used to calculate the download rate.
    cMessage downloadRateTimer;
    //! Self-message used to create keep-alives.
    cMessage keepAliveTimer;
    //! Self-message used to verify if the connection is snubbed.
    cMessage snubbedTimer;
    //! Self-message used to timeout the connection.
    cMessage timeoutTimer;
    //! Self-message used to calculate the upload rate.
    cMessage uploadRateTimer;
    //@}
private:
    //!@name Message processing
    //@{
    //! Cancel all the messages in the queue.
    void cancelMessages();
    //! True if there are messages to process in this thread.
    bool hasMessagesToProcess();
    /*!
     * Start processing the messages in the message queue. Will process all
     * application messages until it finds a PeerWire message.
     *
     * Return the time it will take to process these messages. If no PeerWire
     * was found in the queue, this time is zero.
     */
    simtime_t startProcessing();
    //! Execute the transitions issued during the processing, or delete the thread
    void finishProcessing();
    /*!
     * Process all ApplicationMsgs in the queue until a PeerWireMsg is found or
     * until the queue is empty.
     */
    void processAppMessages();
    //! Insert a message from the Application in the processing queue.
    void sendApplicationMessage(int kind);
    //! Insert a message from the connection in the processing queue.
    void sendPeerWireMessage(cMessage * msg);
    //@}

    //! Issue a transition to one of the state machines.
    void issueTransition(cMessage const* msg);
    //! The Client was not recently unchoked.
    //    void oldUnchoked();
};

#endif /* PEERWIRETHREAD_H_ */
