/*
 * Simple Task Queue
 * Copyright (c) 2023-2024 fdar0536
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

#ifndef _MODEL_ERRMSG_HPP_
#define _MODEL_ERRMSG_HPP_


#include <string>

#include "config.h"

#include "grpcpp/grpcpp.h"

#include "controller/global/defines.h"

#define ErrCode_OK               0
#define ErrCode_INVALID_ARGUMENT 1
#define ErrCode_NOT_FOUND        2
#define ErrCode_ALREADY_EXISTS   3
#define ErrCode_OUT_OF_RANGE     4
#define ErrCode_OS_ERROR         5

namespace Model
{

namespace ErrMsg
{

FF_MODEL_API void init();

FF_MODEL_API grpc::Status toGRPCStatus(u8, const std::string &);

} // end namespace ErrMsg

} // end namespace Model

#endif // _MODEL_ERRMSG_HPP_
