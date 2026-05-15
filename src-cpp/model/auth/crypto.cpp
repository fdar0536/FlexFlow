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

#include "openssl/bio.h"
#include "openssl/buffer.h"
#include "openssl/core_names.h"
#include "openssl/evp.h"
#include "openssl/hmac.h"
#include "openssl/kdf.h"
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

void encodeBase32(const std::vector<u8> &in, std::string &out)
{
    spdlog::debug("{}:{} Model::Auth::Utils::encodeBase32",
                  LOG_FILE_PATH(__FILE__), __LINE__);
    spdlog::debug("in.size(): {}", in.size());

    out.clear();
    out.reserve(((in.size() + 4) / 5) << 3);

    uint32_t buffer = 0;
    int bitsLeft = 0;

    for (u8 b : in)
    {
        buffer = (buffer << 8) | b;
        bitsLeft += 8;

        // when buffer excess 5 bits, output one char
        while (bitsLeft >= 5)
        {
            bitsLeft -= 5;
            out.push_back(alphabet[(buffer >> bitsLeft) & 0x1F]);
        }
    }

    // handle bits left
    if (bitsLeft > 0)
    {
        out.push_back(alphabet[(buffer << (5 - bitsLeft)) & 0x1F]);
    }
}

void decodeBase64(const std::string &base64, std::vector<u8> &out)
{
    spdlog::debug("{}:{} Model::Auth::Utils::decodeBase64",
                  LOG_FILE_PATH(__FILE__), __LINE__);
    spdlog::debug("base64: {}", base64);
    spdlog::debug("out.size(): {}", out.size());

    out.clear();
    out.reserve(base64.size());

    std::unique_ptr<BIO, decltype(&BIO_free_all)> b64(BIO_new(BIO_f_base64()), BIO_free_all);
    std::unique_ptr<BIO, decltype(&BIO_free)> bmem(BIO_new_mem_buf(base64.data(), static_cast<int>(base64.size())), BIO_free);
    
    BIO_set_flags(b64.get(), BIO_FLAGS_BASE64_NO_NL);
    BIO_push(b64.get(), bmem.get());
    
    int decoded_size = BIO_read(b64.get(), out.data(), static_cast<int>(out.size()));
    
    if (decoded_size > 0)
    {
        out.resize(decoded_size);
    }
    else
    {
        out.clear();
    }
}

void encodeBase64(const std::vector<u8> &in, std::string &out)
{
    spdlog::debug("{}:{} Model::Auth::Utils::encodeBase64",
        LOG_FILE_PATH(__FILE__), __LINE__);
    spdlog::debug("in.size(): {}", in.size());

    out.clear();

    std::unique_ptr<BIO, decltype(&BIO_free_all)> b64(BIO_new(BIO_f_base64()), BIO_free_all);
    std::unique_ptr<BIO, decltype(&BIO_free)> bmem(BIO_new(BIO_s_mem()), BIO_free);
    
    BIO_set_flags(b64.get(), BIO_FLAGS_BASE64_NO_NL);
    
    BIO_push(b64.get(), bmem.get());
    
    BIO_write(b64.get(), in.data(), static_cast<int>(in.size()));
    BIO_flush(b64.get());
    
    BUF_MEM *bptr;
    BIO_get_mem_ptr(b64.get(), &bptr);
    out = std::string(bptr->data, bptr->length);
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

u8 argon2id(const std::string& password, 
            const std::vector<uint8_t>& salt, 
            std::vector<uint8_t>& out_hash)
{
    spdlog::debug("{}:{} Model::Auth::Utils::argon2id",
                  LOG_FILE_PATH(__FILE__), __LINE__);
    spdlog::debug("password: {}", password);
    spdlog::debug("salt.size(): {}", salt.size());

    out_hash.clear();
    out_hash.resize(32);

    EVP_KDF *kdf = nullptr;
    EVP_KDF_CTX *kctx = nullptr;
    u8 ret(1);

    // 1. get Argon2id
    kdf = EVP_KDF_fetch(nullptr, "ARGON2ID", nullptr);
    if (!kdf)
    {
        spdlog::error("{}:{} Error: EVP_KDF_fetch failed",
                      LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    // 2. setup KDF context
    kctx = EVP_KDF_CTX_new(kdf);
    EVP_KDF_free(kdf);
    if (!kctx)
    {
        spdlog::error("{}:{} Error: EVP_KDF_CTX_new failed",
                      LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    // 3. set Argon2id parameter
    uint32_t lanes = 4;           // Parallelism
    uint32_t mem_cost = 65536;    // 64MB
    uint32_t iterations = 3;      // Time cost

    OSSL_PARAM params[7];
    params[0] = OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_PASSWORD, (void*)password.data(), password.size());
    params[1] = OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_SALT, (void*)salt.data(), salt.size());
    params[2] = OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_THREADS, &lanes);
    params[3] = OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ARGON2_LANES, &lanes);
    params[4] = OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ARGON2_MEMCOST, &mem_cost);
    params[5] = OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ITER, &iterations);
    params[6] = OSSL_PARAM_construct_end();

    // 4. encrypt the password
    if (EVP_KDF_derive(kctx, out_hash.data(), out_hash.size(), params) > 0)
    {
        ret = 0;
    }
    else
    {
        spdlog::error("{}:{} Error: EVP_KDF_derive failed",
                      LOG_FILE_PATH(__FILE__), __LINE__);
    }

    EVP_KDF_CTX_free(kctx);
    return ret;
}

} // end namespace Utils

} // end namespace Auth

} // end namespace Model
