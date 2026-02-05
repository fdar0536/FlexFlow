/*
 * Simple Task Queue
 * Copyright (c) 2023-2024 fdar0536
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

#include "model/errmsg.hpp"
#include "model/dao/sqliteconnect.hpp"

#include "controller/global/global.hpp"

#include "init.hpp"

namespace Controller
{

namespace GRPCServer
{

Config config;

GRPCServer::Server server;

Model::DAO::IQueueList *sqliteQueueList = nullptr;

u8 init(int argc, char **argv)
{
    spdlog::debug("{}:{} init", LOG_FILE_PATH(__FILE__), __LINE__);

    if (Global::consoleInit())
    {
        spdlog::error("{}:{} initConsole failed", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    u8 ret(Config::parse(&config, argc, argv));
    if (ret)
    {
        if (ret == 2)
        {
            return 2;
        }

        spdlog::error("{}:{} parse config failed",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (Controller::Global::sqliteInit(&sqliteQueueList, config.dbPath))
    {
        spdlog::error("{}:{} Fail to initialize sqlite queue list",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    Model::ErrMsg::init();
    if (config.logPath.empty())
    {
        ret = Global::spdlogInit(config.logPath);
    }
    else
    {
        ret = Global::spdlogInit(config.logPath + "/STQLog.log");
    }

    if (ret)
    {
        spdlog::error("{}:{} Fail to initialize spdlog",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    spdlog::set_level(static_cast<spdlog::level::level_enum>(config.logLevel));
    return 0;
}

void fin()
{
    spdlog::debug("{}:{} fin", LOG_FILE_PATH(__FILE__), __LINE__);
    Global::consoleFin();
    if (sqliteQueueList) delete sqliteQueueList;
}

} // end namespace GRPCServer

} // end namespace Model
