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

#ifdef _WIN32
#include "direct.h"
#else
#include "unistd.h"
#endif

#include "spdlog/spdlog.h"
#include "CLI/CLI.hpp"
#include "yaml-cpp/yaml.h"

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
    
    CLI::App app("Flex Flow Server");
    app.set_help_flag();
    
    app.add_option("-c,--config-file", configFile, 
        "path to config file")->default_str("");
    app.add_option("-d,--db-path", in->dbPath, 
        "path to config file")->default_str(std::string(buf));
    app.add_option("-l,--log-path", in->logPath, 
        "path to config file")->default_str(std::string(buf));
    app.add_option("-L,--log-level", in->logLevel, 
        "log level for spdlog")->default_val(2);
    app.add_option("-a, --address", in->listenIP, 
        "which address will listen")->default_val("127.0.0.1");
    app.add_option("-p,--port", in->listenPort, 
        "which port will listen")->default_val(12345);
    app.add_flag("-v,--version","print version info");
    app.add_flag("-h,--help","print help info");

    CLI11_PARSE(app, argc, argv);

    if (app.count("-v"))
    {
        printVersion();
        return 2;
    }

    if (app.count("-h"))
    {
        fmt::println("{}", app.help());
        return 2;
    }

    if (!configFile.empty())
    {
        if (Model::Utils::verifyFile(configFile))
        {
            spdlog::error("{}:{} Fail to verify config file", LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }
    }

    if (!in->logPath.empty())
    {
        Model::Utils::convertPath(in->logPath);
        if (Model::Utils::verifyDir(in->logPath))
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
    spdlog::debug("{}:{} Config::parse", LOG_FILE_PATH(__FILE__), __LINE__);

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

        Model::Utils::convertPath(obj->logPath);
        if (Model::Utils::verifyDir(obj->logPath))
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
