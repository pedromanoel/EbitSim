cplusplus {{
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

#include "BitField.h"
}}

class noncobject BitField;

// announce event types
enum ANNOUNCE_TYPE
{
    A_STARTED   = 1; // announce event=started
    A_STOPPED   = 2; // announce event=stopped
    A_COMPLETED = 3; // announce event=completed
    A_NORMAL    = 4; // normal announce (after started and before completed)
//    A_FOREIGN   = 5;
};

packet AnnounceRequestMsg {
    // The following header was extracted from the prototype, and is being used
    // as base for the request size, with length of 163 bytes. Attention to the
    // newline character at the end.
    //
    // "GET /announce?<Announce URL> HTTP/1.1
    // User-Agent: Java/1.6.0_14
    // Host: XXX.XXX.XXX.XXX:XXXX
    // Accept: text/html, image/gif, image/jpeg, *; q=.2, */*; q=.2
    // Connection: keep-alive
    //
    // "
    // It is being used as the base size for the message.
    int baseByteLength = 163;

    // string representation of the announce, determine the size of the message
    string urlEncodedRequest;

    // The info hash of the content (swarm).
    int infoHash;

    // announce parameters
    //bool compact;               // True if the requester wants a compact response.
    double downloaded;    // The number of bytes downloaded by the requesting peer.
    int event enum(ANNOUNCE_TYPE); // The type of the event that generated this request.
    //IPvXAddress ip;             // The real, routable IPvXAddress of the requesting peer.
    //string key;                 // the key of the requesting peer.
    //bool noPeerId;              // True if the requester does not want the peers' ids.
    unsigned int numWant;       // The maximum number of peers wanted.
    int peerId;                 // The Id of the requesting peer. See PeerEntry for more info on the Peer id.
    unsigned int port;          // The port that the peer is listening for connections with other peers.
    double uploaded;      // The number of bytes uploaded by the requesting peer.
    //string trackerId;           // The Tracker Id.
//    BitField bitfield;      // The client's BitField.
}
