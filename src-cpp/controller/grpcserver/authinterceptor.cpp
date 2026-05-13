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

#include "spdlog/spdlog.h"

#include "model/utils.hpp"

#include "init.hpp"
#include "utils.hpp"

#include "authinterceptor.hpp"

namespace Controller
{

namespace GRPCServer
{

// api name = "/package.ServiceName/MethodName"
#define SHOULD_NOT_CHECK_TOKEN(x) \
    (x == "/ff.Access/Login")

void AuthInterceptor::Intercept(grpc::experimental::InterceptorBatchMethods *methods)
{
    spdlog::debug("{}:{} Controller::GRPCServer::AuthInterceptor::Intercept",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    if (methods->QueryInterceptionHookPoint(
            grpc::experimental::InterceptionHookPoints::PRE_RECV_INITIAL_METADATA))
    {
        // 1. get api's name
        std::string methodName = m_info->method();

        // 2. get client ip
        auto ctx = m_info->server_context();
        if (!ctx)
        {
            spdlog::error("{}:{} invalid input",
                LOG_FILE_PATH(__FILE__), __LINE__);
            methods->ModifySendStatus(
                grpc::Status(grpc::StatusCode::INTERNAL,
                    "Internal server error"));
            return;
        }

        std::string clientIp = Utils::getIPFromContext(ctx);
        if (SHOULD_NOT_CHECK_TOKEN(methodName))
        {
            if (auth->cannotAccess(clientIp))
            {
                spdlog::error("{}:{} cannot access",
                    LOG_FILE_PATH(__FILE__), __LINE__);
                goto error;
            }
        }
        else
        {
            std::string token;
            if (Utils::getTokenFromContext(ctx, token))
            {
                spdlog::error("{}:{} Fail to get token",
                    LOG_FILE_PATH(__FILE__), __LINE__);
                
                auth->addBannedIp(clientIp);
                goto error;
            }

            if (auth->cannotAccess(clientIp, token))
            {
                spdlog::error("{}:{} cannot access",
                    LOG_FILE_PATH(__FILE__), __LINE__);
                goto error;
            }
        } // if (SHOULD_NOT_CHECK_TOKEN(methodName))
    } // if (methods->QueryInterceptionHookPoint)

    methods->Proceed();
    return;

error:
    methods->ModifySendStatus(grpc::Status(grpc::StatusCode::UNAUTHENTICATED,
                "Access Denied by Security Policy"));
} // void AuthInterceptor::Intercept(grpc::experimental::InterceptorBatchMethods *methods)

} // end namespace GRPCServer

} // end namespace Controller
