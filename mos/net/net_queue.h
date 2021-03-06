/*
 * Copyright (c) 2008-2012 the MansOS team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of  conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MANSOS_NET_QUEUE_H
#define MANSOS_NET_QUEUE_H

#include "mac.h"
#include <lib/list.h>

typedef struct QueuedPacket_s {
    STAILQ_ENTRY(QueuedPacket_s) chain;
    bool isUsed;
    uint8_t sendTries; // how many times already tried to send
    uint32_t ackTime;  // await ACK until this time
    uint8_t data[MAC_PROTOCOL_BUFFER_SIZE];
    uint16_t dataLength;
} QueuedPacket_t;

typedef STAILQ_HEAD(head, QueuedPacket_s) PacketQueue_t;
extern PacketQueue_t packetQueue;

// ---------------------------------------------

void netQueueInit(void);

//  add new packet to userQueue tail. returns error code. locks mutex.
int8_t netQueueAddPacket(MacInfo_t *, const uint8_t *data, uint16_t length,
                      QueuedPacket_t *result);
// frees the userQueue head packet. locks mutex. 
void netQueuePop(void);
// mutex is not locked
static inline QueuedPacket_t *queueHead(void) {
    return STAILQ_FIRST(&packetQueue);
}

// work queue processing. mutex is not locked.
typedef void (*QpacketProcessFn)(QueuedPacket_t *);
typedef bool (*QpacketMatchFn)(QueuedPacket_t *, void *userData);

void netQueueForEachPacket(QpacketProcessFn);
QueuedPacket_t *netQueueGetPacket(QpacketMatchFn, void *userData);
QueuedPacket_t *netQueueRemovePacket(QpacketMatchFn, void *userData);

void queueFreePacket(QueuedPacket_t *);

#endif
