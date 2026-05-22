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

#include "model/proc/task.hpp"
#include "spdlog/spdlog.h"

#include "model/utils.hpp"

#include "queue.hpp"

namespace Controller
{

namespace CLI
{

namespace Queue
{

int listPending(std::shared_ptr<Model::DAO::IQueue> &queue)
{
    spdlog::debug("{}:{} listPending", LOG_FILE_PATH(__FILE__), __LINE__);

    std::vector<int> list;
    if (queue->listPending(list))
    {
        spdlog::error("{}:{} Fail to list pending tasks",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    for (const auto &item : list)
    {
        std::cout << item << std::endl;
    }

    return 0;
}

int listFinished(std::shared_ptr<Model::DAO::IQueue> &queue)
{
    spdlog::debug("{}:{} listFinished", LOG_FILE_PATH(__FILE__), __LINE__);

    std::vector<int> list;
    if (queue->listFinished(list))
    {
        spdlog::error("{}:{} Fail to list finished tasks",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    for (const auto &item : list)
    {
        std::cout << item << std::endl;
    }

    return 0;
}

int pendingDetails(const ::CLI::App *app, std::shared_ptr<Model::DAO::IQueue> &queue)
{
    spdlog::debug("{}:{} pendingDetails", LOG_FILE_PATH(__FILE__), __LINE__);

    auto id = app->get_option("-i")->as<int>();
    if (id < 0)
    {
        spdlog::error("{}:{} id is invalid", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    Model::Proc::Task task;
    if (queue->pendingDetails(id, task))
    {
        spdlog::error("{}:{} Fail to get pending task details",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    Model::Proc::printTask(task);
    return 0;
}

int finishedDetails(const ::CLI::App *app, std::shared_ptr<Model::DAO::IQueue> &queue)
{
    spdlog::debug("{}:{} finishedDetails", LOG_FILE_PATH(__FILE__), __LINE__);

    auto id = app->get_option("-i")->as<int>();
    if (id < 0)
    {
        spdlog::error("{}:{} id is invalid", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    Model::Proc::Task task;
    if (queue->finishedDetails(id, task))
    {
        spdlog::error("{}:{} Fail to get finished task details",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    Model::Proc::printTask(task);
    return 0;
}

int currentTask(std::shared_ptr<Model::DAO::IQueue> &queue)
{
    spdlog::debug("{}:{} currentTask", LOG_FILE_PATH(__FILE__), __LINE__);

    Model::Proc::Task task;
    if (queue->currentTask(task))
    {
        spdlog::error("{}:{} Fail to get current task",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    Model::Proc::printTask(task);
    return 0;
}

// split string
static std::vector<std::string> split(const std::string &s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

int addTask(const ::CLI::App *app, std::shared_ptr<Model::DAO::IQueue> &queue)
{
    spdlog::debug("{}:{} addTask", LOG_FILE_PATH(__FILE__), __LINE__);

    auto execName = app->get_option("-e")->as<std::string>();
    auto args = app->get_option("-a")->as<std::string>();
    auto workDir = app->get_option("-w")->as<std::string>();

    if (execName.empty() || args.empty() || workDir.empty())
    {
        spdlog::error("{}:{} execName, args or workDir is empty",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    Model::Proc::Task task;
    task.execName = execName;
    task.args = split(args, ' ');
    task.workDir = workDir;

    if (queue->addTask(task))
    {
        spdlog::error("{}:{} Fail to add task",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return 0;
}

int removeTask(const ::CLI::App *app, std::shared_ptr<Model::DAO::IQueue> &queue)
{
    spdlog::debug("{}:{} removeTask", LOG_FILE_PATH(__FILE__), __LINE__);

    auto id = app->get_option("-i")->as<int>();
    if (id < 0)
    {
        spdlog::error("{}:{} id is invalid", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (queue->removeTask(id))
    {
        spdlog::error("{}:{} Fail to remove task",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return 0;
}

int readCurrentOutput(std::shared_ptr<Model::DAO::IQueue> &queue)
{
    spdlog::debug("{}:{} readCurrentOutput", LOG_FILE_PATH(__FILE__), __LINE__);

    std::vector<std::string> output;
    queue->readCurrentOutput(output);

    for (const auto &item : output)
    {
        std::cout << item << std::endl;
    }

    return 0;
}

} // end namespace Queue

} // end namespace CLI

} // end namespace Controller
