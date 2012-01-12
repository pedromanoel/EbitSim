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

#include "ContentManager.h"
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <deque>

#include "BitTorrentClient.h"
#include "SwarmManager.h"
//#include "Statistics.h"
#include "PeerWire_m.h"
#include "PeerWireMsgBundle_m.h"

Piece::Piece(int pieceIndex, int numOfBlocks) :
    downloadedBlocks(0), numOfBlocks(numOfBlocks), pieceIndex(pieceIndex) {
    for (int i = 0; i < numOfBlocks; ++i) {
        this->blocks.insert(std::make_pair(pieceIndex, i));
    }
}
bool Piece::setBlock(int blockIndex) {
    if (blockIndex < 0 || this->numOfBlocks <= blockIndex) {
        throw std::out_of_range("Invalid blockIndex");
    }

    this->blocks.erase(std::make_pair(this->pieceIndex, blockIndex));

    return this->blocks.empty();
}
std::list<std::pair<int, int> > Piece::getMissingBlocks() const {
    std::list<std::pair<int, int> > missingBlocks;
    std::set<std::pair<int, int> >::iterator it = this->blocks.begin();

    for (; it != this->blocks.end(); ++it) {
        missingBlocks.push_back(*it);
    }

    return missingBlocks;
}
int Piece::getPieceIndex() const {
    return this->pieceIndex;
}

Define_Module( ContentManager);

ContentManager::ContentManager() :
    bitTorrentClient(NULL), /*clientController(NULL), swarmManager(NULL),
     statistics(NULL), */subPieceSize(0), debugFlag(false), haveBundleSize(0),
            numberOfSubPieces(0), numberOfPieces(0), requestBundleSize(0),
            totalBytesDownloaded(0), totalBytesUploaded(0), infoHash(-1),
            localPeerId(-1), firstMarkEmitted(false), secondMarkEmitted(false),
            thirdMarkEmitted(false) {
}
ContentManager::~ContentManager() {
    std::cerr << this->localPeerId << " completed ";
    std::cerr << this->clientBitField.getCompletedPercentage();
    std::cerr << "% of " << this->infoHash << " - ";
    std::cerr << "missing: ";
    std::vector<bool> const& bitFieldVector =
            this->clientBitField.getBitFieldVector();

    for (unsigned int i = 0; i < bitFieldVector.size(); ++i) {
        if (!bitFieldVector[i]) {
            std::cerr << i << ", ";
        }
    }
    std::cerr << "\n";
}

