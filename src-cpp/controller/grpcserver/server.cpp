/*
 * Simple Task Queue
 * Copyright (c) 2023 fdar0536
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

#include "server.hpp"

#include "controller/grpcserver/init.hpp"

namespace Controller
{

namespace GRPCServer
{

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
        m_server = builder.BuildAndStart();
        spdlog::info("{}:{} Server is listening on {}", __FILE__, __LINE__,
                     listenAddr);

        auto serveFn = [this]()
        {
            this->m_server->Wait();
        };

        m_thread = std::jthread(serveFn);
        m_future = m_exitRequested.get_future();
        m_future.wait();
        m_server->Shutdown();
        m_thread = std::jthread();
        m_server = nullptr;
    }
    catch (...)
    {
        spdlog::error("{}:{} Fail to start server", __FILE__, __LINE__);
        m_server = nullptr;
        return 1;
    }

    return 0;
}

void Server::stop()
{
    m_exitRequested.set_value();
}

} // end namespace GRPCServer

} // end namespace Controller
