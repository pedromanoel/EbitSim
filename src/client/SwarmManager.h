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
#include <TCPSrvHostApp.h>
#include <signal.h>

#include "AnnounceRequestMsg_m.h"
#include "AnnounceResponseMsg_m.h"

class BitTorrentClient;
class Choker;
//class ClientController;
//class ConnectedPeerManager;
class ContentManager;
//class DataRateCollector;
class SwarmManagerThread;
class TorrentMetadata;

class SwarmManager: public TCPSrvHostApp {
    friend class SwarmManagerThread;
public:
    SwarmManager();
    virtual ~ SwarmManager();

    //!@name Swarm management
    //@{
    /*!
     * Connect with the Tracker and send the first announce message. If the
     * Client started as a seeder, the first announce message is of type
     * COMPLETE, being of type NORMAL otherwise.
     */
    void enterSwarm(TorrentMetadata const& torrentInfo, bool seeder);
    //! Connect with the Tracker and send a COMPLETE announce message.
    void finishedDownload(int infoHash);
    //! Connect with the Tracker and send an STOP announce message.
    void leaveSwarm(int infoHash);
    //@}

    //!@name
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

    void stopChoker(int infoHash);
private:
    class SwarmEngine;

    //!@name Pointers to other modules.
    //@{
    //! Used by the SwarmManagerThread objects to send the response to this
    //! module
    BitTorrentClient* bitTorrentClient;
    //    DataRateCollector * dataRateCollector;
    //    ConnectedPeerManager* connectedPeerManager;
    //@}
//    std::map<int, SwarmEngine *> swarmEngines;
    std::map<int, SwarmEngine> swarmEngines;
    std::map<int, SwarmManagerThread*> swarmThreads;

    int localPeerId;

    //!@name Module's parameters
    //@{
    //! True to print debug messages.
    bool debugFlag;
    //! True to print debug messages of the submodules.
    bool subModulesDebugFlag;
    //! Self-message
    //    cMessage keepAlive;
    //! The number of Peers the Client requests to the Tracker
    int numWant;
    //! The port opened for connection from other Peers
    int port;
    //! Time between two announces.
    double refreshInterval;

    IPvXAddress trackerAddress;
    int trackerPort;
    //@}
    //!@name Signals to provide statistics
    //@{
    simsignal_t downloadRateSignal;
    simsignal_t uploadRateSignal;
    /*!
     * Signal emitted the instant the Client Controller gives the order to enter
     * the swarm
     */
    simsignal_t enterSwarmSignal;
    /*!
     * Signal emitted when the client receives the first list of Peers from the tracker.
     */
    simsignal_t enteredSwarmSignal;
    //@}

private:
    /*!
     * Create a new SwarmEngine object and insert it in the swarmEngine map.
     * If the swarm already exists, do nothing.
     *
     * @param [in] torrent Contain information about the content.
     * @param [in] seeder If true, create a ContentManager with all of the
     *      content. If the SwarmEngine already exists, verify if the Content
     *      Manager module is in the seed state.
     * @exception std::logic_error Can't change the state of a SwarmEngine from
     *      leecher to seeder. Thrown when seeder is true, but the existing
     *      SwarmEngine object is not a seeder.
     */
    void setSwarm(TorrentMetadata const& torrent, bool seeder);
    //! Open a connection with the Tracker and send the request for Peers.
    //    void makeAnnounce(SwarmManagerThread * thread);
    //! Print a debug message to the passed ostream, which defaults to clog.
    void printDebugMsg(std::string s);
    void setStatusString(const char * s);

    //!@name Signal registration and subscription methods
    //@{
    //! Register all signals this module is going to emit.
    void registerEmittedSignals();
    //@}

protected:
    void handleMessage(cMessage *msg);
    void initialize(int stage);
    int numInitStages() const;

    double getDownloadRate();
    double getUploadRate();
};

#endif