void ContentManager::addEmptyBitField(int peerId) {
    Enter_Method("addEmptyBitField(id: %d)", peerId);

    if (this->peerBitFields.count(peerId)) {
        std::ostringstream out;
        out << "Peer with id '" << peerId << "' already has a BitField";
        throw std::logic_error(out.str());
    }

    // initializes all maps
    this->peerBitFields.insert(std::make_pair(peerId, BitField(
            this->clientBitField.size(), false)));
    this->pendingRequests[peerId]; // default empty set
    //    this->requestedPieces[peerId]; // default empty set
    this->totalDownloadedByPeer[peerId] = 0;
    this->totalUploadedByPeer[peerId] = 0;
}
void ContentManager::addPeerBitField(BitField const& bitField, int peerId) {
    Enter_Method("setPeerBitField(id: %d)", peerId);

    BitField & peerBitField = this->peerBitFields.at(peerId);

    if (!peerBitField.empty()) {
        std::ostringstream out;
        out << "Peer with id '" << peerId << "' already has a BitField.";
        throw std::logic_error(out.str());
    }

    // reset the Peer BitField
    peerBitField = bitField;

    // add BitField to pieceCount. Throw error if BitField is invalid
    this->rarestPieceCounter.addBitField(peerBitField);

    // initializes all maps
    this->peerBitFields.insert(std::make_pair(peerId, peerBitField));
    this->pendingRequests[peerId]; // default empty set
    //    this->requestedPieces[peerId]; // default empty set
    this->totalDownloadedByPeer[peerId] = 0;
    this->totalUploadedByPeer[peerId] = 0;

    if (this->clientBitField.isBitFieldInteresting(peerBitField)) {
        this->interestingPeers.insert(peerId);
        // peer interesting
        this->bitTorrentClient->peerInteresting(this->infoHash, peerId);
    } else if (peerBitField.full()) {
        // if this client is not interested in a seeder, that means it is
        // also a seeder and this connection has no use, so it is dropped.
        this->bitTorrentClient->drop(this->infoHash, peerId);
    }
}
void ContentManager::cancelPendingRequests(int peerId) {
    Enter_Method("cancelPendingRequests(index: %d)", peerId);

    // move the pieces from the requestedPieces set to the incomplete set
    std::set<std::pair<int, int> >::iterator it, currentIt, endIt;
    it = this->requestedPieces.begin();
    endIt = this->requestedPieces.end();

    std::ostringstream out;
    out << "Canceling requested pieces: ";
    while (it != endIt) {
        currentIt = it++;
        // pair <pieceIndex, peerId>
        if (currentIt->second == peerId) {
            this->requestedPieces.erase(currentIt);
            out << currentIt->first << " ";
        }
    }

    // Clear the requests for the current Peer. This allows for these requests
    // to be made to another Peer. If, for some reason, these canceled requests
    // arrive,
    this->pendingRequests.at(peerId).clear();

    this->printDebugMsg(out.str());
}
bool ContentManager::isBitFieldEmpty() {
    return this->clientBitField.empty();
}
BitFieldMsg* ContentManager::getClientBitFieldMsg() {
    Enter_Method("sendClientBitFieldMsg()");

    BitFieldMsg* bitFieldMsg = NULL;

    if (!this->clientBitField.empty()) {
        bitFieldMsg = new BitFieldMsg("BitFieldMsg");
        bitFieldMsg->setBitField(this->clientBitField);
        // bitField has variable length, so must set the variablePayloadLen attribute
        bitFieldMsg->setVariablePayloadLen(this->clientBitField.getByteSize());
    }

    return bitFieldMsg;
}
BitField const& ContentManager::getClientBitField() const{
	return this->clientBitField;
}
PeerWireMsgBundle* ContentManager::getNextRequestBundle(int peerId) {
    Enter_Method("getNextRequestBundle(index: %d)", peerId);

    // error if the peerId is not in the pendingRequestQueues, meaning that the
    // BitField was not defined.
    std::map<int, std::set<std::pair<int, int> > >::iterator
            peerPendingRequestsIt = this->pendingRequests.find(peerId);
    if (peerPendingRequestsIt == this->pendingRequests.end()) {
        std::ostringstream out;
        out << "Peer with id '" << peerId << "' is not in the ContentManager.";
        throw std::logic_error(out.str());
    }

    PeerWireMsgBundle* requestBundle = NULL;

    // make new request only if there are no pending ones
    if (peerPendingRequestsIt->second.empty()) {
        std::list<std::pair<int, int> > nextBlocksToRequestFromPeer =
                this->requestAvailableBlocks(peerId);

        // make requests with the blocks in this list
        if (!nextBlocksToRequestFromPeer.empty()) {
            std::ostringstream bundleMsgName;
            bundleMsgName << "RequestBundle(";

            cPacketQueue bundle;

            // fill the bundle to the limit
            while (bundle.getLength() < this->requestBundleSize
                    && !nextBlocksToRequestFromPeer.empty()) {
                std::pair<int, int> & block =
                        nextBlocksToRequestFromPeer.front();

                // save the instant the download started
                std::map<int, simtime_t>::iterator pieceRequestTimeIt;
                pieceRequestTimeIt = this->pieceRequestTime.lower_bound(
                        block.first);
                if (pieceRequestTimeIt == this->pieceRequestTime.end()
                        || pieceRequestTimeIt->second != block.first) {
                    this->pieceRequestTime.insert(pieceRequestTimeIt,
                            std::make_pair(block.first, simTime()));
                }

                // create the request and insert it in the bundle
                RequestMsg* request = this->createRequestMsg(block.first,
                        block.second);
                bundle.insert(request);
                bundleMsgName << "(" << block.first << "," << block.second
                        << ")";

                // insert the requested block in the pending set
                peerPendingRequestsIt->second.insert(block);

                // remove the block from the list
                nextBlocksToRequestFromPeer.pop_front();
            }
            bundleMsgName << ")";

            requestBundle = new PeerWireMsgBundle(bundleMsgName.str().c_str());
            requestBundle->setBundle(bundle);
        } else {
            std::ostringstream out;
            out << "There are no blocks available for download";
            this->printDebugMsg(out.str());
        }
    } else {
        std::ostringstream out;
        out << "There are requests pending";
        this->printDebugMsg(out.str());
    }

    return requestBundle;
}
PieceMsg* ContentManager::getPieceMsg(int peerId, int index, int begin,
        int reqLength) {
    Enter_Method(
            "getPieceMsg(peerId: %d, index: %d, begin: %d, reqLength: %d)",
            peerId, index, begin, reqLength);

    if (index < 0 || index >= this->numberOfPieces) {
        throw std::out_of_range("The piece index is out of bounds");
    }

    if (!this->clientBitField.hasPiece(index)) {
        std::ostringstream out;
        out << "The requested piece is not available" << std::endl;
        throw std::logic_error(out.str());
    }

    std::ostringstream name;
    name << "PieceMsg(" << index << ", " << begin / reqLength << ")";
    PieceMsg* pieceMsg = new PieceMsg(name.str().c_str());

    // sent this block to the Peer with the passed peerId
    this->totalUploadedByPeer[peerId] += reqLength;
    this->totalBytesUploaded += reqLength;
    emit(this->totalBytesUploaded_Signal, totalBytesUploaded);

    // fill in the pieceMessage attributes from the request.
    pieceMsg->setIndex(index);
    pieceMsg->setBegin(begin);
    pieceMsg->setBlockSize(reqLength);
    // piece has variable length, so must set the variablePayloadLen attribute
    pieceMsg->setVariablePayloadLen(reqLength);

    return pieceMsg;
}
int ContentManager::getTotalDownloaded(int peerId) {
    Enter_Method("getDownloadedBytes(peerId: %d)", peerId);
    // error if the peerId is not in the totalDownloaded, meaning that the
    // BitField was not defined.
    int totalDownloaded;
    try {
        totalDownloaded = this->totalDownloadedByPeer.at(peerId);
    } catch (std::out_of_range & e) {
        std::ostringstream out;
        out << "Peer with id '" << peerId << "' is not in the ContentManager.";
        throw std::logic_error(out.str());
    }
    return totalDownloaded;

}
int ContentManager::getTotalUploaded(int peerId) {
    Enter_Method("getUploadedBytes(peerId: %d)", peerId);
    // error if the peerId is not in the totalUploaded, meaning that the
    // BitField was not defined.
    int totalUploaded;
    try {
        totalUploaded = this->totalUploadedByPeer.at(peerId);
    } catch (std::out_of_range & e) {
        std::ostringstream out;
        out << "Peer with id '" << peerId << "' is not in the ContentManager.";
        throw std::logic_error(out.str());
    }
    return totalUploaded;
}
/*!
 * If a piece is completed with this pieceMsg, then schedule the sending of a
 * HaveMsg. Also, since a full piece arrived, the ContentManager must check the
 * interesting Peers to verify if they continue interesting to the Client.
 */
