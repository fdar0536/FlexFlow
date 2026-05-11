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
#include "openssl/evp.h"
#include "openssl/hmac.h"
#include "openssl/rand.h"
#include "spdlog/spdlog.h"

#include "model/utils.hpp"

#include "crypto.hpp"

namespace Model
{

namespace Auth
{

namespace Crypto
{

static const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

void decodeBase32(const std::string &base32, std::vector<u8> &out)
{
    spdlog::debug("{}:{} Model::Auth::Utils::decodeBase32",
                  LOG_FILE_PATH(__FILE__), __LINE__);
    spdlog::debug("base32: {}", base32);
    spdlog::debug("out.size(): {}", out.size());

    out.clear();
    out.reserve(32); // 256 bits
    int bitsLeft = 0;
    uint32_t currentByte = 0;

    for (char c : base32)
    {
        auto val = alphabet.find(toupper(c));
        if (val == std::string::npos) continue;
        currentByte = (currentByte << 5) | (val & 0x1F);
        bitsLeft += 5;
        if (bitsLeft >= 8)
        {
            out.push_back((currentByte >> (bitsLeft - 8)) & 0xFF);
            bitsLeft -= 8;
        }
    }
}

std::string generateTotp(const std::vector<u8> &key, u64 time_step)
{
    spdlog::debug("{}:{} Model::Auth::Utils::generateTotp",
                  LOG_FILE_PATH(__FILE__), __LINE__);
    spdlog::debug("secret.size(): {}", key.size());

    u64 counter = std::time(nullptr) / time_step;
    
    // convert to big endian
    uint8_t msg[8];
    for (int i = 7; i >= 0; --i)
    {
        msg[i] = counter & 0xFF;
        counter >>= 8;
    }

    // calculate HMAC-SHA1
    unsigned int md_len = 0;
    unsigned char md[EVP_MAX_MD_SIZE];
    
    HMAC(EVP_sha1(), key.data(), key.size(), msg, 8, md, &md_len);

    // Dynamic Truncation
    int offset = md[md_len - 1] & 0x0F;
    uint32_t binary = ((md[offset] & 0x7F) << 24) |
                      ((md[offset + 1] & 0xFF) << 16) |
                      ((md[offset + 2] & 0xFF) << 8) |
                      (md[offset + 3] & 0xFF);

    // get 6 digitals
    std::stringstream out;
    out << std::setfill('0') << std::setw(6) << (binary % 1000000);
    return out.str();
}

std::string sha512(const std::string &input)
{
    spdlog::debug("{}:{} Model::Auth::Utils::sha512",
                  LOG_FILE_PATH(__FILE__), __LINE__);
    spdlog::debug("input: {}", input);

    // initialize
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    const EVP_MD* algorithm = EVP_sha512();
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int length = 0;

    // calculate
    EVP_DigestInit_ex(context, algorithm, nullptr);
    EVP_DigestUpdate(context, input.c_str(), input.size());
    EVP_DigestFinal_ex(context, hash, &length);

    // clean up
    EVP_MD_CTX_free(context);

    // to Hex String
    std::stringstream ss;
    for (unsigned int i = 0; i < length; ++i)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }

    return ss.str();
}

u8 genHexString(size_t length, std::string &out)
{
    spdlog::debug("{}:{} Model::Auth::Utils::genHexString",
                  LOG_FILE_PATH(__FILE__), __LINE__);
    spdlog::debug("length: {}", length);

    // Each hex character represents 4 bits, so 2 characters = 1 byte
    size_t num_bytes = (length + 1) / 2;
    std::vector<unsigned char> buffer(num_bytes);

    // RAND_bytes returns 1 on success
    if (RAND_bytes(buffer.data(), buffer.size()) != 1)
    {
        spdlog::error("{}:{} OpenSSL RAND_bytes failed",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (auto b : buffer)
    {
        ss << std::setw(2) << static_cast<int>(b);
    }

    out = ss.str().substr(0, length);
    return 0;
}

} // end namespace Utils

} // end namespace Auth

} // end namespace Model
