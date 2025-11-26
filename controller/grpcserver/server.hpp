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

#ifndef _CONTROLLER_GRPCSERVER_SERVER_HPP_
#define _CONTROLLER_GRPCSERVER_SERVER_HPP_

#include <thread>

#include <cinttypes>

#include "grpcpp/server.h"

#include "controller/global/defines.h"
#include "accessimpl.hpp"
#include "queueimpl.hpp"
#include "queuelistimpl.hpp"

#include <future>

namespace Controller
{

namespace GRPCServer
{

class Server
{
public:

    Server();

    ~Server();

    u8 start();

    void stop();

private:

    std::jthread m_thread;

    std::promise<void> m_exitRequested;

    std::future<void> m_future;

    std::unique_ptr<grpc::Server> m_server = nullptr;

    AccessImpl m_accessImpl;

    QueueImpl m_queueImpl;

    QueueListImpl m_queueListImpl;

};

} // end namespace GRPCServer

} // end namespace Controller

#endif // _CONTROLLER_GRPCSERVER_SERVER_HPP_
