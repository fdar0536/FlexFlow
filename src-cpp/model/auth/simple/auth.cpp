/*
 * Flex Flow
 * Copyright (c) 2026-presnet fdar0536
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

#include <iomanip>
#include <mutex>
#include <sstream>

#include "openssl/rand.h"
#include "spdlog/spdlog.h"

#include "model/utils.hpp"
#include "model/auth/crypto.hpp"

#include "auth.hpp"

namespace Model
{

namespace Auth
{

namespace Simple
{

Auth::Auth()
{}

Auth::~Auth()
{}

u8 Auth::login(const std::string &username,
               const std::string &password,
               const std::string &otp,
               std::string &token)
{
    spdlog::debug("{}:{} Model::Auth::Simple::Auth::login",
                  LOG_FILE_PATH(__FILE__), __LINE__);
    spdlog::debug("username: {}", username);
    spdlog::debug("password: {}", password);
    spdlog::debug("otp: {}", otp);

    if (unBanUser())
    {
        spdlog::error("{}:{} user is banned", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    // verify totp first
    if (otp.empty())
    {
        spdlog::error("{}:{} otp is empty", LOG_FILE_PATH(__FILE__), __LINE__);
        banUser();
        return 1;
    }

    if (Crypto::generateTotp(totpKey) != otp)
    {
        spdlog::error("{}:{} otp is invalid", LOG_FILE_PATH(__FILE__), __LINE__);
        banUser();
        return 1;
    }

    if (username.empty() || password.empty())
    {
        spdlog::error("{}:{} username or password is empty",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (username != this->username)
    {
        spdlog::error("{}:{} username is invalid",
            LOG_FILE_PATH(__FILE__), __LINE__);
        banUser();
        return 1;
    }

    std::vector<u8> hash;
    if (Crypto::argon2id(password, salt, hash))
    {
        spdlog::error("{}:{} argon2id failed", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (hash != this->password)
    {
        spdlog::error("{}:{} password is invalid",
            LOG_FILE_PATH(__FILE__), __LINE__);
        banUser();
        return 1;
    }

    if (genToken())
    {
        spdlog::error("{}:{} genToken failed", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    token = m_currentToken;
    m_retry = 0;
    m_lastAccess = std::time(nullptr);
    return 0;
}

u8 Auth::logout(const std::string &username, const std::string &token)
{
    spdlog::debug("{}:{} Model::Auth::Simple::Auth::logout",
        LOG_FILE_PATH(__FILE__), __LINE__);
    spdlog::debug("username: {}", username);
    spdlog::debug("token: {}", token);

    if (username.empty() || token.empty())
    {
        spdlog::error("{}:{} username or token is empty",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (username != this->username)
        {
            spdlog::error("{}:{} username is invalid",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        if (m_currentToken.empty())
        {
            spdlog::warn("{}:{} token is empty", LOG_FILE_PATH(__FILE__), __LINE__);
            return 0; // already logout
        }

        if (token != m_currentToken)
        {
            spdlog::error("{}:{} token is invalid",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        m_currentToken = "";
    }

    return 0;
}

u8 Auth::cannotAccess(const std::string &ip)
{
    spdlog::debug("{}:{} Model::Auth::Simple::Auth::cannotAccess",
                  LOG_FILE_PATH(__FILE__), __LINE__);
    spdlog::debug("ip: {}", ip);

    if (ip.empty())
    {
        spdlog::error("{}:{} ip is empty", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    std::unique_lock<std::mutex> lock(m_mutex);

    auto it = m_ipBanList.find(ip);
    if (it != m_ipBanList.end())
    {
        // check it is unbanned or not
        if (it->second.isNotBaned)
        {
            return 0;
        }

        // isNotBanned == false
        u64 now = std::time(nullptr);
        if ((now - it->second.lastAccess) <= banTime)
        {
            it->second.lastAccess = now;
            spdlog::error("{}:{} ip is banned",
                LOG_FILE_PATH(__FILE__), __LINE__);        
            return 1;
        }
        
        m_ipBanList.erase(it);
    }

    return 0;
}

u8 Auth::cannotAccess(const std::string &ip, const std::string &token)
{
    spdlog::debug("{}:{} Model::Auth::Simple::Auth::cannotAccess",
        LOG_FILE_PATH(__FILE__), __LINE__);
    spdlog::debug("ip: {}", ip);
    spdlog::debug("token: {}", token);

    if (cannotAccess(ip))
    {
        spdlog::error("{}:{} this ip is banned",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (m_currentToken.empty())
    {
        addBannedIp(ip);
        spdlog::error("{}:{} client is not login and access the server",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (token != m_currentToken)
    {
        addBannedIp(ip);
        spdlog::error("{}:{} token is invalid",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    u64 now = std::time(nullptr);
    if ((now - m_lastAccess) > tokenTimeout)
    {
        // token expired
        m_currentToken = "";
        return 1;
    }

    // reset failed time
    auto it = m_ipBanList.find(ip);
    if (it != m_ipBanList.end())
    {
        m_ipBanList.erase(it);
    }

    return 0;
}

void Auth::addBannedIp(const std::string &ip)
{
    spdlog::debug("{}:{} Model::Auth::Simple::Auth::addBannedIp",
        LOG_FILE_PATH(__FILE__), __LINE__);
    spdlog::debug("ip: {}", ip);

    std::unique_lock<std::mutex> lock(m_mutex);

    auto it = m_ipBanList.find(ip);
    if (it != m_ipBanList.end())
    {
        if (it->second.isNotBaned == false)
        {
            it->second.lastAccess = std::time(nullptr);
            return;
        }

        ++it->second.retry;
        if (it->second.retry > maxRetry)
        {
            it->second.isNotBaned = false;
            it->second.lastAccess = std::time(nullptr);
        }

        return;
    }

    m_ipBanList[ip].retry = 1;
}

void Auth::removeBannedIp(const std::string &ip)
{
    spdlog::debug("{}:{} Model::Auth::Simple::Auth::removeBannedIp",
        LOG_FILE_PATH(__FILE__), __LINE__);
    spdlog::debug("ip: {}", ip);

    std::unique_lock<std::mutex> lock(m_mutex);

    auto it = m_ipBanList.find(ip);
    if (it != m_ipBanList.end())
    {
        m_ipBanList.erase(it);
    }
}

// private member functions
u8 Auth::genToken()
{
    spdlog::debug("{}:{} Model::Auth::Simple::Auth::genToken",
                  LOG_FILE_PATH(__FILE__), __LINE__);
    
    std::vector<u8> out = std::vector<u8>(16);
    if (RAND_bytes(out.data(), out.size()) != 1)
    {
        spdlog::error("{}:{} OpenSSL RAND_bytes failed",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (u8 b : out)
    {
        ss << std::setw(2) << (int)b;
    }

    std::unique_lock<std::mutex> lock(m_mutex);
    m_currentToken = username + ss.str();
    m_currentToken = Crypto::sha512(m_currentToken);
    return 0;
}

void Auth::banUser()
{
    spdlog::debug("{}:{} Model::Auth::Simple::Auth::banUser",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    ++m_retry;
    if (m_retry > maxRetry)
    {
        m_baned = true;
        m_lastAccess = std::time(nullptr);
    }
}

u8 Auth::unBanUser()
{
    spdlog::debug("{}:{} Model::Auth::Simple::Auth::unBanUser",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    if (m_baned)
    {
        u64 now = std::time(nullptr);
        if ((now - m_lastAccess) <= banTime)
        {
            m_lastAccess = now;
            spdlog::error("{}:{} user is banned",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        m_baned = false;
        m_retry = 0;
    }

    return 0;
}

} // end namespace Simple

} // end namespace Auth

} // end namespace Model
