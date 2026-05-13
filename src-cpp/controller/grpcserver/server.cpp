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

#include "spdlog/spdlog.h"
#include "grpcpp/server_builder.h"
#include "grpcpp/server.h"

#include "controller/grpcserver/init.hpp"

#include "model/utils.hpp"

#include "authinterceptor.hpp"
#include "server.hpp"

namespace Controller
{

namespace GRPCServer
{

class AuthInterceptorFactory :
    public grpc::experimental::ServerInterceptorFactoryInterface
{
public:
    grpc::experimental::Interceptor
    *CreateServerInterceptor(grpc::experimental::ServerRpcInfo *info) override
    {
        return new AuthInterceptor(info);
    }
};

Server::Server()
{}

Server::~Server()
{
    stop();
}

uint_fast8_t Server::start()
{
    try
    {
        grpc::ServerBuilder builder;
        std::string listenAddr = GRPCServer::config.listenIP + ":" +
                                 std::to_string(GRPCServer::config.listenPort);
        int actualPort(0);
        builder.AddListeningPort(listenAddr,
                                 grpc::InsecureServerCredentials(),
                                 &actualPort);

        builder.RegisterService(&m_accessImpl);
        builder.RegisterService(&m_queueImpl);
        builder.RegisterService(&m_queueListImpl);

        std::vector<std::unique_ptr<grpc::experimental::ServerInterceptorFactoryInterface>>
            creators;
        creators.reserve(1);
        creators.push_back(std::make_unique<AuthInterceptorFactory>());
        builder.experimental()
            .SetInterceptorCreators(std::move(creators));
        
        auto server = builder.BuildAndStart();
        spdlog::info("{}:{} Server is listening on {}",
            LOG_FILE_PATH(__FILE__), __LINE__,
            listenAddr);

        auto serveFn = [&]()
        {
            server->Wait();
        };

        m_thread = std::jthread(serveFn);

        std::unique_lock<std::mutex> lock(m_cvMutex);
        m_cv.wait(lock, [this]{ return m_done; });

        server->Shutdown();
        m_thread = std::jthread();
    }
    catch (...)
    {
        spdlog::error("{}:{} Server failed",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return 0;
}

void Server::stop()
{
    {
        std::lock_guard<std::mutex> lock(m_cvMutex);
        m_done = true;
    }

    m_cv.notify_one();
}

} // end namespace GRPCServer

} // end namespace Controller
