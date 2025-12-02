/*
 * Flex Flow
 * Copyright (c) 2025-present fdar0536
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "spdlog/spdlog.h"

#include "connect.h"
#include "queuelist.h"
#include "queue.h"

#include "capi.h"

static void buildConnect(FFConnect &c)
{
    c.init = connect_init;
    c.destroy = connect_destroy;
    c.startConnect = connect_startConnect;
    c.connectToken = connect_connectToken;
    c.targetPath = connect_targetPath;
}

static void buildQueueList(FFQueueList &l)
{
    l.init = queuelist_init;
    l.destroy = queuelist_destroy;
    l.createQueue = queuelist_createQueue;
    l.listQueue = queuelist_listQueue;
    l.deleteQueue = queuelist_deleteQueue;
    l.renameQueue = queuelist_renameQueue;
    l.getQueue = queuelist_getQueue;
    l.returnQueue = queuelist_returnQueue;
}

static void buildQueue(FFQueue &q)
{
    q.destroyProcTask = queue_destroyProcTask;
    q.listPending = queue_listPending;
    q.listFinished = queue_listFinished;
    q.pendingDetails = queue_pendingDetails;
    q.finishedDetails = queue_finishedDetails;
    q.clearPending = queue_clearPending;
    q.clearFinished = queue_clearFinished;
    q.currentTask = queue_currentTask;
    q.addTask = queue_addTask;
    q.removeTask = queue_removeTask;
    q.isRunning = queue_isRunning;
    q.readCurrentOutput = queue_readCurrentOutput;
    q.start = queue_start;
    q.stop = queue_stop;
}

extern "C"
{

FF_MODEL_API u8 getFFModel(FFModel *in)
{
    if (!in)
    {
        spdlog::error("{}:{} invalid input", __FILE__, __LINE__);
        return 1;
    }

    buildConnect(in->Connect);
    buildQueueList(in->QueueList);
    buildQueue(in->Queue);

    return 0;
}

}
