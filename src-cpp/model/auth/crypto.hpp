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

#ifndef _MODEL_AUTH_UTILS_HPP_
#define _MODEL_AUTH_UTILS_HPP_

#include <vector>
#include <string>

#include "model/defines.h"


namespace Model
{

namespace Auth
{

namespace Crypto
{

void decodeBase32(const std::string &base32, std::vector<u8> &out);

void encodeBase32(const std::vector<u8> &in, std::string &out);

void decodeBase64(const std::string &base64, std::vector<u8> &out);

void encodeBase64(const std::vector<u8> &in, std::string &out);

std::string generateTotp(const std::vector<u8> &key,
                         u64 time_step = 30);

std::string sha512(const std::string &input);

u8 argon2id(const std::string& password, 
            const std::vector<uint8_t>& salt, 
            std::vector<uint8_t>& out_hash);

} // end namespace Utils

} // end namespace Auth

} // end namespace Model

#endif // _MODEL_AUTH_UTILS_HPP_