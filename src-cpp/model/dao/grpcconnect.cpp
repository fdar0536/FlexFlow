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

#include <new>

#include "spdlog/spdlog.h"

#include "model/errmsg.hpp"

#include "grpcconnect.hpp"
#include "grpcutils.hpp"

namespace Model
{

namespace DAO
{

GRPCToken::GRPCToken() :
    channel(nullptr)
{}

GRPCToken::~GRPCToken()
{}

GRPCConnect::GRPCConnect()
{}

GRPCConnect::~GRPCConnect()
{
    freeConnectToken<GRPCToken>();
}

u8 GRPCConnect::init()
{
    return ErrCode_OK;
}

u8 GRPCConnect::startConnect(const std::string &target,
                             const i32 port)
{
    GRPCToken *token = new (std::nothrow) GRPCToken;
    if (!token)
    {
        spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
        return ErrCode_OS_ERROR;
    }

    std::string ip = target;
    ip += ":";
    ip += std::to_string(port);
    std::unique_ptr<ff::Access::Stub> stub;

    try
    {
        token->channel = grpc::CreateChannel(ip,
                                             grpc::InsecureChannelCredentials());

        if (token->channel == nullptr)
        {
            spdlog::error("{}:{} Fail to create channel", __FILE__, __LINE__);
            delete token;
            return ErrCode_OS_ERROR;
        }

        stub = ff::Access::NewStub(token->channel);
        if (stub == nullptr)
        {
            spdlog::error("{}:{} Fail to create access' stub", __FILE__, __LINE__);
            delete token;
            return ErrCode_OS_ERROR;
        }
    }
    catch (...)
    {
        spdlog::error("{}:{} Fail to initialize connection", __FILE__, __LINE__);
        delete token;
        return ErrCode_OS_ERROR;
    }

    ff::Empty req;
    ff::EchoRes res;
    grpc::ClientContext ctx;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = stub->Echo(&ctx, req, &res);
    if (status.ok())
    {
        m_connectToken = reinterpret_cast<void *>(token);
        return ErrCode_OK;
    }

    delete token;
    m_connectToken = nullptr;
    GRPCUtils::buildErrMsg(__FILE__, __LINE__, status);
    return ErrCode_OS_ERROR;
}

} // end namespace DAO

} // end namespace Model
