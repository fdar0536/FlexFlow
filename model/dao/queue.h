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

#ifndef _MODEL_DAO_QUEUE_H_
#define _MODEL_DAO_QUEUE_H_

#include "handle.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct ProcTask
{
    char *execName;
    size_t argc;
    char **argv;
    char *workDir;
    i32 ID;
    i32 exitCode;
    u8 isSuccess;
} ProcTask;

// note that this function only destroy the content of ProcTask
// does not free ProcTask
void queue_destroyProcTask(ProcTask *);

u8 queue_listPending(Handle h, int **out, size_t *outSize);

u8 queue_listFinished(Handle h, int **out, size_t *outSize);

u8 queue_pendingDetails(Handle h, const int id, ProcTask *out);

u8 queue_finishedDetails(Handle h, const int id, ProcTask *out);

u8 queue_clearPending(Handle h);

u8 queue_clearFinished(Handle h);

u8 queue_currentTask(Handle h, ProcTask *out);

u8 queue_addTask(Handle h, const ProcTask *in);

u8 queue_removeTask(Handle h, const i32 in);

u8 queue_isRunning(Handle h);

u8 queue_readCurrentOutput(Handle h, char ***out, size_t *outSize);

u8 queue_start(Handle h);

u8 queue_stop(Handle h);

#ifdef __cplusplus
}
#endif

#endif // _MODEL_DAO_QUEUE_H_
