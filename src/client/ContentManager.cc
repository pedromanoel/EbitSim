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
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <deque>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include "BitTorrentClient.h"
#include "SwarmManager.h"
#include "PeerWire_m.h"
#include "PeerWireMsgBundle_m.h"

namespace {
std::string toStr(long i) {
    return boost::lexical_cast<std::string>(i);
}
}

//!@name Rate Control using token bucket
class ContentManager::TokenBucket {
public:
    TokenBucket(ContentManager * contentManager, int subPieceSize,
        int burstSize, unsigned long bytesSec) :
            tokenIncTimerMsg("Add Tokens") {
        this->contentManager = static_cast<ContentManager *>(contentManager);
        this->bucketSize = subPieceSize * burstSize;
        this->tokenSize = subPieceSize;
        this->tokens = 0;
        this->incInterval = ((double) subPieceSize / bytesSec);
        this->currentQueueIt = this->requestQueues.begin();
        this->scheduleAt();
    }
    ~TokenBucket() {
        this->contentManager->cancelEvent(&this->tokenIncTimerMsg);
    }
    PieceMsg* getPieceMsg(int peerId) {
        RequestQueueMapIt sendQueueIt = this->sendQueues.find(peerId);
        if (sendQueueIt == this->requestQueues.end()) {
            throw std::logic_error("No piece found.");
        }
        // Get the request info and delete it from the queue
        RequestQueue & peerQueue = sendQueueIt->second;
        PieceRequest * pieceRequest = peerQueue.front();
        int begin = pieceRequest->begin;
        int index = pieceRequest->index;
        int reqLength = pieceRequest->reqLength;
        peerQueue.pop();
        delete pieceRequest;

        // erase the queue if empty
        if (peerQueue.empty()) {
            this->sendQueues.erase(sendQueueIt);
        }

        std::string name = "PieceMsg(" + toStr(index) + ", "
            + toStr(begin / reqLength) + ")";
        PieceMsg* pieceMsg = new PieceMsg(name.c_str());

        // sent this block to the Peer with the passed peerId
        this->contentManager->totalUploadedByPeer[peerId] += reqLength;
        this->contentManager->totalBytesUploaded += reqLength;
        this->contentManager->emit(
            this->contentManager->totalBytesUploaded_Signal,
            this->contentManager->totalBytesUploaded);

        // fill in the pieceMessage attributes from the request.
        pieceMsg->setIndex(index);
        pieceMsg->setBegin(begin);
        pieceMsg->setBlockSize(reqLength);
        // piece has variable length, so must set the variablePayloadLen attribute
        pieceMsg->setVariablePayloadLen(reqLength);
        return pieceMsg;
    }
    void requestPieceMsg(int peerId, int index, int begin, int reqLength) {
        // store the request
        PieceRequest * pieceRequest = new PieceRequest();
        pieceRequest->begin = begin;
        pieceRequest->index = index;
        pieceRequest->reqLength = reqLength;

        this->requestQueues[peerId].push(pieceRequest);
        // Fix the iterator, since the queue is definitely not empty anymore
        this->fixCurrentIt();
        this->tryToSend();
    }
    /*!
     * Cancel all requests in the peer pending and send queues.
     */
    void cancelUploadRequests(int peerId) {
        RequestQueueMapIt sendIt = this->sendQueues.find(peerId);
        if (sendIt != this->sendQueues.end()) {
            // Erase the corresponding send queue
            RequestQueue & sendQueue = sendIt->second;
            while (!sendQueue.empty()) {
                PieceRequest * p = sendQueue.front();
                this->addTokens(p->reqLength);
                this->contentManager->printDebugMsg(
                    "Canceling piece send (" + toStr(p->index) + ", "
                        + toStr(p->begin / p->reqLength) + ")");
                delete p;
                sendQueue.pop();
            }
            this->sendQueues.erase(sendIt);
        }
        // Erase the request queue
        RequestQueueMapIt reqIt = this->requestQueues.find(peerId);
        if (reqIt != this->requestQueues.end()) {
            RequestQueue & requestQueue = reqIt->second;
            std::string out = "Canceling piece requests :";
            while (!requestQueue.empty()) {
                PieceRequest * p = requestQueue.front();
                this->addTokens(p->reqLength);
                out += "(" + toStr(p->index) + ", "
                    + toStr(p->begin / p->reqLength) + ") ";
                delete p;
                requestQueue.pop();
            }
            this->contentManager->printDebugMsg(out);
            // erase the request queue and keep the currentQueueIt valid
            if (this->currentQueueIt == reqIt) {
                this->requestQueues.erase(this->currentQueueIt++);
                this->fixCurrentIt();
            } else {
                this->requestQueues.erase(reqIt);
            }
        }
    }
    /*!
     * Handle the increment timer by adding tokenSize to the bucket. If the
     * bucket size was reached, don't schedule the next timeout.
     */
    void handleMessage(cMessage *msg) {
        if (msg == &this->tokenIncTimerMsg) {
            if (this->addTokens(this->tokenSize)) {
                // Schedule the next timeout
                this->scheduleAt();
            }
            this->tryToSend();
        }
    }
private:
    typedef std::queue<PieceRequest *> RequestQueue;
    typedef std::map<int, RequestQueue> RequestQueueMap;
    typedef RequestQueueMap::iterator RequestQueueMapIt;

