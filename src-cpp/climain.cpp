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

#include "CLI/CLI.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h" 

#include "controller/cli/connect.hpp"
#include "controller/cli/queuelist.hpp"
#include "controller/cli/queue.hpp"
#include "model/dao/iqueuelist.hpp"
#include "model/dao/iqueue.hpp"
#include "model/utils.hpp"

#include "config.h"

static std::string profile;

int main(int argc, char **argv)
{
    spdlog::cfg::load_env_levels();

#if !ENABLE_DEBUG
    spdlog::level::level_enum level = spdlog::get_level();
    if (level < spdlog::level::level_enum::info)
    {
        spdlog::set_level(spdlog::level::level_enum::info);
    }
#endif

    // build command line parser
    std::string appName = "Flex Flow ";
    appName.append(FF_VERSION);
    appName.append(" Command Line Interface");

    CLI::App app{appName, argv[0]};

    app.add_option("-p,--profile", profile, "Profile file path");

    // for queue list
    auto createQueue = app.add_subcommand("create", "Create a new queue");
    createQueue->add_option("-n,--name")->required();
    
    auto listQueue = app.add_subcommand("list", "List all queue");
    
    auto deleteQueue = app.add_subcommand("delete", "Delete a queue");
    deleteQueue->add_option("-n,--name")->required();
    
    auto renameQueue = app.add_subcommand("rename", "Rename a queue");
    renameQueue->add_option("-o,--old-name")->required();
    renameQueue->add_option("-n,--new-name")->required();

    // for queue
    auto queue = app.add_subcommand("queue", "Manage a queue");
    queue->add_option("-n,--name")->required();
    
    auto listPending = queue->add_subcommand("list-pending", "List pending tasks");
    
    auto listFinished = queue->add_subcommand("list-finished", "List finished tasks");
    
    auto pendingDetails = queue->add_subcommand("pending-details", "Get pending task details");
    pendingDetails->add_option("-i,--id")->required();
    
    auto finishedDetails = queue->add_subcommand("finished-details", "Get finished task details");
    finishedDetails->add_option("-i,--id")->required();
    
    auto clearPending = queue->add_subcommand("clear-pending", "Clear pending tasks");
    
    auto clearFinished = queue->add_subcommand("clear-finished", "Clear finished tasks");
    
    auto currentTask = queue->add_subcommand("current-task", "Get current task");
    
    auto addTask = queue->add_subcommand("add-task", "Add a task");
    addTask->add_option("-e,--exec-name")->required();
    addTask->add_option("-a,--args")->required();
    addTask->add_option("-w,--work-dir")->required();
    
    auto removeTask = queue->add_subcommand("remove-task", "Remove a task");
    removeTask->add_option("-i,--id")->required();
    
    auto start = queue->add_subcommand("start", "Start the queue");
    
    auto stop = queue->add_subcommand("stop", "Stop the queue");

    auto isRunning = queue->add_subcommand("is-running", "Check if the queue is running");

    auto readCurrentOutput = queue->add_subcommand("read-current-output", "Read current output");

    CLI11_PARSE(app, argc, argv);

    try
    {
        // handle command line arguments
        if (profile.empty())
        {
            spdlog::error("{}:{} profile is empty", LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        // for queue list
        if (Controller::CLI::connect(profile))
        {
            spdlog::error("{}:{} Fail to connect", LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        if (createQueue->parsed())
        {
            return Controller::CLI::QueueList::create(createQueue);
        }

        if (listQueue->parsed())
        {
            return Controller::CLI::QueueList::list();
        }

        if (deleteQueue->parsed())
        {
            return Controller::CLI::QueueList::delete_(deleteQueue);
        }

        if (renameQueue->parsed())
        {
            return Controller::CLI::QueueList::rename(renameQueue);
        }

        // for queue
        std::string queueName;
        std::shared_ptr<Model::DAO::IQueue> queuePtr = nullptr;
        if (queue->parsed())
        {
            queueName = queue->get_option("-n")->as<std::string>();
            if (queueName.empty())
            {
                spdlog::error("{}:{} queueName is empty", LOG_FILE_PATH(__FILE__), __LINE__);
                return 1;
            }
        }
        else
        {
            spdlog::error("{}:{} Could you please tell me what do you want to do?",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        queuePtr = Controller::CLI::queueList->get(queueName);
        if (queuePtr == nullptr)
        {
            spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        if (listPending->parsed())
        {
            return Controller::CLI::Queue::listPending(queuePtr);
        }

        if (listFinished->parsed())
        {
            return Controller::CLI::Queue::listFinished(queuePtr);
        }

        if (pendingDetails->parsed())
        {
            return Controller::CLI::Queue::pendingDetails(pendingDetails, queuePtr);
        }

        if (finishedDetails->parsed())
        {
            return Controller::CLI::Queue::finishedDetails(finishedDetails, queuePtr);
        }

        if (clearPending->parsed())
        {
            if (queuePtr->clearPending())
            {
                spdlog::error("{}:{} Fail to clear pending tasks", LOG_FILE_PATH(__FILE__), __LINE__);
                return 1;
            }

            return 0;
        }

        if (clearFinished->parsed())
        {
            if (queuePtr->clearFinished())
            {
                spdlog::error("{}:{} Fail to clear finished tasks", LOG_FILE_PATH(__FILE__), __LINE__);
                return 1;
            }

            return 0;
        }

        if (currentTask->parsed())
        {
            return Controller::CLI::Queue::currentTask(queuePtr);
        }

        if (addTask->parsed())
        {
            return Controller::CLI::Queue::addTask(addTask, queuePtr);
        }

        if (removeTask->parsed())
        {
            return Controller::CLI::Queue::removeTask(removeTask, queuePtr);
        }

        if (start->parsed())
        {
            if (queuePtr->start())
            {
                spdlog::error("{}:{} Fail to start queue", LOG_FILE_PATH(__FILE__), __LINE__);
                return 1;
            }

            return 0;
        }

        if (stop->parsed())
        {
            queuePtr->stop();
            return 0;
        }

        if (isRunning->parsed())
        {
            if (queuePtr->isRunning())
            {
                std::cout << "true" << std::endl;
            }
            else
            {
                std::cout << "false" << std::endl;
            }

            return 0;
        }

        if (readCurrentOutput->parsed())
        {
            return Controller::CLI::Queue::readCurrentOutput(queuePtr);
        }
    }
    catch (...)
    {
        spdlog::error("{}:{} Fail to parse command line arguments", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }
}
