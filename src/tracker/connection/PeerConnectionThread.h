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
 * TrackerThread.h
 *
 *  Created on: Mar 29, 2010
 *      Author: pevangelista
 */

#ifndef TRACKERTHREAD_H_
#define TRACKERTHREAD_H_

#include <TCPSrvHostApp.h>

#define HTTP_RESPONSE_HEADER_SIZE 137

class AnnounceRequestMsg;
class AnnounceResponseMsg;
class TrackerApp;

class PeerConnectionThread: public TCPServerThreadBase {
public:
    friend class TrackerApp;
    PeerConnectionThread();
    virtual ~PeerConnectionThread();
public:
    //!@name Implementation of virtual methods from TCPServerThreadBase
    /*!
     * TODO Write something about the TCPServerThreadBase and this implementation.
     */
    //@{
    //! Called when the connection closes (successful TCP teardown).
    void closed();
    //! Called when a data packet arrives.
    void dataArrived(cMessage *msg, bool urgent);
    //! Called when connection is established.
    void established();
    //! Called when the connection breaks (TCP error).
    void failure(int code);
    //! Called when the client closes the connection.
    void peerClosed();
    //! Called when a status arrives in response to getSocket()->getStatus().
    void statusArrived(TCPStatusInfo *status);
    //! Called when a timer (scheduled via scheduleAt()) expires.
    void timerExpired(cMessage *timer);
    //@}
    //! Internal: called by the method handleMessage from TCPSrvHostApp when creating a process.
    void init(TCPSrvHostApp* hostmodule, TCPSocket* socket);
private:
    //!@name Own methods
    //@{
    AnnounceResponseMsg* makeResponse(AnnounceRequestMsg* msg);
    void processDataArrived(cMessage *msg);
    void printDebugMsgConnection(std::string s);
    //@}
private:
    enum SelfMessageType {
        SELF_CLOSE_THREAD = 100, SELF_PROCESS_TIME = 101
    };
    TrackerApp* trackerApp;
    /*!
     * Size of the http header string.
     *
     * The following header is being used as reference (attention to the newline
     * character at the end), with length of 137 bytes:
     * "HTTP/1.1 200 OK
     * Content-Type: text/plain
     * Date: Tue, 30 Mar 2010 17:56:39 GMT
     * Server: Default IPTVP2P Tracker
     * Transfer-Encoding: chunked
     *
     * "
     */
    const int httpResponseHeaderSize;
};

#endif /* TRACKERTHREAD_H_ */
