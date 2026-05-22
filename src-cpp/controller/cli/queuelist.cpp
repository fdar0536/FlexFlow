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

#include "spdlog/spdlog.h"

#include "model/dao/iqueuelist.hpp"
#include "model/utils.hpp"

#include "connect.hpp"
#include <mutex>
#include "queuelist.hpp"

namespace Controller
{

namespace CLI
{

namespace QueueList
{

int create(const ::CLI::App *app)
{
    spdlog::debug("{}:{} create", LOG_FILE_PATH(__FILE__), __LINE__);

    std::string name = app->get_option("-n")->as<std::string>();
    if (name.empty())
    {
        spdlog::error("{}:{} name is empty", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (Controller::CLI::queueList->create(name))
    {
        spdlog::error("{}:{} Fail to create queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return 0;
}

int list()
{
    spdlog::debug("{}:{} list", LOG_FILE_PATH(__FILE__), __LINE__);

    std::vector<std::string> list;
    if (Controller::CLI::queueList->list(list))
    {
        spdlog::error("{}:{} Fail to list queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    for (const auto &item : list)
    {
        std::cout << item << std::endl;
    }

    return 0;
}

int delete_(const ::CLI::App *app)
{
    spdlog::debug("{}:{} delete", LOG_FILE_PATH(__FILE__), __LINE__);

    auto name = app->get_option("-n")->as<std::string>();
    if (name.empty())
    {
        spdlog::error("{}:{} name is empty", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (Controller::CLI::queueList->delete_(name))
    {
        spdlog::error("{}:{} Fail to delete queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return 0;
}

int rename(const ::CLI::App *app)
{
    spdlog::debug("{}:{} rename", LOG_FILE_PATH(__FILE__), __LINE__);

    auto oldName = app->get_option("-o")->as<std::string>();
    auto newName = app->get_option("-n")->as<std::string>();

    if (oldName.empty() || newName.empty())
    {
        spdlog::error("{}:{} oldName or newName is empty", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (Controller::CLI::queueList->rename(oldName, newName))
    {
        spdlog::error("{}:{} Fail to rename queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return 0;
}

} // end namespace QueueList

} // end namespace CLI

} // end namespace Controller
