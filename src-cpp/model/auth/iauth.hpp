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

#ifndef _MODEL_AUTH_IAUTH_HPP_
#define _MODEL_AUTH_IAUTH_HPP_

#include <string>

#include "model/defines.h"

namespace Model
{

namespace Auth
{
/**
 * @brief Defines the interface for authentication and access control.
 *
 * @note All member functions are thread-safe.
 */
class IAuth
{
public:

    virtual ~IAuth() = default;

    /**
     * @brief Authenticates a user and issues an authentication token.
     *
     * @param[in] username User name.
     * @param[in] password User password.
     * @param[in] otp One-time password, or an empty string when not provided.
     * @param[out] token Receives the authentication token on success.
     * @param[in,out] tokenTimeout Requested token lifetime in seconds.
     *                             The actual lifetime is determined by this
     *                             function and written back to this parameter.
     *
     * @return 0 on success; otherwise, a non-zero value.
     *
     * @note The output parameters are updated only on success.
     */
    virtual u8 login(const std::string &username,
                     const std::string &password,
                     const std::string &otp,
                     std::string &token,
                     i32 &tokenTimeout) = 0;

    /**
     * @brief Invalidates an authentication token.
     *
     * @param[in] token Authentication token returned by login().
     *
     * @return 0 on success; otherwise, a non-zero value.
     */
    virtual u8 logout(const std::string &token) = 0;

    /**
     * @brief Checks whether access is allowed for a remote IP address.
     *
     * @param[in] ip Remote IP address.
     *
     * @return 0 if access is allowed; otherwise, a non-zero value.
     */
    virtual u8 cannotAccess(const std::string &ip) = 0;

    /**
     * @brief Checks whether access is allowed for an authenticated request.
     *
     * @param[in] ip Remote IP address.
     * @param[in] token Authentication token returned by login().
     *
     * @return 0 if access is allowed; otherwise, a non-zero value.
     */
    virtual u8 cannotAccess(const std::string &ip,
                            const std::string &token) = 0;

    /**
     * @brief Adds an IP address to the banned IP list.
     *
     * @param[in] ip Remote IP address to ban.
     *
     * @note Adding an already banned IP address has no effect.
     */
    virtual void addBannedIp(const std::string &ip) = 0;

    /**
     * @brief Removes an IP address from the banned IP list.
     *
     * @param[in] ip Remote IP address to unban.
     *
     * @note Removing an IP address that is not banned has no effect.
     */
    virtual void removeBannedIp(const std::string &ip) = 0;
};

} // end namespace Auth

} // end namespace Model

#endif // _MODEL_AUTH_IAUTH_HPP_
