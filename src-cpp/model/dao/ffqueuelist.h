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

#include "controller/global/defines.h"

#ifndef _MODEL_DAO_FFQUEUELIST_H_
#define _MODEL_DAO_FFQUEUELIST_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct FFQueueList
{
    u8 (*init)(Handle connect, Handle *out);

    u8 (*destroy)(Handle h);

    u8 (*createQueue)(Handle h, const char *name);

    u8 (*listQueue)(Handle h, char ***out, size_t *outSize);

    u8 (*deleteQueue)(Handle h, const char *name);

    u8 (*renameQueue)(Handle h, const char *oldName,
                             const char *newName);

    u8 (*getQueue)(Handle h, const char *name, Handle *out);

    u8 (*returnQueue)(Handle h, Handle queue);

} FFQueueList;

#ifdef __cplusplus
}
#endif

#endif // _MODEL_DAO_FFQUEUELIST_H_