void ContentManager::processBlock(int peerId, int pieceIndex, int begin,
        int blockSize) {
    Enter_Method(
            "processPiece(peerId: %d, index: %d, begin: %d, blockSize: %d)",
            peerId, pieceIndex, begin, blockSize);

    if (pieceIndex < 0 || pieceIndex >= this->numberOfPieces) {
        throw std::out_of_range("The piece index is out of bounds");
    }

    int blockIndex = begin / blockSize;
    // remove the received block from the Peer's pending request queue
    this->pendingRequests.at(peerId).erase(std::make_pair(pieceIndex,
            blockIndex));

    // ignore block if the piece is already complete
    if (!this->clientBitField.hasPiece(pieceIndex)) {
        // find the incomplete piece, or create an empty one if not found
        std::map<int, Piece>::iterator pieceIt, pieceEndIt;
        pieceIt = this->incompletePieces.lower_bound(pieceIndex);
        pieceEndIt = this->incompletePieces.end();
        if (pieceIt == pieceEndIt || pieceIt->first != pieceIndex) {
            pieceIt = this->incompletePieces.insert(pieceIt, std::make_pair(
                    pieceIndex, Piece(pieceIndex, this->numberOfSubPieces)));
        }

        // set the block into the Piece, and verify if the piece is now complete
        if (pieceIt->second.setBlock(blockIndex)) {
            // downloaded whole piece
            {
                std::ostringstream out;
                out << "Downloaded piece " << pieceIndex;
                this->printDebugMsg(out.str());
            }

            if (this->clientBitField.empty()) {
                // First block fully downloaded, signaling the start of the download
                this->downloadStartTime = simTime();
            }

            // piece no longer incomplete
            this->incompletePieces.erase(pieceIndex);
            // add the piece to the Client's BitField
            this->clientBitField.addPiece(pieceIndex);

            // find if this piece was requested, then remove it from the set
            std::set<std::pair<int, int> >::iterator requestedPiecesIt;
            requestedPiecesIt = this->requestedPieces.lower_bound(
                    std::make_pair(pieceIndex, -1));

            if (requestedPiecesIt != this->requestedPieces.end()) {
                this->requestedPieces.erase(requestedPiecesIt);
            }

            // Statistics
            generateDownloadStatistics(pieceIndex);

            // schedule the sending of the HaveMsg to all Peers.
            this->bitTorrentClient->sendHaveMessage(this->infoHash, pieceIndex);

            // became seeder
            bool becameSeeder = this->clientBitField.full();
            if (becameSeeder) {
                std::ostringstream out;
                out << "Became a seeder";
                this->printDebugMsg(out.str());

                // warn the tracker
                this->bitTorrentClient->finishedDownload(this->infoHash);
            }

            // might drop this connection, if both the client and the peer are seeders.
            this->verifyInterestOnAllPeers();

            this->updateStatusString();
        }
    }
}
void ContentManager::processHaveMsg(int index, int peerId) {
    Enter_Method("updatePeerBitField(index: %d, id: %d)", index, peerId);

    if (index < 0 || index >= this->numberOfPieces) {
        throw std::out_of_range("The piece index is out of bounds");
    }

    try {
        BitField& peerBitField = this->peerBitFields.at(peerId);
        peerBitField.addPiece(index);

        this->rarestPieceCounter.addPiece(index);

        // Last piece resulted in the Peer becoming a seeder
        if (peerBitField.full()) {
            std::ostringstream out;
            if (this->clientBitField.full()) {
                out << "Both the Client and Peer " << peerId << " are seeders.";
                this->printDebugMsg(out.str());
                // the connection between two seeders is useless. Drop it.
                this->bitTorrentClient->drop(this->infoHash, peerId);
            } else {
                out << "Peer " << peerId << " became a seeder.";
                this->printDebugMsg(out.str());
            }
        }

        // verify if the peer with peerId became interesting.
        if (!this->interestingPeers.count(peerId)
                && this->clientBitField.isBitFieldInteresting(peerBitField)) {
            this->interestingPeers.insert(peerId);
            this->bitTorrentClient->peerInteresting(this->infoHash, peerId);
        }
    } catch (std::out_of_range & e) {
        std::ostringstream out;
        out << "Peer with id '" << peerId << "' not found.";
        throw std::logic_error(out.str());
    }
}
void ContentManager::removePeerInfo(int peerId) {
    if (this->peerBitFields.count(peerId)) {
        std::ostringstream out;
        out << "removing peer " << peerId;
        this->printDebugMsg(out.str());

        // subtract BitField from pieceCount
        this->rarestPieceCounter.removeBitField(this->peerBitFields[peerId]);

        // remove peerId from all maps
        this->interestingPeers.erase(peerId);

        this->peerBitFields.erase(peerId);
        this->pendingRequests.erase(peerId);

        // TODO can I really erase these before the end of simulation?
        this->totalDownloadedByPeer.erase(peerId);
        this->totalUploadedByPeer.erase(peerId);

        this->updateStatusString();
    }
}

