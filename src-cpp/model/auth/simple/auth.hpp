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

#ifndef _MODEL_AUTH_SIMPLE_AUTH_HPP_
#define _MODEL_AUTH_SIMPLE_AUTH_HPP_

#include <unordered_map>
#include <vector>

#include "model/auth/iauth.hpp"
#include "model/defines.h"

namespace Model
{

namespace Auth
{

namespace Simple
{

typedef struct IPData
{
    u8 retry = 0;
    u64 lastAccess = 0;
    bool isNotBaned = true;
} IPData;

class Auth : public IAuth
{

public:

    Auth();

    ~Auth();

    virtual u8 login(const std::string &username,
                     const std::string &password,
                     const std::string &otp,
                     std::string &token) override;

    virtual u8 logout(const std::string &username) override;

    virtual u8 cannotAccess(const std::string &ip) override;

    virtual u8 cannotAccess(const std::string &ip, const std::string &token) override;

    virtual void addBannedIp(const std::string &ip) override;

    std::string username = "test";
    
    std::string password = ""; // hashed password

    std::vector<u8> key;

    u8 maxRetry = 3;

    u64 banTime = 600;

    u64 tokenTimeout = 86400; // 24hr

private:

    u64 m_retry = 0;

    u64 m_lastAccess = 0;

    bool m_baned = false;

    std::unordered_map<std::string, IPData> m_ipBanList;

    std::string m_currentToken = "";

    std::mutex m_mutex;

    u8 genToken();

}; // class Auth

} // end namespace Simple

} // end namespace Auth

} // end namespace Model

#endif // _MODEL_AUTH_SIMPLE_AUTH_HPP_