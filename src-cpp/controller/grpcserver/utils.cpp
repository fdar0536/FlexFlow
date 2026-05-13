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

#include "utils.hpp"

namespace Controller
{

namespace GRPCServer
{

namespace Utils
{

// this function is assisted by Google Gemini
static std::string getCleanIP(const std::string& peer)
{
    spdlog::debug("{}:{} Utils::getCleanIP", LOG_FILE_PATH(__FILE__), __LINE__);
    spdlog::debug("{}:{} peer: {}", LOG_FILE_PATH(__FILE__), __LINE__, peer);

    // format: <ipv4 or ipv6>:<address>:<port>
    // 1. find first colon and skip protocol (ipv4: or ipv6:)
    size_t firstColon = peer.find(':');
    if (firstColon == std::string::npos) return peer;

    // get "<address>:<port>"
    std::string addrPort = peer.substr(firstColon + 1);

    // 2. find last colon from right to separate ip and port
    size_t lastColon = addrPort.find_last_of(':');
    if (lastColon == std::string::npos) return addrPort;

    std::string ip = addrPort.substr(0, lastColon);

    // 3. handle ipv6's [ ]
    if (!ip.empty() && ip.front() == '[' && ip.back() == ']')
    {
        ip = ip.substr(1, ip.size() - 2);
    }

    return ip;
}

// this function is assisted by Google Gemini
std::string getIPFromContext(grpc::ServerContextBase *ctx)
{
    spdlog::debug("{}:{} Utils::getIPFromContext",
        LOG_FILE_PATH(__FILE__), __LINE__);

    auto metadata = ctx->client_metadata();

    // 1. check X-Forwarded-For (from nginx)
    auto it = metadata.find("x-forwarded-for");
    if (it != metadata.end())
    {
        return std::string(it->second.data(), it->second.length());
    } 
    // 2. check X-Real-IP
    else if (it = metadata.find("x-real-ip"); it != metadata.end())
    {
        return std::string(it->second.data(), it->second.length());
    } 
    
    // 3. if it has no data in header, get ip from grpc peer
    return getCleanIP(ctx->peer());
}

u8 getTokenFromContext(grpc::ServerContextBase *ctx, std::string &out)
{
    spdlog::debug("{}:{} Utils::getTokenFromContext",
        LOG_FILE_PATH(__FILE__), __LINE__);

    auto metadata = ctx->client_metadata();
    out.clear();
    auto it = metadata.find("x-auth-token");
    if (it != metadata.end())
    {
        out = std::string(it->second.data(), it->second.length());
        if (out.empty())
        {
            spdlog::error("{}:{} token is empty string",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        return 0;
    }

    spdlog::error("{}:{} token is not found", LOG_FILE_PATH(__FILE__), __LINE__);
    return 1;
}

} // end namespace Utils

} // end namespace GRPCServer

} // end namespace Controller