// private methods
RequestMsg *ContentManager::createRequestMsg(int pieceIndex, int blockIndex) {
    // create the request and insert it in the bundle
    RequestMsg *request;
    std::ostringstream name;
    name << "RequestMsg(" << pieceIndex << ", " << blockIndex << ")";
    request = new RequestMsg(name.str().c_str());
    request->setIndex(pieceIndex);
    request->setBegin(blockIndex * this->subPieceSize);
    request->setReqLength(this->subPieceSize);
    // set the size of the request
    request->setByteLength(request->getHeaderLen() + request->getPayloadLen());
    return request;
}
void ContentManager::generateDownloadStatistics(int pieceIndex) {
    // Gather statistics in the completion time
    emit(this->pieceDownloadTime_Signal, simTime() - this->pieceRequestTime.at(
            pieceIndex));
    // request time already used, so delete it
    this->pieceRequestTime.erase(pieceIndex);

    simsignal_t markTime_Signal;
    bool emitSignal = false;

    // download statistics
    // when one of the download marks is reached, send the corresponding signal
    if (!this->firstMarkEmitted
            && this->clientBitField.getCompletedPercentage() >= 25.0) {
        this->firstMarkEmitted = emitSignal = true;
        markTime_Signal = this->_25_percentDownloadMarkTime_Signal;
    } else if (!this->secondMarkEmitted
            && this->clientBitField.getCompletedPercentage() >= 50.0) {
        this->secondMarkEmitted = emitSignal = true;
        markTime_Signal = this->_50_percentDownloadMarkTime_Signal;
    } else if (!this->thirdMarkEmitted
            && this->clientBitField.getCompletedPercentage() >= 75.0) {
        this->thirdMarkEmitted = emitSignal = true;
        markTime_Signal = this->_75_percentDownloadMarkTime_Signal;
    } else if (this->clientBitField.full()) {
        emitSignal = true;
        markTime_Signal = this->_100_percentDownloadMarkTime_Signal;
        emit(this->totalDownloadTime_Signal, simTime()
                - this->downloadStartTime);

        // send signal warning this ContentManager became a seeder
        emit(this->becameSeeder_Signal, this->infoHash);
    }

    if (emitSignal) {
        simtime_t downloadInterval = simTime() - this->downloadStartTime;

        std::ostringstream out;
        out << "Start: " << this->downloadStartTime;
        out << "Now: " << simTime();
        out << "Interval: " << downloadInterval;

        this->printDebugMsg(out.str());

        emit(this->downloadMarkPeerId_Signal, this->localPeerId);
        emit(markTime_Signal, downloadInterval);
    }
}
/*!
 * The returned list has enough elements to fill a request bundle.
 *
 * @param peerId The id of the Peer from which the blocks will be acquired.
 */
