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

Define_Module(RateLimiter);

RateLimiter::RateLimiter() :
        messageQueue("Unsent messages"), tokenIncTimerMsg("Add Tokens"),
            bucketSize(0), bytesSec(0), tokenSize(0), tokens(0) {
}
RateLimiter::~RateLimiter() {
    cancelEvent(&this->tokenIncTimerMsg);
}

bool RateLimiter::tryToSend() {
    cPacket * packet = dynamic_cast<cPacket *>(this->messageQueue.front());

    // send if cMessage or if cPacket and there are enough tokens to consume
    bool sendMsg = !packet || (packet->getByteLength() <= this->tokens);
    if (sendMsg) {
        if (packet) {
            this->tokens -= packet->getByteLength();
        }
        cMessage * msg = static_cast<cMessage *>(this->messageQueue.pop());
        send(msg, "out");
    }

    // If a message was sent, unpause the increment timer (if paused)
    if (sendMsg && !this->tokenIncTimerMsg.isScheduled()) {
        scheduleAt(simTime() + this->incInterval, &this->tokenIncTimerMsg);
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
    scheduleAt(simTime() + this->incInterval, &this->tokenIncTimerMsg);
}

void RateLimiter::handleMessage(cMessage *msg) {
    if (msg == &this->tokenIncTimerMsg) {
        this->tokens = this->tokens + this->tokenSize;

        // cap the number of tokens
        if (this->tokens > this->bucketSize) {
            // Pause the increment timer when the token bucket is full,
            // avoiding useless increment calls
            this->tokens = this->bucketSize;
        } else {
            scheduleAt(simTime() + this->incInterval, &this->tokenIncTimerMsg);
        }

        // send messages from the queue until there are not enough tokens or no
        // more messages to send
        bool messageSent = true;
        while (!this->messageQueue.empty() && messageSent) {
            messageSent = this->tryToSend();
        }

    } else if (msg->arrivedOn("in")) {
        this->messageQueue.insert(msg);
        this->tryToSend();
    }
}
