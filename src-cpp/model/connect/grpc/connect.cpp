/*
 * Flex Flow
 * Copyright (c) 2026-present fdar0536
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

#include <memory>
#include "spdlog/spdlog.h"

#include "model/utils.hpp"
#include "model/dao/grpc/utils.hpp"
#include "access.pb.h"

#include "connect.hpp"

namespace Model
{

namespace Connect
{

namespace GRPC
{

std::shared_ptr<Token> connect(
    const std::string &target,
    const u16 port,
    const std::string username,
    const std::string password,
    const std::string otp)
{
    spdlog::debug("{}:{} Model::Connect::GRPC::connect",
        LOG_FILE_PATH(__FILE__), __LINE__);

    auto channel = std::shared_ptr<grpc::ChannelInterface>();

    std::string ip = target;
    ip += ":";
    ip += std::to_string(port);
    std::unique_ptr<ff::Access::Stub> stub;

    Token *token = new (std::nothrow) Token;
    if (!token)
    {
        spdlog::error("{}:{} Fail to allocate memory",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return nullptr;
    }

    auto ret = std::shared_ptr<Token>(token);

    try
    {
        // connect to server
        channel = grpc::CreateChannel(ip, 
            grpc::InsecureChannelCredentials());

        if (channel == nullptr)
        {
            spdlog::error("{}:{} Fail to create channel",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return nullptr;
        }

        // get "access" node
        stub = ff::Access::NewStub(channel);
        if (stub == nullptr)
        {
            spdlog::error("{}:{} Fail to create access' stub",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return nullptr;
        }

        // login target server
        ff::LoginReq req;
        req.set_username(username);
        req.set_password(password);
        req.set_otp(otp);

        ff::LoginRes res;
        grpc::ClientContext ctx;
        DAO::GRPC::Utils::setupCtx(ctx);
        grpc::Status status = stub->Login(&ctx, req, &res);

        if (status.ok())
        {
            ret->token = res.token();
            ret->channel = channel;
        }
        else
        {
            DAO::GRPC::Utils::buildErrMsg(
                LOG_FILE_PATH(__FILE__), __LINE__, status);
            return nullptr;
        }
    }
    catch (...)
    {
        spdlog::error("{}:{} Fail to initialize connection",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return nullptr;
    }

    return ret;
}

} // end namespace GRPC

} // end namespace Connect

} // end namespace Model
