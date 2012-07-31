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
#include <cassert>
#include <iostream>
#include <iomanip>
#include <queue>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include "BitTorrentClient.h"
#include "PeerWire_m.h"
#include "PeerWireMsgBundle_m.h"

// Dumb fix because of the CDT parser (https://bugs.eclipse.org/bugs/show_bug.cgi?id=332278)
#ifdef __CDT_PARSER__
#undef BOOST_FOREACH
#define BOOST_FOREACH(a, b) for(a; b; )
#endif

namespace {
std::string toStr(long i) {
    return boost::lexical_cast<std::string>(i);
}
}

/*!
 * Object that keeps track of pending requests for a piece.
 */
class ContentManager::PieceBlocks {
public:
    PieceBlocks(int pieceIndex, int numOfBlocks) :
        numOfBlocks(numOfBlocks), pieceIndex(pieceIndex), blocks(numOfBlocks) {
        for (unsigned int i = 0; i < this->numOfBlocks; ++i) {
            this->requests.push_back(std::make_pair(this->pieceIndex, i));
        }
    }
    //! Reset the request list to contain all blocks that are not set yet.
    bool resetRequests() {
        std::list<std::pair<int, int> > requests;
        for (unsigned int i = 0; i < this->numOfBlocks; ++i) {
            if (!this->blocks.test(i)) {
                requests.push_back(std::make_pair(this->pieceIndex, i));
            }
        }
        this->requests = requests;
    }
    //! Set the passed block inside the piece. Return true if the block was unset.
    bool setBlock(int blockIndex) {
        bool unset = !this->blocks.test(blockIndex);
        this->blocks[blockIndex] = 1;
        return unset;
    }
    //! Return true if all blocks were set already.
    bool isComplete() {
        return this->blocks.count() == this->numOfBlocks;
    }
    /*!
     * Return a reference to the list of requests not performed yet.
     *
     * A request is removed from this list when used.
     * @return Reference to the list of requests, where each request is a pair
     * (pieceIndex, blockIndex) that was not set yet.
     */
    std::list<std::pair<int, int> > & getRequests() {
        return this->requests;
    }
private:
    unsigned int numOfBlocks;
    unsigned int pieceIndex;
    std::list<std::pair<int, int> > requests;
    boost::dynamic_bitset<> blocks;
};

