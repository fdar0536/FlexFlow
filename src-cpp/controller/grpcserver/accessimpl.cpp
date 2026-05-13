/*
 * Flex Flow
 * Copyright (c) 2023-present fdar0536
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

#include <string>

#include "spdlog/spdlog.h"

#include "model/utils.hpp"

#include "config.h"

#include "init.hpp"
#include "utils.hpp"

#include "accessimpl.hpp"

namespace Controller
{

namespace GRPCServer
{

grpc::Status AccessImpl::Info(grpc::ServerContext *ctx,
                              const ff::Empty *req,
                              ff::InfoRes *res)
{
    spdlog::debug("{}:{} AccessImpl::Info", LOG_FILE_PATH(__FILE__), __LINE__);
    UNUSED(ctx);
    UNUSED(req);
    res->set_branch(FF_BRANCH);
    res->set_commit(FF_COMMIT);
    res->set_version(FF_VERSION);
    return grpc::Status::OK;
}

grpc::Status AccessImpl::Login(grpc::ServerContext *ctx,
                               const ff::LoginReq *req,
                               ff::LoginRes *res)
{
    spdlog::debug("{}:{} AccessImpl::Login", LOG_FILE_PATH(__FILE__), __LINE__);
    
    if (!ctx || !req || !res)
    {
        spdlog::error("{}:{} invalid input", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    std::string token;

    if (auth->login(req->username(),
    req->password(), req->otp(), token))
    {
        spdlog::error("{}:{} Fail to login", LOG_FILE_PATH(__FILE__), __LINE__);
        std::string ip = Utils::getIPFromContext(ctx);
        auth->addBannedIp(ip);
        return grpc::Status(grpc::StatusCode::UNAUTHENTICATED,
            "Fail to login");
    }

    res->set_token(token);
    return grpc::Status::OK;
}
    
grpc::Status AccessImpl::Logout(grpc::ServerContext *ctx,
                                const ff::LogoutReq *req,
                                ff::Empty *res)
{
    if (!ctx || !req || !res)
    {
        spdlog::error("{}:{} invalid input", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    auto it = ctx->client_metadata().find("x-auth-token");
    std::string token = std::string(it->second.data(), it->second.length());
    if (auth->logout(req->username(), token))
    {
        spdlog::error("{}:{} Fail to logout", LOG_FILE_PATH(__FILE__), __LINE__);
        std::string ip = Utils::getIPFromContext(ctx);
        auth->addBannedIp(ip);
        return grpc::Status(grpc::StatusCode::UNAUTHENTICATED,
            "Fail to logout");
    }

    return grpc::Status::OK;
}

} // end namespace GRPCServer

} // end namespace Controller
