/*
 * Flex Flow
 * Copyright (c) 2023-present fdar0536
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

#include "model/defines.h"
#include <vector>
#ifdef _WIN32
#include "direct.h"
#else
#include "unistd.h"
#endif

#include "openssl/rand.h"
#include "spdlog/spdlog.h"
#include "CLI/CLI.hpp"

#include "model/auth/simple/auth.hpp"
#include "model/auth/crypto.hpp"
#include "model/utils.hpp"
#include "init.hpp"

#include "config.hpp"

#define MIN_LOGIN_TIMEOUT 1800 // 30 mins

namespace Controller
{

namespace GRPCServer
{

Config::Config()
{}

Config::~Config()
{}

u8 Config::parse(Config *in, int argc, char **argv)
{
    spdlog::debug("{}:{} Config::parse", LOG_FILE_PATH(__FILE__), __LINE__);

    if (!in)
    {
        spdlog::warn("{}:{} input is nullptr", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (!argv)
    {
        spdlog::error("{}:{} you should never see this line", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    char buf[1024];
    size_t len(1024);
#ifdef _WIN32
    if (!_getcwd(buf, len))
#else
    if (!getcwd(buf, len))
#endif
    {
        spdlog::error("{}:{} Fail to get current path", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (argc == 1 || !argv)
    {
        spdlog::warn("{}:{} use default config", LOG_FILE_PATH(__FILE__), __LINE__);
        in->dbPath = std::string(buf);
        return 0;
    }

    std::string configFile("");

    std::string appName = "Flex Flow ";
    appName.append(FF_VERSION);
    appName.append(" gRPC Server");
    
    CLI::App app(appName);
    app.set_help_flag();
    
    app.add_option("-c,--config-file", configFile, 
        "path to server configuration file")->default_str("config.yaml");
    app.add_flag("-v,--version","print version and exit");
    app.add_flag("-h,--help","print help and exit");

    CLI11_PARSE(app, argc, argv);

    if (app.count("-v"))
    {
        printVersion(argv[0]);
        return 2;
    }

    if (app.count("-h"))
    {
        fmt::println("{}", app.help());
        return 2;
    }

    if (configFile.empty())
    {
        spdlog::error("{}:{} no config file", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (Model::Utils::verifyFile(configFile))
    {
        spdlog::error("{}:{} Fail to verify config file", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (parse(in, configFile))
    {
        spdlog::error("{}:{} fail to parse config file", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return 0;
}

uint_fast8_t Config::parse(Config *obj, const std::string &path)
{
    spdlog::debug("{}:{} Config::parse", LOG_FILE_PATH(__FILE__), __LINE__);

    if (!obj)
    {
        spdlog::warn("{}:{} input is nullptr", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (!auth)
    {
        spdlog::error("{}:{} auth is nullptr", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    try
    {
        YAML::Node config = YAML::LoadFile(path);

        obj->dbPath = config["db path"].as<std::string>();

        obj->listenPort = config["port"].as<u16>();

        if (parseAuth(config, path))
        {
            spdlog::error("{}:{} fail to parse auth config",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }
    }
    catch (...)
    {
        spdlog::error("{}:{} error caught", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return 0;
}

// private member functions
void Config::printVersion(const char *name)
{
    fmt::println("{} {}", name, FF_VERSION);
    fmt::println("Branch:  " FF_BRANCH);
    fmt::println("Commit:  " FF_COMMIT);
}

u8 Config::parseAuth(YAML::Node &config, const std::string &path)
{
    spdlog::debug("{}:{} Config::parseAuth", LOG_FILE_PATH(__FILE__), __LINE__);
    spdlog::debug("{}:{} path: {}", LOG_FILE_PATH(__FILE__), __LINE__, path);

    YAML::Node authConfig = config["auth"];
    auto simpleAuth =
        dynamic_cast<Model::Auth::Simple::Auth *>(auth);
    bool needWriteBack = false;
    simpleAuth->username = authConfig["username"].as<std::string>();
    std::string password = authConfig["password"].as<std::string>();
    if (password.empty())
    {
        spdlog::error("{}:{} password is empty", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    std::string salt = authConfig["salt"].as<std::string>();
    if (salt.empty())
    {
        needWriteBack = true;

        // generate 128bits salt
        // so 128 / 8 = 16(bytes)
        std::vector<u8> out = std::vector<u8>(16);
        if (RAND_bytes(out.data(), out.size()) != 1)
        {
            spdlog::error("{}:{} OpenSSL RAND_bytes failed",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        if (Model::Auth::Crypto::encodeBase64(out, salt))
        {
            spdlog::error("{}:{} Fail to encode salt",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        authConfig["salt"] = salt;
        simpleAuth->salt = out;

        // hash password
        out.clear();
        if (Model::Auth::Crypto::argon2id(password, simpleAuth->salt, out))
        {
            spdlog::error("{}:{} argon2id failed",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        // encode password
        if (Model::Auth::Crypto::encodeBase64(out, password))
        {
            spdlog::error("{}:{} Fail to encode password",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        authConfig["password"] = password;
    }
    else
    {
        // decode salt
        if (Model::Auth::Crypto::decodeBase64(salt, simpleAuth->salt))
        {
            spdlog::error("{}:{} Fail to decode salt",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }
        
        if (simpleAuth->salt.size() < 16)
        {
            spdlog::error("{}:{} salt is too short or invalid salt",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        // decode password
        if (Model::Auth::Crypto::decodeBase64(password, simpleAuth->password))
        {
            spdlog::error("{}:{} Fail to decode password",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }
    }

    std::string totpKey = authConfig["totp key"].as<std::string>();
    if (totpKey.empty())
    {
        needWriteBack = true;

        // generate 256bits key
        // 256 / 8 = 32(bytes)
        std::vector<u8> out = std::vector<u8>(32);
        if (RAND_bytes(out.data(), out.size()) != 1)
        {
            spdlog::error("{}:{} OpenSSL RAND_bytes failed",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        simpleAuth->totpKey = out;

        // encode it into base32
        Model::Auth::Crypto::encodeBase32(out, totpKey);

        // write back to yaml
        authConfig["totp key"] = totpKey;
    }
    else
    {
        // decode totp key
        Model::Auth::Crypto::decodeBase32(totpKey, simpleAuth->totpKey);
        if (simpleAuth->totpKey.size() < 10) // 80 bits
        {
            spdlog::error("{}:{} totp key is too short or invalid totp key",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }
    }

    simpleAuth->banTime = authConfig["ban time"].as<i32>();
    simpleAuth->maxRetry = authConfig["max retry"].as<u8>();
    simpleAuth->minTokenTimeout = authConfig["min token timeout"].as<i32>();
    simpleAuth->maxTokenTimeout = authConfig["max token timeout"].as<i32>();

    if (simpleAuth->minTokenTimeout > simpleAuth->maxTokenTimeout)
    {
        spdlog::error("{}:{} default token timeout is greater than max token timeout",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (simpleAuth->minTokenTimeout < MIN_LOGIN_TIMEOUT)
    {
        spdlog::error("{}:{} default token timeout is less than {} s",
            LOG_FILE_PATH(__FILE__), __LINE__, MIN_LOGIN_TIMEOUT);
        return 1;
    }

    if (simpleAuth->maxTokenTimeout < MIN_LOGIN_TIMEOUT)
    {
        spdlog::error("{}:{} max token timeout is less than {} s",
            LOG_FILE_PATH(__FILE__), __LINE__, MIN_LOGIN_TIMEOUT);
        return 1;
    }

    if (needWriteBack)
    {
        // write entire yaml back into file
        config["auth"] = authConfig;

        std::ofstream fout(path);
        if (!fout.is_open())
        {
            spdlog::error("{}:{} Fail to open config file",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        fout << config; 
    }

    return 0;
}

} // end namespace GRPCServer

} // end namespace Model
