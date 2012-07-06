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

//
// Controls which pieces are being downloaded.
//
// Emit the signals:
//  - "ContentManager_BecameSeeder": when the ContentManager starts seeding the
//      content.
//  - "ContentManager_TotalDownloaded": when a block is downloaded, with the
//      total of bytes downloaded so far.
//  - "ContentManager_PieceDownloadTime": when a whole piece is downloaded, with
//      the time interval it took to download it.
//
simple ContentManager
{
    parameters:
        @signal[ContentManager_100_percentDownloadMarkTime];
//        @signal[ContentManager_DownloadPiece];
        @signal[ContentManager_TotalBytesDownloaded];
        @signal[ContentManager_TotalBytesUploaded];
        @statistic[PiecesDownloaded](
            source=ContentManager_DownloadPiece;
            title="Pieces Downloaded";
//            interpolationmode=none;
            record=vector
        );
        @statistic[TotalBytesDownloaded](
            source=ContentManager_TotalBytesDownloaded;
            title="Total of Bytes Downloaded";
            unit=B;
//            interpolationmode=linear;
            record=vector
        );
        @statistic[TotalBytesUploaded](
            source=ContentManager_TotalBytesUploaded;
            title="Total of Bytes Uploaded";
            unit=B;
//            interpolationmode=linear;
            record=vector
        );
        // X axis
//        @statistic[ContentManager_DownloadMarkPeerId](
//            title="Download Time Abscissa";
//            unit=s;
//            record=vector
//        );
//        // 25%
//        @statistic[ContentManager_25_percentDownloadMarkTime](
//            title="25% Download Time Ordinate";
//            unit=s;
//            record=last,vector
//        );
//        // 50%
//        @statistic[ContentManager_50_percentDownloadMarkTime](
//            title="50% Download Time Ordinate";
//            unit=s;
//            record=last,vector
//        );
//        // 75%
//        @statistic[ContentManager_75_percentDownloadMarkTime](
//            title="75% Download Time Ordinate";
//            unit=s;
//            record=last,vector
//        );
        // 100%
//        @statistic[DownloadComplete](
//            source=ContentManager_100_percentDownloadMarkTime;
//            title="Download Complete Time";
//            unit=s;
//            record=vector
//        );

        // The number of pieces in the file
        int numOfPieces;
        // The number of sub-pieces in each piece
        int numOfSubPieces;
        // The size of the sub-piece
        int subPieceSize @unit(B);
        // The maximum number of bytes transmitted in one second
        int bytesSec @unit(B) = default(512KiB);
        // The number of pieces that can be sent in one burst
        int burstSize = default(50);
        // Set to true to print debug messages
        bool debugFlag = default(false);
        // The number of have messages  that can be sent in one go.
        int haveBundleSize = default(10);
        // The number of request messages that can be sent in one go.
        int requestBundleSize = default(10);
        // True if the Client have all pieces already.
        bool seeder;
        // The infoHash of the swarm the ContentManager is serving
        int infoHash;
        @display("i=block/table2");
}
