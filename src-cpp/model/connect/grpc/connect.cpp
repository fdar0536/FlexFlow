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
#include "model/dao/grpc/utils.hpp"

#include "connect.hpp"

namespace Model
{

namespace Connect
{

namespace GRPC
{

std::shared_ptr<grpc::ChannelInterface> connect(const std::string &target, const u16 port)
{
    spdlog::debug("{}:{} Model::Connect::GRPC::connect",
        LOG_FILE_PATH(__FILE__), __LINE__);

    auto token = std::shared_ptr<grpc::ChannelInterface>();

    std::string ip = target;
    ip += ":";
    ip += std::to_string(port);
    std::unique_ptr<ff::Access::Stub> stub;

    try
    {
        token = grpc::CreateChannel(ip, 
            grpc::InsecureChannelCredentials());

        if (token == nullptr)
        {
            spdlog::error("{}:{} Fail to create channel",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return nullptr;
        }

        stub = ff::Access::NewStub(token);
        if (stub == nullptr)
        {
            spdlog::error("{}:{} Fail to create access' stub",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return nullptr;
        }
    }
    catch (...)
    {
        spdlog::error("{}:{} Fail to initialize connection",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return nullptr;
    }

    return token;
}

} // end namespace GRPC

} // end namespace Connect

} // end namespace Model
