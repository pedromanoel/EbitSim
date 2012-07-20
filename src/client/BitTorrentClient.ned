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

package br.larc.usp.client;

import inet.applications.tcpapp.TCPSrvHostApp;
//
// Manage the interaction between the client and other peers.
//
simple BitTorrentClient extends TCPSrvHostApp
{
    parameters:
//        @statistic[BitTorrentClient_PeerWireBytesSent](
//            title="BitTorrent Bytes Sent";
//            record=sum
//        );
//        @statistic[BitTorrentClient_PeerWireBytesReceived](
//            title="BitTorrent Bytes Received";
//            record=sum
//        );
//        @statistic[BitTorrentClient_ContentBytesSent](
//            title="Content Bytes Sent";
//            record=sum
//        );
//        @statistic[BitTorrentClient_ContentBytesReceived](
//            title="Content Bytes Received";
//            record=sum
//        );
//        @statistic[BitTorrentClient_PieceReceived](
//            title="Number of Handshakes Received";
//            record=count
//        );
//        @statistic[BitTorrentClient_PieceSent](
//            title="Number of Handshakes Sent";
//            record=count
//        );
//        @statistic[BitTorrentClient_HandshakeReceived](
//            title="Number of Handshakes Received";
//            record=count
//        );
//        @statistic[BitTorrentClient_HandshakeSent](
//            title="Number of Handshakes Sent";
//            record=count
//        );
//        @statistic[BitTorrentClient_NumUnconnected](
//            title="Number of Unconnected Peers";
//            record=vector
//        );
//        @statistic[BitTorrentClient_NumConnected](
//            title="Number of Connected Peers";
//            record=vector,max
//        );
        @signal[BitTorrentClient_ProcessingTime];
        @statistic[ProcessingTime](
            source=BitTorrentClient_ProcessingTime;
            title="Time in processing";
            record=sum
        );
        
        @display("i=misc/globe,green");
        @class(BitTorrentClient); // tell omnet++ to use this class instead of TCPSrvHostApp
        // class name of "thread" objects to launch on incoming connections
        serverThreadClass = "n/a";
        port = default(6881); // port the client is listening
        bool debugFlag = default(false);
        bool subModulesDebugFlag = default(false);
        // file-saved histogram with the processing time
        string processingTimeHistogram;
        double downloadRateInterval @unit(s) = default(1s);
        double keepAliveInterval @unit(s) = default(60s);
        double oldUnchokeInterval @unit(s) = default(20s);
        double snubbedInterval @unit(s) = default(30s);
        double timeoutInterval @unit(s) = default(120s);
        double uploadRateInterval @unit(s) = default(1s);
        int globalNumberOfPeers = default(30);
        int numberOfActivePeers = default(8);
        int numberOfPassivePeers = default(2);
}
