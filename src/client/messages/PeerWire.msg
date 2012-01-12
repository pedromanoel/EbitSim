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
}}

enum PeerWireMessageType {
    PW_CHOKE_MSG = 0;
    PW_UNCHOKE_MSG = 1;
    PW_INTERESTED_MSG = 2;
    PW_NOT_INTERESTED_MSG = 3;
    PW_HAVE_MSG = 4;
    PW_BITFIELD_MSG = 5;
    PW_REQUEST_MSG = 6;
    PW_PIECE_MSG = 7;
    PW_CANCEL_MSG = 8;
    // id given to simplify the code (the real keep-alive don't have an id field)
    PW_KEEP_ALIVE_MSG = 9;
    PW_HANDSHAKE_MSG = 10;
}

cplusplus {{
#include "BitField.h"
}}

class noncobject BitField;

//
// The basic PeerWire message. All messages subclasses from this.
// A PeerWire message is composed of three fields: <length><id><payload>
// The payload greatly changes depending on the message.
//
packet PeerWireMsg {
    int headerLen = 4;
    // 4-byte big-endian value.
    int payloadLen = 1;
    // must be set to the size of the real payload for BitFieldMsg and PieceMsg
	int variablePayloadLen = 0;
    //single byte value.
    int messageId enum(PeerWireMessageType);
}

//
// When a Peer receives this message from another Peer, it knows that it is being
// choked by it and all piece requests will be ignored.
// This message has no payload.
//
packet ChokeMsg extends PeerWireMsg {
    int messageId = PW_CHOKE_MSG;
}

//
// When a Peer receives this message from another Peer, it knows that it is possible
// to acquire data from it.
// This message has no payload.
//
packet UnchokeMsg extends PeerWireMsg {
    int messageId = PW_UNCHOKE_MSG;
}

//
// A Peer sends this message to another Peer when the content owned by that Peer is interesting to it.
// This message has no payload.
//
packet InterestedMsg extends PeerWireMsg {
    int messageId = PW_INTERESTED_MSG;
}

//
// If a Peer obtains all the content another Peer has, then this Peer becomes uninteresting and this message
// is sent.
// This message has no payload.
//
packet NotInterestedMsg extends PeerWireMsg {
    int messageId = PW_NOT_INTERESTED_MSG;
}

// When a Peer finishes downloading a piece, this message is sent to all connected Peers.
packet HaveMsg extends PeerWireMsg {
    int payloadLen = 5;
    int messageId = PW_HAVE_MSG;
    int index; // 4 bytes
}

//
// A Peer sends this message when it wants to acquire a sub-piece.
// By analyzing only this message's parameters, it is plausible to
// assume that a Peer can request blocks of different sizes, but
// this behavior is undefined in modern BitTorrent clients. Some of
// them may actually drop the connection if the reqLength is different
// from the expected block size. This will be the behavior in this version
// of the simulation, but a possible extension is to allow different block
// sizes.
//
packet RequestMsg extends PeerWireMsg {
    int payloadLen = 13;
    int messageId = PW_REQUEST_MSG;
    int index;
    int begin;
    int reqLength;
}

//
// Right after the handshake message, a Peer sends which pieces it has through a bitField.
// The length of the message depends on the size of the bitField (that is, the number of pieces).
// The bitField is defined in bytes, therefore all extra bits are set to zero.
//
packet BitFieldMsg extends PeerWireMsg {
    int messageId = PW_BITFIELD_MSG;
    int payloadLen = 1 + 0; // must sum the number of pieces to the payload length
    BitField bitField;
    int numberOfPieces;
}

//
// The actual data exchanged between peers.
// The length is dependant on the size of the piece. The user must add to the msgLen
// the size of the block.
//
packet PieceMsg extends PeerWireMsg {
    int payloadLen = 9 + 0; // must sum the block size to the payload length
    int messageId = PW_PIECE_MSG;
    int index;
    int begin;
    int blockSize;
}

//
// A Peer sends this message when it whishes to cancel a sub-piece request.
//
packet CancelMsg extends PeerWireMsg {
    int payloadLen = 13;
    int messageId = PW_CANCEL_MSG;
    int index;
    int begin;
    int reqLength;
}

//
// If a Peer does not receive a keep-alive from another Peer before the timeout,
// then the connection between them must be closed.
// The keep-alive message is the only one that has length zero.
//
packet KeepAliveMsg extends PeerWireMsg {
    int payloadLen = 0;
    int messageId = PW_KEEP_ALIVE_MSG;
}

//
// The handshake msg is not part of the peer-wire protocol, but it was put here
// as a mean to simplify the message handling.
// The handshake exchanged between Peers when the connection is established.
// It is 49+len(pstr) bytes long. In this case, 68 bytes long.
//
packet Handshake extends PeerWireMsg {
    int messageId = PW_HANDSHAKE_MSG;
    int length = 68; // (49+19).
    int pstrlen = 19;   // the length of the pstr.
    string pstr = "BitTorrent protocol";    // string identifying the protocol.
    int reserved = 0;       // 8 reserved bytes for extensions.
    int infoHash;    // 20-byte SHA1 hash.
    // The specification says the peerId is a 20-byte string, but to simplify
    // the code development, the id of the module is used instead, since it is
    // guaranteed to be unique.
    int peerId;
}