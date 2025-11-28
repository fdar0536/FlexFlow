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

#ifndef _MODEL_DAO_CONNECT_H_
#define _MODEL_DAO_CONNECT_H_

#include "handle.h"

#ifdef __cplusplus
extern "C"
{
#endif

FF_MODEL_API u8 connect_init(Handle *out, u8 backend);

FF_MODEL_API u8 connect_destroy(Handle h);

FF_MODEL_API u8 connect_startConnect(Handle h, const char *target, const i32 port);

FF_MODEL_API void *connect_onnectToken(Handle h);

FF_MODEL_API u8 connect_targetPath(Handle h, char *buf, size_t *bufSize);

#ifdef __cplusplus
}
#endif

#endif // _MODEL_DAO_CONNECT_H_
