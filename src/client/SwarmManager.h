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

#ifndef __TRACKERCLIENT_H__
#define __TRACKERCLIENT_H__

#include <omnetpp.h>
#include <signal.h>
#include <TCPSocketMap.h>

class AnnounceRequestMsg;
class AnnounceResponseMsg;
class BitTorrentClient;
class Choker;
class ContentManager;
class TorrentMetadata;

class SwarmManager: public cSimpleModule {
friend class TrackerSocketCallback;
public:
    SwarmManager();
    virtual ~ SwarmManager();
    //!@name Swarm management
    //@{
    void askMorePeers(int infoHash);
    //! Connect with the Tracker and send a COMPLETE announce message.
    void finishedDownload(int infoHash);
    //@}

    /*!
     * Return true if the SwarmManager has a swarm with the passed infoHash.
     *
     * @param [in] infoHash The infoHash of the desired swarm.
     * @param [out] contentManager Return a pointer to the ContentManager of the
     *      swarm with the passed infoHash or set to NULL if there is no swarm.
     * @param [out] choker Return a pointer to the ContentManager of the swarm
     *      with the passed infoHash or set to NULL if there is no swarm.
     */
    std::pair<Choker*, ContentManager*> checkSwarm(int infoHash);
    //! Stop the choker timers.
//    void stopChoker(int infoHash);
private:
    //!@name Pointers to other modules.
    //@{
    //! Used by the SwarmManagerThread objects to send the response to this
    //! module
    BitTorrentClient* bitTorrentClient;
    //@}
    //! Treat the data received from the Tracker
    void socketDataArrived(int infoHash, AnnounceResponseMsg *msg);

    //! Used to find the sockets for the arriving messages
    TCPSocketMap socketMap;
    //! Base message for creating announce requests
//    AnnounceRequestMsg * announceRequestBase;

    //! Class that manages the swarm modules
    class TrackerSocketCallback;
    //! Maps an infoHash to a SwarmModules object
    std::map<int, TrackerSocketCallback*> callbacksByInfoHash;

    /*! The id of this peer. In the real implementation, it is a 20-byte string.
     * At http://wiki.theory.org/BitTorrentSpecification#peer_id there is a brief explanation
     * on how BitTorrent clients generate the peer_id.
     */
    int localPeerId;

    //!@name Module's parameters
    //@{
    //! True to print debug messages.
    bool debugFlag;
    //! True to print debug messages of the submodules.
//    bool subModulesDebugFlag;
    //! The number of Peers the Client requests to the Tracker
    int numWant;
    //! The port opened for connection from other Peers
    int localPort;
    //! Normal time between two announces.
    double normalRefreshInterval;
    //! Smallest time between two announces.
    double minimumRefreshInterval;
    //@}

private:
    //!@name Related to user commands
    //@{
    //! Give the proper treatment to user command messages.
    void treatUserCommand(cMessage * msg);
    //! Give the proper treatment to self messages.
    void treatSelfMessages(cMessage * msg);
    //! Give the proper treatment to TCP messages.
    void treatTCPMessage(cMessage * msg);
    /*!
     * Send an announce to the Tracker stating that this Peer entered the swarm.
     * The response to this announce will be passed to the BitTorrentClient,
     * which will start connecting with the received peers.
     * @param torrentInfo Information got from the torrent metadata.
     * @param seeder True if this Peer is a seeder.
     * @param trackerAddress The address of the Tracker.
     * @param trackerPort The port which the Tracker is listening.
     */
    void enterSwarm(TorrentMetadata const& torrentInfo, bool seeder,
            IPvXAddress const& trackerAddress, int trackerPort);
    /*! Send an announce to the Tracker stating that this Peer is exiting the
     * swarm. Upon response, close the connections with all the peers and delete
     * the swarm related modules
     * @param infoHash The infoHash of the swarm.
     */
    void leaveSwarm(int infoHash);
    //@}

    //! Print a debug message to the passed ostream, which defaults to clog.
    void printDebugMsg(std::string s);
    void setStatusString(const char * s);

private:
    //!@name Signals to provide statistics.
    //@{
    simsignal_t downloadRateSignal;
    simsignal_t uploadRateSignal;
    //!Signal emitted when the swarm is created.
    simsignal_t enterSwarmSignal;
    simsignal_t leaveSwarmSignal;
    // t this peerId emitted a signal
    simsignal_t emittedPeerId_Signal;
    //@}

    //! Register all signals this module is going to emit.
    void registerEmittedSignals();
    //@}

protected:
    void handleMessage(cMessage *msg);
    void initialize();
};

#endif