std::list<std::pair<int, int> > ContentManager::requestAvailableBlocks(
        int peerId) {
    std::list<std::pair<int, int> > availableBlocks;
    int numberOfBlocks;

    // Get a COPY of the Client BitField and use it to evaluate which pieces
    // from the current Peer are interesting.
    BitField filteredClientBitField = this->clientBitField;

    std::set<std::pair<int, int> >::const_iterator requestedPiecesIt,
            requestedPiecesEndIt;
    requestedPiecesIt = this->requestedPieces.begin();
    requestedPiecesEndIt = this->requestedPieces.end();

    // pieces requested to other Peers aren't available, and pieces requested by
    // this Peer must be completed first, so their remaining blocks are used.
    for (; requestedPiecesIt != requestedPiecesEndIt && availableBlocks.size()
            < this->requestBundleSize; ++requestedPiecesIt) {
        if (requestedPiecesIt->second != peerId) {
            // add the unavailable piece to the filteredClientBitField so it
            // will not be evaluated as interesting
            filteredClientBitField.addPiece(requestedPiecesIt->first);
        } else {
            // get the remaining blocks from the piece
            std::list<std::pair<int, int> > blocks = this->incompletePieces.at(
                    requestedPiecesIt->first).getMissingBlocks();
            // move the elements from block to returnBlocks
            availableBlocks.splice(availableBlocks.end(), blocks);
        }
    }

    // there still is space to request new pieces
    if (availableBlocks.size() < this->requestBundleSize) {
        // get interesting pieces only
        std::set<int> interestingPieces =
                filteredClientBitField.getInterestingPieces(
                        this->peerBitFields.at(peerId));

        if (!interestingPieces.empty()) {
            // get the rarest pieces among the interesting ones.
            std::vector<int> rarest = this->rarestPieceCounter.getRarestPieces(
                    interestingPieces, numberOfPieces);

            std::vector<int>::iterator rarestIt = rarest.begin();

            // get blocks until there are no more space available or until the
            // rarest list ends
            for (; rarestIt != rarest.end() && availableBlocks.size()
                    < this->requestBundleSize; ++rarestIt) {

                int pieceId = *rarestIt;
                // this piece is now requested
                this->requestedPieces.insert(std::make_pair(pieceId, peerId));

                // find the incomplete piece, or create it if necessary
                std::map<int, Piece>::iterator incompleteIt =
                        this->incompletePieces.lower_bound(pieceId);

                if (incompleteIt == this->incompletePieces.end()
                        || incompleteIt->first != pieceId) {
                    // this piece was never requested, so create a new one
                    Piece emptyPiece(pieceId, this->numberOfSubPieces);
                    incompleteIt = this->incompletePieces.insert(incompleteIt,
                            std::make_pair(pieceId, emptyPiece));
                }

                // insert the missing blocks into the availableBlocks
                std::list<std::pair<int, int> > missingBlocks;
                missingBlocks = incompleteIt->second.getMissingBlocks();
                availableBlocks.splice(availableBlocks.end(), missingBlocks);
            }
        }
    }

    return availableBlocks;
}
void ContentManager::verifyInterestOnAllPeers() {
    // Send NotInterestedMsg to all Peers in which the client lost interest
    // and, if now seeding, disconnect from all other seeders
    std::set<int>::iterator interestingPeersIt = this->interestingPeers.begin();

    // Send a NotInterested message to each Peer that is no longer
    // interesting to the Client.
    while (interestingPeersIt != this->interestingPeers.end()) {
        // Copy then increment. Deleting this iterator don't invalidate it.
        std::set<int>::iterator currentPeerIt = interestingPeersIt++;
        int peerId = *currentPeerIt;

        BitField const& peerBitField = this->peerBitFields.at(peerId);

        // Drop this connection because it is no longer useful.
        if (peerBitField.full() && this->clientBitField.full()) {
            std::ostringstream out;
            out << "Both the Client and Peer " << peerId << " are seeders.";
            this->printDebugMsg(out.str());
            this->bitTorrentClient->drop(this->infoHash, peerId);
        } else if (!this->clientBitField.isBitFieldInteresting(peerBitField)) {
            // not interesting anymore
            this->interestingPeers.erase(currentPeerIt);
            this->bitTorrentClient->peerNotInteresting(this->infoHash, peerId);
        }
    }
}
// signal methods
void ContentManager::registerEmittedSignals() {
    // Configure signals
    this->becameSeeder_Signal = registerSignal("ContentManager_BecameSeeder");
    this->pieceDownloadTime_Signal = registerSignal("ContentManager_PieceDownloadTime");
    this->totalDownloadTime_Signal = registerSignal("ContentManager_TotalDownloadTime");
    this->totalBytesDownloaded_Signal = registerSignal("ContentManager_TotalBytesDownloaded");
    this->totalBytesUploaded_Signal = registerSignal("ContentManager_TotalBytesUploaded");

    this->_25_percentDownloadMarkTime_Signal = registerSignal("ContentManager_25_percentDownloadMarkTime");
    this->_50_percentDownloadMarkTime_Signal = registerSignal("ContentManager_50_percentDownloadMarkTime");
    this->_75_percentDownloadMarkTime_Signal = registerSignal("ContentManager_75_percentDownloadMarkTime");
    this->_100_percentDownloadMarkTime_Signal = registerSignal("ContentManager_100_percentDownloadMarkTime");

    this->downloadMarkPeerId_Signal = registerSignal("ContentManager_DownloadMarkPeerId");
}
void ContentManager::subscribeToSignals() {
}
// module methods
void ContentManager::printDebugMsg(std::string s) {
    if (this->debugFlag) {
        // debug "header"
        std::cerr << simulation.getEventNumber() << " (T=";
        std::cerr << simulation.getSimTime() << ")(ContentManager) - ";
        std::cerr << "Peer " << this->localPeerId;
        std::cerr << ": infoHash " << this->infoHash << " - ";
        std::cerr << s << "\n";
        std::cerr.flush();
    }
}
void ContentManager::updateStatusString() {
    if (ev.isGUI()) {
        std::ostringstream out;
        if (this->clientBitField.full()) {
            out << "Seeder";
        } else {
            out << std::setprecision(2) << std::fixed
                    << this->clientBitField.getCompletedPercentage() << "%";
        }

        // TODO show more info
        getDisplayString().setTagArg("t", 0, out.str().c_str());
    }
}

