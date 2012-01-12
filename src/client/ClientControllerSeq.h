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

#ifndef __CLIENTBEHAVIOR_H__
#define __CLIENTBEHAVIOR_H__

#include <omnetpp.h>
#include <clistener.h>
#include <list>

#include "TrackerApp.h"

class SwarmManager;

/**
 * This module is responsible for controlling the Client's behavior: what content
 * to download, when to start downloading, how long should the seeding time be.
 */
class ClientControllerSeq: public cListener, public cSimpleModule {
public:
    //!@name cListener method
    //@{
    void receiveSignal(cComponent *source, simsignal_t signalID, long infoHash);
    //@}
public:
    ClientControllerSeq();
    virtual ~ClientControllerSeq();
    int getPeerId() const;
private:
    //! Self message that signals the Client to enter a swarm.
    cMessage enterSwarmMsg;
    //! Self message that signals the Client to enter all swarms it is seeding.
    cMessage enterSwarmSeederMsg;
    SwarmManager* swarmManager;

    //! A list of metadata for contents the Client will download. These
    //! contents are downloaded one at a time, from the front to the back.
    std::list<TorrentMetadata> contentDownloadQueue;

    /*! The id of this peer. In the real implementation, it is a 20-byte string.
     * At http://wiki.theory.org/BitTorrentSpecification#peer_id there is a brief explanation
     * on how BitTorrent clients generate the peer_id.
     */
    int localPeerId;
    //! Set to true to print debug messages
    bool debugFlag;
    //! Signal emitted to warn that a Peer will download a content during the simulation.
    simsignal_t leecherSignal;
    //! This controller subscribes to this signal in order to know when a peer
    //! becomes a seeder.
    simsignal_t seederSignal;
private:
    //! Print a debug message to clog.
    void printDebugMsg(std::string s);
    void updateStatusString();
    //!@name Signal registration and subscription methods
    //@{
    //! Register all signals this module is going to emit.
    void registerEmittedSignals();
    //! Subscribe to signals.
    void subscribeToSignals();
    //@}
protected:
    int numInitStages() const;
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
};

#endif