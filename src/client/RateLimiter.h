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

#ifndef __EBITSIM_RATELIMITER_H_
#define __EBITSIM_RATELIMITER_H_

#include <omnetpp.h>

/**
 * TODO - Generated class
 */
class RateLimiter: public cSimpleModule {
public:
    RateLimiter();
    ~RateLimiter();
private:
    /*!
     * Send the message if there are enough tokens in the bucket.
     * Return true if the message was sent.
     */
    bool tryToSend();
private:
    long bucketSize;
    long bytesSec;
    long tokenSize;
    long tokens; // Implements a token bucket
    simtime_t incInterval;
    cQueue messageQueue; // Token bucket queue
    cMessage tokenIncTimerMsg; // self message that add tokens
protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

#endif