// protected methods
void ContentManager::initialize() {
    this->registerEmittedSignals();

    // TODO allow peers to start with random BitFields.
    // will throw an error if this module is not owned by a SwarmManager
    SwarmManager * swarmManager = check_and_cast<SwarmManager*> (
            getParentModule());
    this->localPeerId
            = swarmManager->getParentModule()->getParentModule()->getId();

    cModule* bitTorrentClient = swarmManager->getParentModule()->getSubmodule(
            "bitTorrentClient");
    //    cModule* statistics = swarmManager->getParentModule()->getSubmodule(
    //            "statistics");

    if (bitTorrentClient == NULL) {
        throw cException("BitTorrentClient module not found");
    }
    //    if (statistics == NULL) {
    //        throw cException("Statistics module not found");
    //    }

    this->bitTorrentClient = check_and_cast<BitTorrentClient*> (
            bitTorrentClient);
    //    this->clientController = check_and_cast<ClientController*> (
    //            clientController);
    //    this->statistics = check_and_cast<Statistics*> (statistics);

    // setting parameters from NED
    this->numberOfPieces = par("numOfPieces").longValue();
    this->numberOfSubPieces = par("numOfSubPieces").longValue();
    this->subPieceSize = par("subPieceSize").longValue();

    this->requestBundleSize = par("requestBundleSize").longValue();
    this->haveBundleSize = par("haveBundleSize").longValue();
    this->infoHash = par("infoHash").longValue();
    bool seeder = par("seeder").boolValue();

    // create an empty or full BitField, depending on the parameter "seeder"
    this->clientBitField = BitField(this->numberOfPieces, seeder);

    if (seeder) {
        this->updateStatusString();
    }

    // create an empty RarestPieceCounter
    this->rarestPieceCounter = RarestPieceCounter(this->numberOfPieces);

    //	this->downloadTime.setName("Download time");
    //	this->requestTime.setName("Request time");
    //	this->responseTime.setName("Response time");

    //    WATCH_MAP(this->peerBitFields);
    //    WATCH_MAP(this->pendingRequests);
    //    WATCH_MAP(this->totalDownloadedByPeer);
    //    WATCH_MAP(this->totalUploadedByPeer);
    //    WATCH_SET(this->requestedPieces);
    //    WATCH_SET(this->interestingPeers);
    //    WATCH(this->clientBitField);

    this->debugFlag = par("debugFlag").boolValue();
}

void ContentManager::handleMessage(cMessage *msg) {
    delete msg;
    throw cException("This module doesn't process messages");
}

std::ostream& operator<<(std::ostream& out,
        std::set<std::pair<int, int> > const& reqQueue) {
    std::set<std::pair<int, int> >::iterator it = reqQueue.begin();

    for (; it != reqQueue.end(); ++it) {
        out << "(" << it->first << ", " << it->second << ") ";
    }

    return out;
}

std::ostream& operator<<(std::ostream& out,
        std::pair<int, int> const& currentPiece) {
    out << "- (" << currentPiece.first << ", " << currentPiece.second << ")";
    return out;
}
