/*
 * Flex Flow
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

#ifndef _MODEL_DAO_GRPC_CONNECT_HPP_
#define _MODEL_DAO_GRPC_CONNECT_HPP_

#include "access.grpc.pb.h"

#include "model/dao/iconnect.hpp"

namespace Model
{

namespace DAO
{

namespace GRPC
{

class Token
{
public:

    Token();

    ~Token();

    std::shared_ptr<grpc::ChannelInterface> channel;
};

class Connect : public IConnect
{

public:

    Connect();

    ~Connect();

    u8 init() override;

    u8 startConnect(const std::string &target,
                    const i32 port = 0) override;

}; // end class Connect

} // end namespace GRPC

} // end namespace DAO

} // end namespace Model

#endif // _MODEL_DAO_GRPC_CONNECT_HPP_
