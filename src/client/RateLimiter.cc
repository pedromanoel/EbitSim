//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "RateLimiter.h"
#include "TCPCommand_m.h"
#include "PeerWire_m.h"

Define_Module(RateLimiter);

RateLimiter::RateLimiter() :
        tokenIncTimerMsg("Add Tokens"), bucketSize(0), bytesSec(0),
            tokenSize(0), tokens(0) {
    this->currentQueue = this->messageQueues.end();
}
RateLimiter::~RateLimiter() {
    cancelEvent(&this->tokenIncTimerMsg);
}

bool RateLimiter::tryToSend(PieceMsg * msg) {
    int connId = static_cast<TCPCommand *>(msg->getControlInfo())->getConnId();

    bool enoughTokens = msg->getByteLength() <= this->tokens;
    bool msgSent = false;

    // There aren't enough tokens OR there are enough tokens, but there are
    // other messages in the queue
    if (!enoughTokens || this->messageQueues.size() > 0) {
        this->messageQueues[connId].insert(msg);

        // if there weren't any queues, fix the iterator
        if (this->currentQueue == this->messageQueues.end()) {
            this->currentQueue = this->messageQueues.begin();
        }
    } else {
        // the queue is empty and there are enough tokens to send the message,
        // so no need to create a queue for a message that will be sent
        this->tokens -= msg->getByteLength();
        msgSent = true;
        send(msg, "out");
    }
    return msgSent;
}

bool RateLimiter::tryToSend() {
    bool sendMsg = false;

    if (!this->messageQueues.empty()) {
        cQueue & current = this->currentQueue->second;
        PieceMsg * msg = dynamic_cast<PieceMsg *>(current.front());

        // If there are enough tokens, send a message from the current queue and
        // go to the next queue
        sendMsg = msg->getByteLength() <= this->tokens;
        if (sendMsg) {
            // consume tokens, pop the message from the queue and send it
            this->tokens -= msg->getByteLength();
            current.pop();
            send(msg, "out");

            // erase the queue if empty (the iterator is still valid) and forward
            // the current iterator
            if (current.empty()) {
                this->messageQueues.erase(this->currentQueue++);
            } else {
                ++this->currentQueue;
            }

            // keep the iterator valid
            if (this->currentQueue == this->messageQueues.end()) {
                this->currentQueue = this->messageQueues.begin();
            }
            // Un-pause the increment timer (if paused)
            if (!this->tokenIncTimerMsg.isScheduled()) {
                scheduleAt(simTime() + this->incInterval,
                    &this->tokenIncTimerMsg);
            }
        }
    }
    return sendMsg;
}

void RateLimiter::initialize() {
    this->bucketSize = par("bucketSize").longValue();
    this->bytesSec = par("bytesSec").longValue();
    this->tokenSize = par("tokenSize").longValue();

    if (this->bytesSec < this->tokenSize) {
        throw cException("The rate must be bigger than the increment");
    }

    this->incInterval = ((double) this->tokenSize / this->bytesSec);

    WATCH(tokens);
    WATCH_MAP(messageQueues);
    scheduleAt(simTime() + this->incInterval, &this->tokenIncTimerMsg);
}

void RateLimiter::handleMessage(cMessage *msg) {
    if (msg == &this->tokenIncTimerMsg) {
        this->tokens += this->tokenSize;

        // cap the number of tokens
        if (this->tokens > this->bucketSize) {
            this->tokens = this->bucketSize;
        } else {
            // Pause the increment timer when the token bucket is full,
            // avoiding useless increment calls
            scheduleAt(simTime() + this->incInterval, &this->tokenIncTimerMsg);
        }

        while (this->tryToSend()) {
            // try to send as much messages as possible
        }
    } else if (msg->arrivedOn("in")) {
        // queue only piece messages
        PieceMsg * pieceMsg = dynamic_cast<PieceMsg *>(msg);
        if (pieceMsg) {
            this->tryToSend(pieceMsg);
        } else {
            send(msg, "out");
        }
    }
}