    //! Schedule the timer used to increment the tokens.
    void scheduleAt() {
        simtime_t now = simTime();
        this->contentManager->scheduleAt(now + this->incInterval,
            &this->tokenIncTimerMsg);

    }
    /*!
     * Add tokens to the bucket and cap them to the bucket size.
     * Return true if the maximum number of tokens (bucket size) was not reached.
     */
    bool addTokens(unsigned long tokenSize) {
        std::string out;
        // cap the number of tokens
        this->tokens += tokenSize;
        if (this->tokens > this->bucketSize) {
            this->tokens = this->bucketSize;
            out = "Max tokens reached: ";
        } else {
            out = "Tokens added: ";
        }
        this->contentManager->printDebugMsg(out + toStr(this->tokens));
        return this->tokens < this->bucketSize;
    }
    /*!
     * Try to send as many messages as possible, one requester at a time, to
     * ensure that all requesters have the same bandwidth.
     */
    void tryToSend() {
        bool hasEnoughTokens = true;
        // Prepare one message from each queue, until there are no tokens or no
        // more messages
        while (hasEnoughTokens && !this->requestQueues.empty()) {
            RequestQueue & currentQueueRef = this->currentQueueIt->second;
            int reqLength = static_cast<PieceRequest *>(currentQueueRef.front())
                ->reqLength;
            hasEnoughTokens = reqLength <= this->tokens;
            if (hasEnoughTokens) {
                // Consume the tokens
                this->tokens -= reqLength;
                std::string out = "Tokens subtracted: " + toStr(this->tokens);
                this->contentManager->printDebugMsg(out);

                // Move the request from the requestQueue to the sendQueue
                int peerId = this->currentQueueIt->first;
                sendQueues[peerId].push(currentQueueRef.front());
                currentQueueRef.pop();
                // Warn the thread that there's a piece waiting
                this->contentManager->bitTorrentClient->sendPieceMessage(
                    this->contentManager->infoHash, peerId);

                // forward the iterator to the current request queue

                // If empty, erase the request queue. Also forward the current it
                if (currentQueueRef.empty()) {
                    this->requestQueues.erase(this->currentQueueIt++);
                } else {
                    ++this->currentQueueIt;
                }
                this->fixCurrentIt();
                // Un-pause the increment timer (if paused)
                if (!this->tokenIncTimerMsg.isScheduled()) {
                    scheduleAt();
                }
            }
        }
    }
    /*!
     * If the requestQueues is not empty, keep the current iterator circular.
     * That means that if the iterator points to end, go back to the beginning.
     */
    void fixCurrentIt() {
        if (!this->requestQueues.empty()
            && this->currentQueueIt == this->requestQueues.end()) {
            this->currentQueueIt = this->requestQueues.begin();
        }
    }
private:
    ContentManager * contentManager;

//! The number of PieceMsgs that could be sent in a burst.
    unsigned long bucketSize;
//! The number of bytes added at each interval.
    unsigned long tokenSize;
//! The current number of tokens, in bytes.
    unsigned long tokens;
//! The interval between addition of tokens. t = subPieceSize/bytesSec.
    simtime_t incInterval;
//! Timer message for adding tokens.
    cMessage tokenIncTimerMsg;
//! Iterator to the queue from where the next message will be popped.
    RequestQueueMapIt currentQueueIt;
//! Maps peerIds to the piece requests they made.
    RequestQueueMap requestQueues;
//! Maps peerIds to the pieces waiting to be transmitted.
    RequestQueueMap sendQueues;
};

