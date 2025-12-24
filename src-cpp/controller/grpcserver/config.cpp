/*
 * Simple Task Queue
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

#ifdef _WIN32
#include "direct.h"
#else
#include "unistd.h"
#endif

#include "spdlog/spdlog.h"
#include "cxxopts.hpp"
#include "yaml-cpp/yaml.h"

#include "controller/global/global.hpp"
#include "model/dao/dirutils.hpp"
#include "model/utils.hpp"
#include "config.hpp"

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
        in->logPath = std::string(buf);
        return 0;
    }

    std::string configFile("");

    try
    {
        cxxopts::Options options("FFSERVER", "FF Server");
        options.add_options()
            ("c,config-file", "path to config file", cxxopts::value<std::string>(configFile)->default_value(""))
            ("d,db-path", "path to config file", cxxopts::value<std::string>(in->dbPath)->default_value(std::string(buf)))
            ("l,log-path", "path for output log", cxxopts::value<std::string>(in->logPath)->default_value(""))
            ("L,log-level", "log level for spdlog", cxxopts::value<i32>(in->logLevel)->default_value("2"))
            ("a,address", "which addess will listen", cxxopts::value<std::string>(in->listenIP)->default_value("127.0.0.1"))
            ("p,port", "which port will listen", cxxopts::value<u16>(in->listenPort)->default_value("12345"))
            ("v,version", "print version")
            ("h,help", "print help")
            ;

        auto result = options.parse(argc, argv);
        if (result.count("help"))
        {
            fmt::print("{}", options.help());
            return 2;
        }

        if (result.count("version"))
        {
            printVersion();
            return 2;
        }

        if (!configFile.empty())
        {
            if (Model::DAO::DirUtils::verifyFile(configFile))
            {
                spdlog::error("{}:{} Fail to verify config file", LOG_FILE_PATH(__FILE__), __LINE__);
                return 1;
            }
        }

        if (!in->logPath.empty())
        {
            Model::DAO::DirUtils::convertPath(in->logPath);
            if (Model::DAO::DirUtils::verifyDir(in->logPath))
            {
                spdlog::error("{}:{} fail to verify log path", LOG_FILE_PATH(__FILE__), __LINE__);
                return 1;
            }
        }

        if (Model::Utils::verifyIP(in->listenIP))
        {
            spdlog::error("{}:{} Invalid ip", LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }
    }
    catch(const cxxopts::exceptions::exception &e)
    {
        spdlog::error("{}:{} {}", LOG_FILE_PATH(__FILE__), __LINE__, e.what());
        return 1;
    }

    if (configFile.empty())
    {
        spdlog::warn("{}:{} no config file", LOG_FILE_PATH(__FILE__), __LINE__);
        return 0;
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
    if (!obj)
    {
        spdlog::warn("{}:{} input is nullptr", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    try
    {
        YAML::Node config = YAML::LoadFile(path);

        obj->dbPath = config["db path"].as<std::string>();
        obj->logPath = config["log path"].as<std::string>();

        Model::DAO::DirUtils::convertPath(obj->logPath);
        if (Model::DAO::DirUtils::verifyDir(obj->logPath))
        {
            spdlog::error("{}:{} fail to verify log path", LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        obj->listenPort = config["port"].as<u16>();
        obj->listenIP = config["ip"].as<std::string>();

        if (Model::Utils::verifyIP(obj->listenIP))
        {
            spdlog::error("{}:{} Invalid ip", LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        u8 level(0);
        level = config["log level"].as<u8>();
        obj->logLevel = static_cast<spdlog::level::level_enum>(level);
    }
    catch (...)
    {
        spdlog::error("{}:{} error caught", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return 0;
}

// private member functions
void Config::printVersion()
{
    fmt::println("FFSERVER version info:");
    fmt::println("branch:  " FF_BRANCH);
    fmt::println("commit:  " FF_COMMIT);
    fmt::println("version: " FF_VERSION);
}

} // end namespace GRPCServer

} // end namespace Model
