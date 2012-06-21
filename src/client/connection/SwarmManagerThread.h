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

#ifndef SWARMMANAGERTHREAD_H_
#define SWARMMANAGERTHREAD_H_

#include <TCPSrvHostApp.h>

#include "AnnounceRequestMsg_m.h"
#include "AnnounceResponseMsg_m.h"
#include "ClientController.h"

class SwarmManagerThread: public TCPServerThreadBase {
public:
    SwarmManagerThread();
    SwarmManagerThread(AnnounceRequestMsg const& announceRequest,
            double refreshInterval, bool seeder,
            IPvXAddress const& trackerAddress, int trackerPort);
    virtual ~SwarmManagerThread();

    /*!
     * Cancel the next periodic announce and set the announce type of the next
     * announce. This method must be called before manually sending an announce
     * to the Tracker.
     */
    void sendAnnounce(ANNOUNCE_TYPE announceType);

    //!@name Implementation of the virtual methods from TCPSrvHostApp.
    //@{
    //! Called when the connection is established. Do nothing.
    void established();
    //! Called when a data packet arrives. Process the response from the Tracker.
    void dataArrived(cMessage *msg, bool urgent);
    /**
     * Called when a timer (scheduled via scheduleAt()) expires. The only timer
     * schedule by this thread is the keep-alive timer, used to resend the
     * announce message.
     */
    void timerExpired(cMessage *timer);
    /**
     * Called when the connection closes (successful TCP teardown). Asks the
     * hostmod to delete this thread.
     */
    void closed();
    /**
     * Called when the connection breaks (TCP error). Send the announce message
     * again.
     */
    void failure(int code);
    //@}
public:
    enum SelfMessageType {
        SELF_THREAD_DELETION = 1000, SELF_ANNOUNCE_TIMER,
    };
private:
    //! Return the string representation of the current event.
    char const* getEventStr(ANNOUNCE_TYPE type);
private:
    //! Copy of the request made to the tracker, to allow re-sending it.
    AnnounceRequestMsg announceRequest;
    cMessage requestTimer;
    double refreshInterval;
    bool seeder;
    IPvXAddress trackerAddress;
    int trackerPort;
};

#endif /* SWARMMANAGERTHREAD_H_ */