ContentManager::Piece::Piece(int requesterPeerId, int pieceIndex,
    int numOfBlocks) :
        requesterPeerId(requesterPeerId), downloadedBlocks(0),
            numOfBlocks(numOfBlocks), pieceIndex(pieceIndex),
            blocks(numOfBlocks) {
}
bool ContentManager::Piece::setBlock(int blockIndex) {
    this->blocks[blockIndex] = 1;
    return this->blocks.count() == this->numOfBlocks;
}
std::list<std::pair<int, int> > ContentManager::Piece::getMissingBlocks() const {
    std::list<std::pair<int, int> > missingBlocks;

    for (int i = 0; i < this->numOfBlocks; ++i) {
        if (!this->blocks.test(i)) {
            missingBlocks.push_back(std::make_pair(this->pieceIndex, i));
        }
    }

    return missingBlocks;
}
int ContentManager::Piece::getPieceIndex() const {
    return this->pieceIndex;
}
int ContentManager::Piece::getRequesterId() const {
    return this->requesterPeerId;
}
std::string ContentManager::Piece::str() const {
    std::string strRep;
    boost::to_string(this->blocks, strRep);
    return strRep;
}

Define_Module(ContentManager);

ContentManager::ContentManager() :
        bitTorrentClient(NULL), tokenBucket(NULL), subPieceSize(0),
            debugFlag(false), haveBundleSize(0), numberOfSubPieces(0),
            numberOfPieces(0), requestBundleSize(0), totalBytesDownloaded(0),
            totalBytesUploaded(0), infoHash(-1), localPeerId(-1),
            firstMarkEmitted(false), secondMarkEmitted(false),
            thirdMarkEmitted(false) {
}
ContentManager::~ContentManager() {
    delete this->tokenBucket;
    std::cerr << this->localPeerId << " completed ";
    std::cerr << this->clientBitField.getCompletedPercentage();
    std::cerr << "% of " << this->infoHash << " - ";
    std::cerr << this->clientBitField.unavailablePieces();
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
    this->peerBitFields.insert(
        std::make_pair(peerId, BitField(this->clientBitField.size(), false)));
    this->pendingRequests[peerId]; // default empty set
    this->totalDownloadedByPeer[peerId] = 0;
    this->totalUploadedByPeer[peerId] = 0;
}
void ContentManager::addPeerBitField(BitField const& bitField, int peerId) {
    Enter_Method("setPeerBitField(id: %d)", peerId);

    BitField & peerBitField = this->peerBitFields.at(peerId);

    if (!peerBitField.empty()) {
        std::string out = "Peer with id '" + toStr(peerId)
            + "' already has a BitField.";
        throw std::logic_error(out);
    }

// reset the Peer BitField
    peerBitField = bitField;

// add BitField to pieceCount. Throw error if BitField is invalid
    this->rarestPieceCounter.addBitField(peerBitField);

// initializes all maps
    this->peerBitFields.insert(std::make_pair(peerId, peerBitField));
    this->pendingRequests[peerId]; // default empty set
    this->totalDownloadedByPeer[peerId] = 0;
    this->totalUploadedByPeer[peerId] = 0;

    if (this->clientBitField.isBitFieldInteresting(peerBitField)) {
        this->interestingPeers.insert(peerId);
        // peer interesting
        this->bitTorrentClient->peerInteresting(this->infoHash, peerId);
    } else if (peerBitField.full()) {
        // if this client is not interested in a seeder, that means it is
        // also a seeder and this connection has no use, so it is dropped.
        this->bitTorrentClient->closeConnection(this->infoHash, peerId);
    }
}
void ContentManager::cancelDownloadRequests(int peerId) {
    Enter_Method("cancelDownloadRequests(index: %d)", peerId);

    std::ostringstream out;
    out << "Canceling requested pieces: ";
// Clear the requests for the current Peer. This allows for these requests
// to be made to another Peer. If, for some reason, these canceled requests
// arrive,
    typedef std::pair<int, int> pending_pair_t;
    BOOST_FOREACH(pending_pair_t const& p, pendingRequests.at(peerId))
            {
                int pieceIndex = p.first;
                int blockIndex = p.second;
                out << "(" << pieceIndex << ", " << blockIndex << ") ";
            }
    this->pendingRequests.at(peerId).clear();
    this->printDebugMsg(out.str());
}
void ContentManager::cancelUploadRequests(int peerId) {
    Enter_Method("cancelUploadRequests(index: %d)", peerId);
    this->tokenBucket->cancelUploadRequests(peerId);
}
bool ContentManager::isBitFieldEmpty() {
    Enter_Method("isBitFieldEmpty()");
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
PeerWireMsgBundle* ContentManager::getNextRequestBundle(int peerId) {
    Enter_Method("getNextRequestBundle(index: %d)", peerId);

// error if the peerId is not in the pendingRequestQueues, meaning that the
// BitField was not defined.
    std::map<int, std::set<std::pair<int, int> > >::iterator peerPendingRequestsIt =
        this->pendingRequests.find(peerId);
    if (peerPendingRequestsIt == this->pendingRequests.end()) {
        std::string out = "Peer with id '" + toStr(peerId)
            + "' is not in the ContentManager.";
        throw std::logic_error(out);
    }

    PeerWireMsgBundle* requestBundle = NULL;

// make new request only if there are no pending ones
    if (peerPendingRequestsIt->second.empty()) {
        std::list<std::pair<int, int> > nextBlocksToRequestFromPeer = this
            ->requestAvailableBlocks(peerId);

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
void ContentManager::requestPieceMsg(int peerId, int index, int begin,
    int reqLength) {
    Enter_Method(
        "requestPieceMsg(peerId: %d, index: %d, begin: %d, reqLength: %d)",
        peerId, index, begin, reqLength);

    if (index < 0 || index >= this->numberOfPieces) {
        throw std::out_of_range("The piece index is out of bounds");
    }

    if (!this->clientBitField.hasPiece(index)) {
        std::ostringstream out;
        out << "The requested piece is not available" << std::endl;
        throw std::logic_error(out.str());
    }
    this->tokenBucket->requestPieceMsg(peerId, index, begin, reqLength);
}

PieceMsg* ContentManager::getPieceMsg(int peerId) {
    Enter_Method("getPieceMsg(peerId: %d)", peerId);
    return this->tokenBucket->getPieceMsg(peerId);
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
        std::string out = "Peer with id '" + toStr(peerId)
            + "' is not in the ContentManager.";
        throw std::logic_error(out);
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
        "processPiece(peerId: %d, index: %d, begin: %d, blockSize: %d)", peerId,
        pieceIndex, begin, blockSize);

    if (pieceIndex < 0 || pieceIndex >= this->numberOfPieces) {
        throw std::out_of_range("The piece index is out of bounds");
    }

// update bytes counter
    this->totalDownloadedByPeer[peerId] += blockSize;
    this->totalBytesDownloaded += blockSize;
    emit(this->totalBytesDownloaded_Signal, this->totalBytesDownloaded);

    int blockIndex = begin / blockSize;
// remove the received block from the Peer's pending request queue
    this->pendingRequests.at(peerId).erase(
        std::make_pair(pieceIndex, blockIndex));

// ignore block if the piece is already complete
    if (!this->clientBitField.hasPiece(pieceIndex)) {
        // Return the incomplete piece, or create a new one
        // similar to operator[], but without the default constructor
        // http://cplusplus.com/reference/stl/map/operator[]/
        Piece & piece = this->incompletePieces.insert(
            std::make_pair(pieceIndex,
                Piece(peerId, pieceIndex, this->numberOfSubPieces))).first
            ->second;

        // set the block into the Piece, and verify if the piece is now complete
        if (piece.setBlock(blockIndex)) {
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
            std::string out;
            std::string peerIdStr = toStr(peerId);
            if (this->clientBitField.full()) {
                out = "Both the Client and Peer " + peerIdStr + " are seeders.";
                // the connection between two seeders is useless. Drop it.
                this->bitTorrentClient->closeConnection(this->infoHash, peerId);
            } else {
                out = "Peer " + peerIdStr + " became a seeder.";
            }
            this->printDebugMsg(out);
        }

        // Verify if the peer with peerId became interesting only if the piece
        // is not being requested already (strict priority policy)
        if (!this->incompletePieces.count(index)
            && !this->interestingPeers.count(peerId)
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
    emit(this->pieceDownloadTime_Signal,
        simTime() - this->pieceRequestTime.at(pieceIndex));
    emit(this->downloadPiece_Signal, pieceIndex);
// request time already used, so delete it
    this->pieceRequestTime.erase(pieceIndex);

    simsignal_t markTime_Signal;
    bool emitCompletionSignal = false;

// download statistics
// when one of the download marks is reached, send the corresponding signal
    if (!this->firstMarkEmitted
        && this->clientBitField.getCompletedPercentage() >= 25.0) {
        this->firstMarkEmitted = emitCompletionSignal = true;
        markTime_Signal = this->_25_percentDownloadMarkTime_Signal;
    } else if (!this->secondMarkEmitted
        && this->clientBitField.getCompletedPercentage() >= 50.0) {
        this->secondMarkEmitted = emitCompletionSignal = true;
        markTime_Signal = this->_50_percentDownloadMarkTime_Signal;
    } else if (!this->thirdMarkEmitted
        && this->clientBitField.getCompletedPercentage() >= 75.0) {
        this->thirdMarkEmitted = emitCompletionSignal = true;
        markTime_Signal = this->_75_percentDownloadMarkTime_Signal;
    } else if (this->clientBitField.full()) {
        emitCompletionSignal = true;
        markTime_Signal = this->_100_percentDownloadMarkTime_Signal;
        emit(this->totalDownloadTime_Signal,
            simTime() - this->downloadStartTime);

        // send signal warning this ContentManager became a seeder
        emit(this->becameSeeder_Signal, this->infoHash);
    }

    if (emitCompletionSignal) {
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
    std::list<std::pair<int, int> > availBlocks;

// Get a COPY of the Client BitField and use it to evaluate which pieces
// from the current Peer are interesting.
    BitField clientBitFieldCopy = this->clientBitField;

// get blocks from previously requested pieces
    typedef std::pair<int, Piece> incomplete_pair_t;
    BOOST_FOREACH(incomplete_pair_t const p, this->incompletePieces)
            {
                int pieceId = p.first;
                Piece const& pieceRef = p.second;

                clientBitFieldCopy.addPiece(pieceId); // ignore pieces requested to other peers

                if (pieceRef.getRequesterId() == peerId) {
                    std::list<std::pair<int, int> > blocks;
                    blocks = pieceRef.getMissingBlocks();
                    // append blocks to availBlocks
                    availBlocks.splice(availBlocks.end(), blocks);
                }

                if (availBlocks.size() >= this->requestBundleSize) {
                    break;
                }
            }

// If there is space in the bundle, add blocks from interesting pieces
    if (availBlocks.size() < this->requestBundleSize) {
        std::set<int> intPieces = clientBitFieldCopy.getInterestingPieces(
            this->peerBitFields.at(peerId));

        if (!intPieces.empty()) {
            // get the rarest pieces among the interesting ones.
            std::vector<int> rarest = this->rarestPieceCounter.getRarestPieces(
                intPieces, numberOfPieces);

            // get blocks until there are no more space available or until the
            BOOST_FOREACH(int pieceIndex, rarest)
                    {
                        // Return the incomplete piece, or create a new one
                        // similar to operator[], but without the default constructor
                        // http://cplusplus.com/reference/stl/map/operator[]/
                        Piece & piece = this->incompletePieces.insert(
                            std::make_pair(
                                pieceIndex,
                                Piece(peerId, pieceIndex,
                                    this->numberOfSubPieces))).first->second;
                        // insert the missing blocks into the availableBlocks
                        std::list<std::pair<int, int> > missingBlocks;
                        missingBlocks = piece.getMissingBlocks();
                        availBlocks.splice(availBlocks.end(), missingBlocks);

                        if (availBlocks.size() >= this->requestBundleSize) {
                            break;
                        }
                    }
        }
    }

    return availBlocks;
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

        // ignore already requested pieces
        BitField clientBitFieldCopy = this->clientBitField;
//        typedef std::pair<int, Piece> pair_t;
//        BOOST_FOREACH(pair_t p, this->incompletePieces) {
//            clientBitFieldCopy.addPiece(p.first);
//        }

        BitField const& peerBitField = this->peerBitFields.at(peerId);
        // Drop this connection because it is no longer useful.
        if (peerBitField.full() && this->clientBitField.full()) {
            std::string out = "Both the Client and Peer " + toStr(peerId)
                + " are seeders.";
            this->printDebugMsg(out);
            this->bitTorrentClient->closeConnection(this->infoHash, peerId);
        } else if (!clientBitFieldCopy.isBitFieldInteresting(peerBitField)) {
            // not interesting anymore
            this->interestingPeers.erase(currentPeerIt);
            this->bitTorrentClient->peerNotInteresting(this->infoHash, peerId);
        }
    }
}
//void ContentManager::eraseRequestedPiece(int pieceIndex) {
//    IntPairSetIt requestedPiecesIt;
//    requestedPiecesIt = this->requestedPieces.lower_bound(
//        std::make_pair(pieceIndex, -1));
//
//    if (requestedPiecesIt != this->requestedPieces.end()) {
//        this->requestedPieces.erase(requestedPiecesIt);
//    }
//}
// signal methods
void ContentManager::registerEmittedSignals() {
// Configure signals
#define SIGNAL(X, Y) this->X = registerSignal("ContentManager_" #Y)
    SIGNAL(becameSeeder_Signal, BecameSeeder);
    SIGNAL(pieceDownloadTime_Signal, PieceDownloadTime);
    SIGNAL(totalDownloadTime_Signal, TotalDownloadTime);
    SIGNAL(totalBytesDownloaded_Signal, TotalBytesDownloaded);

    SIGNAL(totalBytesUploaded_Signal, TotalBytesUploaded);
    SIGNAL(_25_percentDownloadMarkTime_Signal, 25_percentDownloadMarkTime);
    SIGNAL(_50_percentDownloadMarkTime_Signal, 50_percentDownloadMarkTime);
    SIGNAL(_75_percentDownloadMarkTime_Signal, 75_percentDownloadMarkTime);
    SIGNAL(_100_percentDownloadMarkTime_Signal, 100_percentDownloadMarkTime);

    SIGNAL(downloadMarkPeerId_Signal, DownloadMarkPeerId);
    SIGNAL(downloadPiece_Signal, DownloadPiece);
#undef SIGNAL
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
    SwarmManager * swarmManager = check_and_cast<SwarmManager*>(
        getParentModule());
    this->localPeerId = swarmManager->getParentModule()->getParentModule()
        ->getId();

    cModule* bitTorrentClient = swarmManager->getParentModule()->getSubmodule(
        "bitTorrentClient");

    if (bitTorrentClient == NULL) {
        throw cException("BitTorrentClient module not found");
    }

    this->bitTorrentClient = check_and_cast<BitTorrentClient*>(
        bitTorrentClient);

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

// Token bucket stuff
    this->tokenBucket = new TokenBucket(this, this->subPieceSize,
        par("burstSize").longValue(), par("bytesSec").longValue());

// create an empty RarestPieceCounter
    this->rarestPieceCounter = RarestPieceCounter(this->numberOfPieces);
    this->debugFlag = par("debugFlag").boolValue();
}

void ContentManager::handleMessage(cMessage *msg) {
    this->tokenBucket->handleMessage(msg);
}