//!@name Rate Control using token bucket
class ContentManager::TokenBucket {
private:
    //! Piece request abstraction
    struct Request {
        int index;
        int begin;
        int reqLength;
    };
    typedef std::queue<Request *> RequestQueue;
    typedef std::map<int, RequestQueue> RequestQueueMap;
    typedef RequestQueueMap::iterator RequestQueueMapIt;

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
    //! Maps peerIds to the piece blocks they made.
    RequestQueueMap requestQueues;
    //! Maps peerIds to the pieces waiting to be transmitted.
    RequestQueueMap sendQueues;
public:
    TokenBucket(ContentManager * contentManager, int subPieceSize,
        int burstSize, unsigned long bytesSec) :
        tokenIncTimerMsg("Add Tokens") {
        this->contentManager = contentManager;
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
        if (!this->sendQueues.count(peerId)) {
            return NULL;
        }

        // Get the request info and delete it from the queue
        RequestQueue & peerQueue = this->sendQueues.at(peerId);
        Request * pieceRequest = peerQueue.front();
        peerQueue.pop();
#ifdef DEBUG_MSG
        std::string out = "Piece " + this->getRequestName(pieceRequest)
            + " taken by peer " + toStr(peerId);
        this->contentManager->printDebugMsg(out);
#endif

        std::string name = "PieceMsg" + this->getRequestName(pieceRequest);
        PieceMsg* pieceMsg = new PieceMsg(name.c_str());

        int reqLength = pieceRequest->reqLength;
        // sent this block to the Peer with the passed peerId
        this->contentManager->totalUploadedByPeer[peerId] += reqLength;
        this->contentManager->totalBytesUploaded += reqLength;
        this->contentManager->emit(
            this->contentManager->totalBytesUploaded_Signal,
            this->contentManager->totalBytesUploaded);

        // fill in the pieceMessage attributes from the request.
        pieceMsg->setIndex(pieceRequest->index);
        pieceMsg->setBegin(pieceRequest->begin);
        pieceMsg->setBlockSize(reqLength);
        // piece has variable length, so must set the variablePayloadLen attribute
        pieceMsg->setVariablePayloadLen(reqLength);

        delete pieceRequest; // don't need the object anymore

        return pieceMsg;
    }
    void requestPieceMsg(int peerId, int index, int begin, int reqLength) {
        // store the request
        Request * pieceRequest = new Request();
        pieceRequest->begin = begin;
        pieceRequest->index = index;
        pieceRequest->reqLength = reqLength;
#ifdef DEBUG_MSG
        std::string out = "Request " + this->getRequestName(pieceRequest)
            + " made by peer " + toStr(peerId);
        this->contentManager->printDebugMsg(out);
#endif

        this->requestQueues[peerId].push(pieceRequest);
        // Fix the iterator, since the queue is definitely not empty anymore
        this->fixCurrentIt();
        this->tryToSend();
    }
    /*!
     * Cancel all blocks in the peer pending and send queues.
     */
    void cancelUploadRequests(int peerId) {
        std::ostringstream out;
        out << "Cancel requests ";
        int currentTokens = this->tokens;
        RequestQueueMapIt sendIt = this->sendQueues.find(peerId);
        if (sendIt != this->sendQueues.end()) {
            out << " Ready to send: ";

            // Erase the corresponding send queue
            RequestQueue & sendQueue = sendIt->second;
            while (!sendQueue.empty()) {
                Request * pieceRequest = sendQueue.front();
                this->addTokens(pieceRequest->reqLength);

                out << this->getRequestName(pieceRequest);

                delete pieceRequest;
                sendQueue.pop();
            }
            this->sendQueues.erase(sendIt);
        }
        // Erase the request queue
        RequestQueueMapIt reqIt = this->requestQueues.find(peerId);
        if (reqIt != this->requestQueues.end()) {
#ifdef DEBUG_MSG
            out << " In bucket: ";
#endif

            RequestQueue & requestQueue = reqIt->second;
            while (!requestQueue.empty()) {
                Request * pieceRequest = requestQueue.front();
#ifdef DEBUG_MSG
                out << this->getRequestName(pieceRequest);
#endif

                delete pieceRequest;
                requestQueue.pop();
            }

            // erase the request queue and keep the currentQueueIt valid
            if (this->currentQueueIt == reqIt) {
                this->requestQueues.erase(this->currentQueueIt++);
                this->fixCurrentIt();
            } else {
                this->requestQueues.erase(reqIt);
            }
        }
#ifdef DEBUG_MSG
        out << " (" << (this->tokens - currentTokens)
            << " tokens returned) for peer " << peerId;
        this->contentManager->printDebugMsg(out.str());
#endif
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
    std::string getRequestName(Request const* pieceRequest) {
        int pieceIndex = pieceRequest->index;
        int blockIndex = pieceRequest->begin / pieceRequest->reqLength;
        std::string out = "(" + toStr(pieceIndex) + "," + toStr(blockIndex)
            + ")";
        return out;
    }

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
        }
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
            unsigned long reqLength =
                static_cast<Request *>(currentQueueRef.front())->reqLength;
            hasEnoughTokens = reqLength <= this->tokens;
            if (hasEnoughTokens) {
                // Consume the tokens
                this->tokens -= reqLength;

                // Move the request from the requestQueue to the sendQueue
                int peerId = this->currentQueueIt->first;
                Request * pieceRequest = currentQueueRef.front();
                this->sendQueues[peerId].push(pieceRequest);
#ifdef DEBUG_MSG
                std::string out = "Request "
                    + this->getRequestName(pieceRequest)
                    + " ready to send to peer " + toStr(peerId);
                this->contentManager->printDebugMsg(out);
#endif

                currentQueueRef.pop();
                // If empty, erase the request queue and forward the current it
                if (currentQueueRef.empty()) {
                    this->requestQueues.erase(this->currentQueueIt++);
#ifdef DEBUG_MSG
                    std::string out = "No more requests pending for peer "
                        + toStr(peerId);
                    this->contentManager->printDebugMsg(out);
#endif
                } else {
                    ++this->currentQueueIt;
                }
                // Ensure the iterator remains valid after forwarding
                this->fixCurrentIt();

                // Warn the thread that there's a piece waiting
                this->contentManager->bitTorrentClient->sendPieceMessage(
                    this->contentManager->infoHash, peerId);

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
};

Define_Module(ContentManager);

ContentManager::ContentManager() :
    bitTorrentClient(NULL), tokenBucket(NULL), subPieceSize(0), debugFlag(
        false), haveBundleSize(0), numberOfSubPieces(0), numberOfPieces(0), requestBundleSize(
        0), totalBytesDownloaded(0), totalBytesUploaded(0), infoHash(-1), localPeerId(
        -1), firstMarkEmitted(false), secondMarkEmitted(false), thirdMarkEmitted(
        false) {
}
ContentManager::~ContentManager() {
    delete this->tokenBucket;
#ifdef DEBUG_MSG
    double completePerc = this->clientBitField.getCompletedPercentage();
    std::ostringstream out;
    out << "Completed " << completePerc << "% of the download";
    if (!this->clientBitField.full()) {
        out << ". Missing pieces - ";
        out << this->clientBitField.unavailablePieces();
    }

    this->printDebugMsg(out.str());
#endif
}

void ContentManager::addEmptyBitField(int peerId) {
    Enter_Method("addEmptyBitField(id: %d)", peerId);
    assert(!this->peerBitFields.count(peerId));

    // initializes all maps
    this->peerBitFields.insert(
        std::make_pair(peerId, BitField(this->clientBitField.size(), false)));
    this->numPendingRequests[peerId] = 0;
    this->totalDownloadedByPeer[peerId] = 0;
    this->totalUploadedByPeer[peerId] = 0;
}
void ContentManager::addPeerBitField(BitField const& bitField, int peerId) {
    Enter_Method("setPeerBitField(id: %d)", peerId);
    // If not empty, BitField already set
    assert(this->peerBitFields.at(peerId).empty());

    // add BitField to pieceCount. Throw error if BitField is invalid
    this->rarestPieceCounter.addBitField(bitField);

    // initializes all maps
    this->peerBitFields.at(peerId) = bitField; // reset the Peer BitField
    this->numPendingRequests[peerId] = 0;
    this->totalDownloadedByPeer[peerId] = 0;
    this->totalUploadedByPeer[peerId] = 0;

    if (this->isPeerInteresting(peerId)) {
        this->interestingPeers.insert(peerId);
        // peer interesting
        this->bitTorrentClient->peerInteresting(this->infoHash, peerId);
    } else if (bitField.full()) {
        // if this client is not interested in a seeder, that means it is
        // also a seeder and this connection has no use, so it is dropped.
        this->bitTorrentClient->closeConnection(this->infoHash, peerId);
    }
}
void ContentManager::removePeerBitField(int peerId) {
    Enter_Method("removePeerBitField(id: %d)", peerId);
    assert(this->peerBitFields.count(peerId));
#ifdef DEBUG_MSG
    std::ostringstream out;
    out << "removing peer " << peerId;
    this->printDebugMsg(out.str());
#endif

    // subtract BitField from pieceCount
    this->rarestPieceCounter.removeBitField(this->peerBitFields[peerId]);

    // remove from the interesting set
    this->interestingPeers.erase(peerId);
    // remove pieces requested by this peer
    this->requestedPieces.erase(peerId);

    // remove peerId from token bucket
    this->tokenBucket->cancelUploadRequests(peerId);

    // remove peerId from all maps
    this->peerBitFields.erase(peerId);
    this->numPendingRequests.erase(peerId);
    this->totalDownloadedByPeer.erase(peerId);
    this->totalUploadedByPeer.erase(peerId);

    this->updateStatusString();
}
void ContentManager::cancelDownloadRequests(int peerId) {
    Enter_Method("cancelDownloadRequests(index: %d)", peerId);
    // The peer must be registered here
    assert(this->peerBitFields.count(peerId));
#ifdef DEBUG_MSG
    std::ostringstream out;
    out << "Canceling requested pieces: ";
#endif

    std::multimap<int, int>::iterator begin, end;
    boost::tie(begin, end) = this->requestedPieces.equal_range(peerId);
    while (begin != end) {
        // Restore the requests that were not responded.
        this->missingBlocks.at(begin->second).resetRequests();
#ifdef DEBUG_MSG
        out << begin->second << " ";
#endif
        ++begin;
    }
    // Remove the requested pieces so others can request them
    this->requestedPieces.erase(peerId);
    // Clear the number of pending requests
    this->numPendingRequests.at(peerId) = 0;
#ifdef DEBUG_MSG
    this->printDebugMsg(out.str());
#endif
}
void ContentManager::cancelUploadRequests(int peerId) {
    Enter_Method("cancelUploadRequests(index: %d)", peerId);
    // The peer must be registered here
    assert(this->peerBitFields.count(peerId));

    this->tokenBucket->cancelUploadRequests(peerId);
}
bool ContentManager::isBitFieldEmpty() const {
    Enter_Method("isBitFieldEmpty()");
    return this->clientBitField.empty();
}
BitFieldMsg* ContentManager::getClientBitFieldMsg() const {
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
    // The peer must be registered here
    assert(this->peerBitFields.count(peerId));
    PeerWireMsgBundle* requestBundle = NULL;

    int numWantedRequests = this->requestBundleSize
        - this->numPendingRequests.at(peerId);
    // make new request only if there is space
    if (numWantedRequests > 0) {
        // The bundle to be filled with blocks
        cPacketQueue* bundle = new cPacketQueue();
        std::ostringstream name_o;
        name_o << "RequestBundle(";

        this->getRemainingPieces(bundle, name_o, peerId, numWantedRequests);
        assert(bundle->getLength() <= numWantedRequests);
        this->getInterestingPieces(bundle, name_o, peerId,
            numWantedRequests - bundle->getLength());
        assert(bundle->getLength() <= numWantedRequests);

        this->numPendingRequests.at(peerId) += bundle->getLength();

        if (!bundle->empty()) {
            name_o << ")";
            requestBundle = new PeerWireMsgBundle(name_o.str().c_str());
            requestBundle->setBundle(bundle);
        } else {
            delete bundle;
#ifdef DEBUG_MSG
            std::ostringstream out;
            out << "There are no blocks available for download";
            this->printDebugMsg(out.str());
#endif
        }
    } else {
#ifdef DEBUG_MSG
        std::ostringstream out;
        out << "There are blocks pending";
        this->printDebugMsg(out.str());
#endif
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
    // The piece must be available
    assert(this->clientBitField.hasPiece(index));

    this->tokenBucket->requestPieceMsg(peerId, index, begin, reqLength);
}

PieceMsg* ContentManager::getPieceMsg(int peerId) {
    Enter_Method("getPieceMsg(peerId: %d)", peerId);
    // The peer must be registered here
    assert(this->peerBitFields.count(peerId));

    return this->tokenBucket->getPieceMsg(peerId);
}

unsigned long ContentManager::getTotalDownloaded(int peerId) const {
    Enter_Method("getDownloadedBytes(peerId: %d)", peerId);
    // The peer must be registered here
    assert(this->peerBitFields.count(peerId));

    // error if the peerId is not in the totalDownloaded, meaning that the
    // BitField was not defined.
    return this->totalDownloadedByPeer.at(peerId);

}
unsigned long ContentManager::getTotalUploaded(int peerId) const {
    Enter_Method("getUploadedBytes(peerId: %d)", peerId);
    // The peer must be registered here
    assert(this->peerBitFields.count(peerId));

    // error if the peerId is not in the totalUploaded, meaning that the
    // BitField was not defined.
    return this->totalUploadedByPeer.at(peerId);
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

    // The peer must be registered here
    assert(this->peerBitFields.count(peerId));

    // update bytes counter
    this->totalDownloadedByPeer[peerId] += blockSize;
    this->totalBytesDownloaded += blockSize;
    emit(this->totalBytesDownloaded_Signal, this->totalBytesDownloaded);

    // ignore block if the piece is already complete
    if (!this->clientBitField.hasPiece(pieceIndex)) {
        // Try to find the piece in the missing blocks. If not found, create it
        typedef std::map<int, PieceBlocks>::iterator map_it;
        map_it lb = this->missingBlocks.lower_bound(pieceIndex);
        if (lb == this->missingBlocks.end() || lb->first != pieceIndex) {
            std::pair<int, PieceBlocks> const& p = std::make_pair(pieceIndex,
                PieceBlocks(pieceIndex, this->numberOfSubPieces));
            lb = this->missingBlocks.insert(lb, p);
        }

        // Set the piece and check if it was requested to this peerId. If so,
        // then decrement the number of pending requests
        PieceBlocks & req = lb->second;
        int blockIndex = begin / blockSize;
        if (req.setBlock(blockIndex)) {
            std::multimap<int, int>::iterator it, end;
            tie(it, end) = this->requestedPieces.equal_range(peerId);
            for (/* empty */; it != end; ++it) {
                if (it->second == pieceIndex) {
                    --this->numPendingRequests.at(peerId);
                    break;
                }
            }
        }

        // If the piece became complete, perform the needed cleanups
        if (req.isComplete()) {
#ifdef DEBUG_MSG
            {
                std::ostringstream out;
                out << "Downloaded piece " << pieceIndex;
                this->printDebugMsg(out.str());
            }
#endif

            // If this piece was not requested to peerId, it may have been
            // requested to a different peerId. That's why it is necessary to
            // go through the whole requestedPieces map
            std::multimap<int, int>::iterator it, end;
            it = this->requestedPieces.begin();
            end = this->requestedPieces.end();
            while (it != end) {
                std::multimap<int, int>::iterator currentIt = it++;
                if (currentIt->second == pieceIndex) {
                    this->requestedPieces.erase(currentIt);
                }
            }

            // First block fully downloaded, signaling the start of the download
            if (this->clientBitField.empty()) {
                this->downloadStartTime = simTime();
            }

            // "Move" the piece from the missingBlocks to the clientBitField
            this->missingBlocks.erase(pieceIndex);
            this->clientBitField.addPiece(pieceIndex);

            // Check if the connected peers continue to be interesting
            this->verifyInterestOnAllPeers();
            // Statistics
            generateDownloadStatistics(pieceIndex);
            // schedule the sending of the HaveMsg to all Peers.
            this->bitTorrentClient->sendHaveMessages(this->infoHash,
                pieceIndex);

            if (this->clientBitField.full()) { // became seeder
#ifdef DEBUG_MSG
                this->printDebugMsg("Became a seeder");
#endif
                // warn the tracker
                this->bitTorrentClient->finishedDownload(this->infoHash);
            }

            this->updateStatusString();
        }
    }
}
void ContentManager::processHaveMsg(int pieceIndex, int peerId) {
    Enter_Method("updatePeerBitField(index: %d, id: %d)", pieceIndex, peerId);

    if (pieceIndex < 0 || pieceIndex >= this->numberOfPieces) {
        throw std::out_of_range("The piece index is out of bounds");
    }

    // The peer must be registered here
    assert(this->peerBitFields.count(peerId));

    this->rarestPieceCounter.addPiece(pieceIndex);
    // add the piece to the client's BitField
    BitField & peerBitField = this->peerBitFields.at(peerId);
    peerBitField.addPiece(pieceIndex);

    if (!this->interestingPeers.count(peerId) && // Peer was not interesting
        this->isPeerInteresting(peerId)) { // but now is
        this->interestingPeers.insert(peerId);
        this->bitTorrentClient->peerInteresting(this->infoHash, peerId);
    }

    // Last piece resulted in the Peer becoming a seeder
    if (peerBitField.full()) {
#ifdef DEBUG_MSG
        std::string out;
        std::string peerIdStr = toStr(peerId);
#endif
        if (this->clientBitField.full()) {
#ifdef DEBUG_MSG
            out = "Both the Client and Peer " + peerIdStr + " are seeders.";
#endif
            // the connection between two seeders is useless. Drop it.
            this->bitTorrentClient->closeConnection(this->infoHash, peerId);
#ifdef DEBUG_MSG
        } else {
            out = "Peer " + peerIdStr + " became a seeder.";
#endif
        }
#ifdef DEBUG_MSG
        this->printDebugMsg(out);
#endif
    }
}

// private methods
void ContentManager::insertRequestInBundle(cPacketQueue * const bundle,
    std::ostringstream & bundleMsgName, int pieceIndex, int blockIndex) {
    assert(bundle->length() < this->requestBundleSize);
    // create the request and insert it in the bundle
    RequestMsg *request;
    std::ostringstream name;
    name << "RequestMsg(" << pieceIndex << "," << blockIndex << ")";
    request = new RequestMsg(name.str().c_str());
    request->setIndex(pieceIndex);
    request->setBegin(blockIndex * this->subPieceSize);
    request->setReqLength(this->subPieceSize);
    // set the size of the request
    request->setByteLength(request->getHeaderLen() + request->getPayloadLen());

    bundleMsgName << "(" << pieceIndex << "," << blockIndex << ")";
    bundle->insert(request);
}
void ContentManager::getRemainingPieces(cPacketQueue * const bundle,
    std::ostringstream & bundleMsgName, int peerId, int requestBundleSize) {
    assert(bundle->getLength() == 0); // Bundle starts empty
    // get blocks from pieces previously requested by this peerId
    std::multimap<int, int>::iterator it;
    std::multimap<int, int>::const_iterator end;
    tie(it, end) = this->requestedPieces.equal_range(peerId);
    typedef std::pair<int, int> pair_t;
    while (it != end) {
        PieceBlocks & req = this->missingBlocks.at(it->second);
        std::list<pair_t> & blocks = req.getRequests();
        while (!blocks.empty()) {
            pair_t block = blocks.front();
            this->insertRequestInBundle(bundle, bundleMsgName, block.first,
                block.second);
            blocks.pop_front();
            // exit the loop because the bundle is full
            if (bundle->getLength() == requestBundleSize) break;
        }

        // exit the loop because the bundle is full
        if (bundle->getLength() == requestBundleSize) break;
        ++it;
    }
}
void ContentManager::getInterestingPieces(cPacketQueue * const bundle,
    std::ostringstream & bundleMsgName, int peerId, int requestBundleSize) {

    // Don't do anything if there's no space in the bundle
    if (requestBundleSize == 0) {
        return;
    }

    // Ignore all pieces previously requested (including the ones requested by
    // this peerId because they were already added to the bundle).
    BitField allRequestBitField = this->clientBitField; // get a copy
    typedef std::pair<int, int> pair_t;
    BOOST_FOREACH(pair_t p, this->requestedPieces) {
        allRequestBitField.addPiece(p.second);
    }

    std::set<int> const& intPieces = allRequestBitField.getInterestingPieces(
        this->peerBitFields.at(peerId));

    if (!intPieces.empty()) {
        // get the rarest pieces among the interesting ones.
        std::vector<int> const& rarest =
            this->rarestPieceCounter.getRarestPieces(intPieces, numberOfPieces);
        BOOST_FOREACH (int pieceIndex, rarest) {
            // Return the incomplete piece, or create a new one
            // similar to operator[], but without the default constructor
            // http://cplusplus.com/reference/stl/map/operator[]/

            // Try to find the piece in the missing blocks. If not found, create it
            typedef std::map<int, PieceBlocks>::iterator map_it;
            map_it lb = this->missingBlocks.lower_bound(pieceIndex);
            if (lb == this->missingBlocks.end() || lb->first != pieceIndex) {
                std::pair<int, PieceBlocks> const& p = std::make_pair(
                    pieceIndex,
                    PieceBlocks(pieceIndex, this->numberOfSubPieces));
                lb = this->missingBlocks.insert(lb, p);
            }
            PieceBlocks & req = lb->second;

            // Add the piece to the requested map
            this->requestedPieces.insert(std::make_pair(peerId, pieceIndex));
            // Save the time this request was made
            this->pieceRequestTime[pieceIndex];

            // use the blocks from the request object
            typedef std::pair<int, int> pair_t;
            std::list<pair_t> & blocks = req.getRequests();
            while (!blocks.empty()) {
                pair_t block = blocks.front();
                this->insertRequestInBundle(bundle, bundleMsgName, block.first,
                    block.second);
                blocks.pop_front();
                // exit the loop because the bundle is full
                if (bundle->getLength() == requestBundleSize) break;
            }

            // exit the loop because the bundle is full
            if (bundle->getLength() == requestBundleSize) break;
        }
    }
}
bool ContentManager::isPeerInteresting(int peerId) const {
    assert(this->peerBitFields.count(peerId));
    // Ignore the pieces requested by other peers, but not the ones this peer
    // has requested.
    BitField clientRequestBitField = this->clientBitField; // get a copy
    typedef std::pair<int, int> pair_t;
    BOOST_FOREACH(pair_t p, this->requestedPieces) {
        if (p.first != peerId) clientRequestBitField.addPiece(p.second);
    }
    BitField const& peerBitField = this->peerBitFields.at(peerId);
    return clientRequestBitField.isBitFieldInteresting(peerBitField);

}
void ContentManager::generateDownloadStatistics(int pieceIndex) {
    assert(this->pieceRequestTime.count(pieceIndex));
    // Gather statistics in the completion time
    emit(this->pieceDownloadTime_Signal,
        simTime() - this->pieceRequestTime.at(pieceIndex));
    emit(this->pieceDownloaded_Signal, pieceIndex);
    // request time already used, so delete it
    this->pieceRequestTime.erase(pieceIndex);

    simsignal_t markTime_Signal;
    bool emitCompletionSignal = false;

    // download statistics
    // when one of the download marks is reached, send the corresponding signal
    if (!this->firstMarkEmitted
        && this->clientBitField.getCompletedPercentage() >= 25.0) {
        this->firstMarkEmitted = emitCompletionSignal = true;
        markTime_Signal = this->_25_percentDownloadTime_Signal;
    } else if (!this->secondMarkEmitted
        && this->clientBitField.getCompletedPercentage() >= 50.0) {
        this->secondMarkEmitted = emitCompletionSignal = true;
        markTime_Signal = this->_50_percentDownloadTime_Signal;
    } else if (!this->thirdMarkEmitted
        && this->clientBitField.getCompletedPercentage() >= 75.0) {
        this->thirdMarkEmitted = emitCompletionSignal = true;
        markTime_Signal = this->_75_percentDownloadTime_Signal;
    } else if (this->clientBitField.full()) {
        emitCompletionSignal = true;
        markTime_Signal = this->_100_percentDownloadTime_Signal;

        // send signal warning this ContentManager became a seeder
        emit(this->becameSeeder_Signal, this->infoHash);
    }

    if (emitCompletionSignal) {
        simtime_t downloadInterval = simTime() - this->downloadStartTime;
#ifdef DEBUG_MSG
        std::ostringstream out;
        out << "Start: " << this->downloadStartTime;
        out << "Now: " << simTime();
        out << "Interval: " << downloadInterval;

        this->printDebugMsg(out.str());
#endif

        emit(this->emittedPeerId_Signal, this->localPeerId);
        emit(markTime_Signal, downloadInterval);
    }
}
void ContentManager::verifyInterestOnAllPeers() {
    std::set<int>::iterator it = this->interestingPeers.begin();
    while (it != this->interestingPeers.end()) {
        // Copy then increment. Delete using this iterator don't invalidate 'it'.
        std::set<int>::iterator currentPeerIt = it++;
        int peerId = *currentPeerIt;

        BitField const& peerBitField = this->peerBitFields.at(peerId);
        // Drop this connection because it is no longer useful.
        if (peerBitField.full() && this->clientBitField.full()) {
#ifdef DEBUG_MSG
            std::string out = "Both the Client and Peer " + toStr(peerId)
                + " are seeders.";
            this->printDebugMsg(out);
#endif
            this->bitTorrentClient->closeConnection(this->infoHash, peerId);
        } else if (!this->isPeerInteresting(peerId)) {
            // not interesting anymore
            this->interestingPeers.erase(currentPeerIt);
            this->bitTorrentClient->peerNotInteresting(this->infoHash, peerId);
        }
    }
}
// signal methods
void ContentManager::registerEmittedSignals() {
// Configure signals
#define SIGNAL(X, Y) this->X = registerSignal("ContentManager_" #Y)
    SIGNAL(becameSeeder_Signal, BecameSeeder);
    SIGNAL(pieceDownloadTime_Signal, PieceDownloadTime);
    SIGNAL(totalBytesDownloaded_Signal, TotalBytesDownloaded);

    SIGNAL(totalBytesUploaded_Signal, TotalBytesUploaded);
    SIGNAL(_25_percentDownloadTime_Signal, 25_percentDownloadTime);
    SIGNAL(_50_percentDownloadTime_Signal, 50_percentDownloadTime);
    SIGNAL(_75_percentDownloadTime_Signal, 75_percentDownloadTime);
    SIGNAL(_100_percentDownloadTime_Signal, 100_percentDownloadTime);

    SIGNAL(emittedPeerId_Signal, EmittedPeerId);
    SIGNAL(pieceDownloaded_Signal, PieceDownloaded);
#undef SIGNAL
}
// module methods
void ContentManager::printDebugMsg(std::string s) {
#ifdef DEBUG_MSG
    if (this->debugFlag) {
        // debug "header"
        std::cerr << simulation.getEventNumber();
        std::cerr << ";" << simulation.getSimTime();
        std::cerr << ";(cmanager);Peer " << this->localPeerId;
        std::cerr << ";infoHash " << this->infoHash << ";";
        std::cerr << s << "\n";
        std::cerr.flush();
    }
#endif
}
void ContentManager::updateStatusString() {
    if (ev.isGUI()) {
        std::ostringstream out;
        if (this->clientBitField.full()) {
            out << "Seeder";
        } else {
            out.precision(2);
            out.setf(std::ios::fixed);
            out << this->clientBitField.getCompletedPercentage() << "%";
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
    this->bitTorrentClient = check_and_cast<BitTorrentClient*>(
        getParentModule());

    this->localPeerId = this->bitTorrentClient->getLocalPeerId();

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
