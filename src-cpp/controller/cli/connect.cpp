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

#include <iostream>

#include <string>

#ifdef _WIN32
#include "windows.h"
#else
#include "termios.h"
#include "unistd.h"
#endif

#include "spdlog/spdlog.h"
#include "yaml-cpp/yaml.h"

#include "model/connect/grpc/connect.hpp"
#include "model/dao/grpc/queuelist.hpp"
#include "model/defines.h"
#include "model/utils.hpp"

#include "connect.hpp"

namespace Controller
{

namespace CLI
{

Profile profile;

std::shared_ptr<Model::DAO::IQueueList> queueList = nullptr;

static u8 connect();

static void setEcho(bool on);

u8 connect(const std::string &file)
{
    spdlog::debug("{}:{} parseProfile", LOG_FILE_PATH(__FILE__), __LINE__);
    spdlog::debug("{}:{} file is: {}", LOG_FILE_PATH(__FILE__), __LINE__, file);

    if (file.empty())
    {
        spdlog::error("{}:{} file is empty", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    YAML::Node config = YAML::LoadFile(file);
    if (config.IsNull())
    {
        spdlog::error("{}:{} Fail to load file", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    try
    {
        profile.ssl = config["ssl"].as<bool>();
        profile.host = config["host"].as<std::string>();
        profile.port = config["port"].as<u16>();
        profile.username = config["username"].as<std::string>();
        profile.password = config["password"].as<std::string>();
    }
    catch (...)
    {
        spdlog::error("{}:{} Fail to parse profile", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return connect();
}

static u8 connect()
{
    spdlog::debug("{}:{} connect", LOG_FILE_PATH(__FILE__), __LINE__);

    std::cout << "Enter your TOTP code (no echo): ";
    setEcho(false);
    std::string code;
    std::cin >> code;
    setEcho(true);
    std::cout << std::endl;

    auto token = Model::Connect::GRPC::connect(
        profile.ssl,
        profile.host,
        profile.port,
        profile.username,
        profile.password,
        code);
    
    if (token == nullptr)
    {
        spdlog::error("{}:{} Fail to connect", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    auto list = new (std::nothrow) Model::DAO::GRPC::QueueList;
    if (!list)
    {
        spdlog::error("{}:{} Fail to allocate memory", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (list->init(token))
    {
        spdlog::error("{}:{} Fail to initialize queue list", LOG_FILE_PATH(__FILE__), __LINE__);
        delete list;
        return 1;
    }

    queueList = std::shared_ptr<Model::DAO::IQueueList>(list);
    return 0;
}

static void setEcho(bool on)
{
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;

    // Get current mode
    GetConsoleMode(hStdin, &mode);
    if (on)
        // Enable echo
        mode |= ENABLE_ECHO_INPUT;
    else
        // Disable echo
        mode &= ~ENABLE_ECHO_INPUT; 

    // Apply new mode
    SetConsoleMode(hStdin, mode);
#else
    struct termios t;

    // Get current settings
    tcgetattr(STDIN_FILENO, &t);

    if (on)
        // Turn echo on
        t.c_lflag |= ECHO;
    else
        // Turn echo off
        t.c_lflag &= ~ECHO;      

    // Apply settings immediately
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
#endif
}

} // end namespace CLI

} // end namespace Controller
