/*
 * Flex Flow
 * Copyright (c) 2025-present fdar0536
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

#include "controller/global/global.hpp"
#include "handlemanager.hpp"
#include "iqueue.hpp"

#include "queue.h"

using namespace Model::DAO;

static u8 taskToProcTask(const Model::Proc::Task &in, ProcTask *out)
{
    spdlog::debug("{}:{} taskToProcTask", LOG_FILE_PATH(__FILE__), __LINE__);

    if (!out)
    {
        spdlog::error("{}:{} out is nullptr", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    memset(out, 0, sizeof(ProcTask));

    (*out).execName = (char *)calloc(in.execName.size() + 1, sizeof(char));
    if (!(*out).execName)
    {
        spdlog::error("{}:{} Fail to calloc", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    snprintf((*out).execName,
    in.execName.size() + 1, "%s", in.execName.c_str());

    (*out).argc = in.args.size();
    if (in.args.size())
    {
        (*out).argv = (char **)calloc(in.args.size(), sizeof(char *));
        if (!(*out).argv)
        {
            spdlog::error("{}:{} Fail to calloc", LOG_FILE_PATH(__FILE__), __LINE__);
            queue_destroyProcTask(out);
            return 1;
        }

        for (size_t i = 0; i < in.args.size(); ++i)
        {
            (*out).argv[i] = (char *)calloc(in.args.at(i).size() + 1,
                                             sizeof(char));
            if (!(*out).argv[i])
            {
                spdlog::error("{}:{} Fail to calloc",
                    LOG_FILE_PATH(__FILE__), __LINE__);
                queue_destroyProcTask(out);
                return 1;
            }

            snprintf((*out).argv[i], in.args.at(i).size() + 1, "%s",
                     in.args.at(i).c_str());
        }
    }

    if (!in.workDir.size())
    {
        (*out).workDir = (char *)calloc(in.workDir.size() + 1, sizeof(char));
        if (!(*out).workDir)
        {
            spdlog::error("{}:{} Fail to calloc", LOG_FILE_PATH(__FILE__), __LINE__);
            queue_destroyProcTask(out);
            return 1;
        }

        snprintf((*out).workDir, in.workDir.size() + 1, "%s",
                 in.workDir.c_str());
    }

    (*out).ID = in.ID;
    (*out).exitCode = in.exitCode;
    (*out).isSuccess = in.isSuccess;

    return 0;
}

static IQueue *getQueue(Handle h)
{
    Parent parent = hm.parent(h);
    if (parent != Parent::IQueue)
    {
        spdlog::error("{}:{} parent is not IQueue",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return nullptr;
    }

    IQueue *queue = hm.get<IQueue>(h);
    if (!queue)
    {
        spdlog::error("{}:{} queue is nullptr",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return nullptr;
    }

    return queue;
}

extern "C"
{

// note that this function only destroy the content of ProcTask
// does not free ProcTask
void queue_destroyProcTask(ProcTask *in)
{
    spdlog::debug("{}:{} queue_destroyProcTask",
        LOG_FILE_PATH(__FILE__), __LINE__);

    if (!in) return;

    if (in->execName)
    {
        free(in->execName);
    }

    if (in->argc)
    {
        if (in->argv)
        {
            for (size_t i = 0; i < in->argc; ++i)
            {
                // for safety
                if (in->argv[i]) free(in->argv[i]);
            }

            free(in->argv);
        }
    }

    if (in->workDir)
    {
        free(in->workDir);
    }

    memset(in, 0, sizeof(ProcTask));
}

u8 queue_listPending(Handle h, int **out, size_t *outSize)
{
    spdlog::debug("{}:{} queue_listPending", LOG_FILE_PATH(__FILE__), __LINE__);

    if (!out || !outSize)
    {
        spdlog::error("{}:{} out or outSize is nullptr",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    IQueue *queue = getQueue(h);
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    std::vector<int> output;
    if (queue->listPending(output))
    {
        spdlog::error("{}:{} Fail to list pending",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (!output.size())
    {
        *outSize = 0;
        return 0;
    }

    *outSize = output.size();
    *out = (int *)calloc(*outSize, sizeof(int));
    if (!(*out))
    {
        spdlog::error("{}:{} Fail to calloc", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    std::copy(output.begin(), output.end(), (*out));
    return 0;
}

u8 queue_listFinished(Handle h, int **out, size_t *outSize)
{
    spdlog::debug("{}:{} queue_listFinished", LOG_FILE_PATH(__FILE__), __LINE__);

    if (!out || !outSize)
    {
        spdlog::error("{}:{} out or outSize is nullptr",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    IQueue *queue = getQueue(h);
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    std::vector<int> output;
    if (queue->listFinished(output))
    {
        spdlog::error("{}:{} Fail to list finished",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (!output.size())
    {
        *outSize = 0;
        return 0;
    }

    *outSize = output.size();
    *out = (int *)calloc(*outSize, sizeof(int));
    if (!(*out))
    {
        spdlog::error("{}:{} Fail to calloc", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    std::copy(output.begin(), output.end(), (*out));
    return 0;
}

u8 queue_pendingDetails(Handle h, const int id, ProcTask *out)
{
    spdlog::debug("{}:{} queue_pendingDetails", LOG_FILE_PATH(__FILE__), __LINE__);

    if (!out)
    {
        spdlog::error("{}:{} out is nullptr", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    IQueue *queue = getQueue(h);
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    Model::Proc::Task task;
    if (queue->pendingDetails(id, task))
    {
        spdlog::error("{}:{} Fail to get pending details",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (taskToProcTask(task, out))
    {
        spdlog::error("{}:{} Fail to convert task to ProcTask",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return 0;
}

u8 queue_finishedDetails(Handle h, const int id, ProcTask *out)
{
    spdlog::debug("{}:{} queue_finishedDetails", LOG_FILE_PATH(__FILE__), __LINE__);

    if (!out)
    {
        spdlog::error("{}:{} out is nullptr", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    IQueue *queue = getQueue(h);
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    Model::Proc::Task task;
    if (queue->finishedDetails(id, task))
    {
        spdlog::error("{}:{} Fail to get finished details",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (taskToProcTask(task, out))
    {
        spdlog::error("{}:{} Fail to convert task to ProcTask",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return 0;
}

u8 queue_clearPending(Handle h)
{
    spdlog::debug("{}:{} queue_clearPending", LOG_FILE_PATH(__FILE__), __LINE__);

    IQueue *queue = getQueue(h);
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return queue->clearPending();
}

u8 queue_clearFinished(Handle h)
{
    spdlog::debug("{}:{} queue_clearFinished", LOG_FILE_PATH(__FILE__), __LINE__);

    IQueue *queue = getQueue(h);
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return queue->clearFinished();
}

u8 queue_currentTask(Handle h, ProcTask *out)
{
    spdlog::debug("{}:{} queue_currentTask", LOG_FILE_PATH(__FILE__), __LINE__);

    if (!out) return 1;
    IQueue *queue = getQueue(h);
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    Model::Proc::Task task;
    if (queue->currentTask(task))
    {
        spdlog::error("{}:{} Fail to get current task",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (taskToProcTask(task, out))
    {
        spdlog::error("{}:{} Fail to convert task to ProcTask",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return 0;
}

u8 queue_addTask(Handle h, const ProcTask *in)
{
    spdlog::debug("{}:{} queue_addTask", LOG_FILE_PATH(__FILE__), __LINE__);

    if (!in)
    {
        spdlog::error("{}:{} in is nullptr", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    IQueue *queue = getQueue(h);
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    Model::Proc::Task task;

    if (!in->execName) return 1;
    task.execName = std::string(in->execName);

    task.args.reserve(in->argc);
    if (in->argc)
    {
        if (!in->argv)
        {
            spdlog::error("{}:{} in->argv is nullptr",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        for (size_t i = 0; i < in->argc; ++i)
        {
            if (!in->argv[i])
            {
                spdlog::error("{}:{} in->argv[{}] is nullptr",
                    LOG_FILE_PATH(__FILE__), __LINE__, i);
                return 1;
            }

            task.args.push_back(std::string(in->argv[i]));
        }
    }

    if (in->workDir)
    {
        task.workDir = std::string(in->workDir);
    }

    task.ID = in->ID;
    task.exitCode = in->exitCode;
    task.isSuccess = in->isSuccess;

    return queue->addTask(task);
}

u8 queue_removeTask(Handle h, const i32 in)
{
    spdlog::debug("{}:{} queue_removeTask", LOG_FILE_PATH(__FILE__), __LINE__);

    IQueue *queue = getQueue(h);
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return queue->removeTask(in);
}

u8 queue_isRunning(Handle h)
{
    spdlog::debug("{}:{} queue_isRunning", LOG_FILE_PATH(__FILE__), __LINE__);

    IQueue *queue = getQueue(h);
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return static_cast<u8>(queue->isRunning());
}

u8 queue_readCurrentOutput(Handle h, char ***out, size_t *outSize)
{
    spdlog::debug("{}:{} queue_readCurrentOutput",
        LOG_FILE_PATH(__FILE__), __LINE__);


    if (!out || !outSize) return 1;
    IQueue *queue = getQueue(h);
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    std::vector<std::string> output;
    queue->readCurrentOutput(output);

    *outSize = output.size();
    if (!(*outSize))
    {
        return 0;
    }

    *out = (char **)calloc(*outSize, sizeof(char *));
    if (!(*out))
    {
        spdlog::error("{}:{} Fail to calloc", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    for (size_t i = 0; i < output.size(); ++i)
    {
        *out[i] = (char *)calloc(output.at(i).size() + 1, sizeof(char));
        if (!(*out[i]))
        {
            spdlog::error("{}:{} Fail to calloc", LOG_FILE_PATH(__FILE__), __LINE__);

            for (size_t j = 1; j <= i; ++j)
            {
                free(*out[j]);
            }

            if (*out[0]) free(*out[0]);
            free(*out);
            *out = NULL;
            return 1;
        }

        snprintf(*out[i], output.at(i).size() + 1, "%s", output.at(i).c_str());
    }

    return 0;
}

u8 queue_start(Handle h)
{
    spdlog::debug("{}:{} queue_start", LOG_FILE_PATH(__FILE__), __LINE__);

    IQueue *queue = getQueue(h);
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return queue->start();
}

u8 queue_stop(Handle h)
{
    spdlog::debug("{}:{} queue_stop", LOG_FILE_PATH(__FILE__), __LINE__);

    IQueue *queue = getQueue(h);
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    queue->stop();
    return 0;
}

} // extern "C"
