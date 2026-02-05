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

#include "controller/global/global.hpp"
#include "model/errmsg.hpp"

#include "init.hpp"

#include "queueimpl.hpp"

namespace Controller
{

namespace GRPCServer
{

grpc::Status
QueueImpl::ListPending(grpc::ServerContext *ctx,
                       const ff::QueueReq *req,
                       grpc::ServerWriter<ff::ListTaskRes> *writer)
{
    spdlog::debug("{}:{} QueueImpl::ListPending", LOG_FILE_PATH(__FILE__), __LINE__);
    
    UNUSED(ctx);
    if (!req || !writer)
    {
        spdlog::error("{}:{} invalid input", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    auto queue = sqliteQueueList->getQueue(req->name());
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND,
            "Fail to get queue");
    }

    std::vector<int> out;
    u8 code = queue->listPending(out);
    if (code)
    {
        sqliteQueueList->returnQueue(queue);
        spdlog::error("{}:{} Fail to list pending",
            LOG_FILE_PATH(__FILE__), __LINE__); 
        return Model::ErrMsg::toGRPCStatus(code, "Fail to list pending");
    }

    ff::ListTaskRes res;
    for (auto it = out.begin(); it != out.end(); ++it)
    {
        res.set_id(*it);
        writer->Write(res);
    }

    sqliteQueueList->returnQueue(queue);
    return grpc::Status::OK;
}

grpc::Status
QueueImpl::ListFinished(grpc::ServerContext *ctx,
                        const ff::QueueReq *req,
                        grpc::ServerWriter<ff::ListTaskRes> *writer)
{
    spdlog::debug("{}:{} QueueImpl::ListFinished",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    UNUSED(ctx);
    if (!req || !writer)
    {
        spdlog::error("{}:{} Invalid input", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    auto queue = sqliteQueueList->getQueue(req->name());
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Fail to get queue");
    }

    std::vector<int> out;
    u8 code = queue->listFinished(out);
    if (code)
    {
        sqliteQueueList->returnQueue(queue);
        spdlog::error("{}:{} Fail to list finished",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return Model::ErrMsg::toGRPCStatus(code, "Fail to list finished");
    }

    ff::ListTaskRes res;
    for (auto it = out.begin(); it != out.end(); ++it)
    {
        res.set_id(*it);
        writer->Write(res);
    }

    sqliteQueueList->returnQueue(queue);
    return grpc::Status::OK;
}

static void
buildTaskDetailsRes(Model::Proc::Task &task, ff::TaskDetailsRes *res)
{
    spdlog::debug("{}:{} buildTaskDetailsRes", LOG_FILE_PATH(__FILE__), __LINE__);
    if (!res)
    {
        spdlog::error("{}:{} invalid input", LOG_FILE_PATH(__FILE__), __LINE__);
        return;
    }

    res->set_workdir(task.workDir);
    res->set_execname(task.execName);
    for (auto it = task.args.begin(); it != task.args.end(); ++it)
    {
        res->add_args(*it);
    }

    res->set_exitcode(task.exitCode);
    res->set_id(task.ID);
}

grpc::Status
QueueImpl::PendingDetails(grpc::ServerContext *ctx,
                          const ff::TaskDetailsReq *req,
                          ff::TaskDetailsRes *res)
{
    spdlog::debug("{}:{} QueueImpl::PendingDetails",
        LOG_FILE_PATH(__FILE__), __LINE__);

    UNUSED(ctx);
    if (!req || !res)
    {
        spdlog::error("{}:{} invalid input", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    auto queue = sqliteQueueList->getQueue(req->name());
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND,
            "Fail to get queue");
    }

    Model::Proc::Task out;
    u8 code = queue->pendingDetails(req->id(), out);
    if (code)
    {
        spdlog::error("{}:{} Fail to get pending detailes",
            LOG_FILE_PATH(__FILE__), __LINE__);
        sqliteQueueList->returnQueue(queue);
        return Model::ErrMsg::toGRPCStatus(code, "Fail to get pending detailes");
    }

    buildTaskDetailsRes(out, res);
    sqliteQueueList->returnQueue(queue);
    return grpc::Status::OK;
}

grpc::Status
QueueImpl::FinishedDetails(grpc::ServerContext *ctx,
                           const ff::TaskDetailsReq *req,
                           ff::TaskDetailsRes *res)
{
    spdlog::debug("{}:{} QueueImpl::FinishedDetails",
        LOG_FILE_PATH(__FILE__), __LINE__);

    UNUSED(ctx);
    if (!req || !res)
    {
        spdlog::error("{}:{} invalid input", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    auto queue = sqliteQueueList->getQueue(req->name());
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND,
            "Fail to get queue");
    }

    Model::Proc::Task out;
    u8 code = queue->finishedDetails(req->id(), out);
    if (code)
    {
        sqliteQueueList->returnQueue(queue);
        spdlog::error("{}:{} Fail to get pending detailes",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return Model::ErrMsg::toGRPCStatus(code, "Fail to get pending detailes");
    }

    buildTaskDetailsRes(out, res);
    sqliteQueueList->returnQueue(queue);
    return grpc::Status::OK;
}

grpc::Status
QueueImpl::ClearPending(grpc::ServerContext *ctx,
                        const ff::QueueReq *req,
                        ff::Empty *res)
{
    spdlog::debug("{}:{} QueueImpl::ClearPending", LOG_FILE_PATH(__FILE__), __LINE__);

    UNUSED(ctx);
    UNUSED(res);
    if (!req)
    {
        spdlog::error("{}:{} invalid input", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    auto queue = sqliteQueueList->getQueue(req->name());
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Fail to get queue");
    }

    u8 code = queue->clearPending();
    if (code)
    {
        spdlog::error("{}:{} Fail to clean pending", LOG_FILE_PATH(__FILE__), __LINE__);
        sqliteQueueList->returnQueue(queue);
        return Model::ErrMsg::toGRPCStatus(code, "Fail to clean pending");
    }

    sqliteQueueList->returnQueue(queue);
    return grpc::Status::OK;
}

grpc::Status
QueueImpl::ClearFinished(grpc::ServerContext *ctx,
                         const ff::QueueReq *req,
                         ff::Empty *res)
{
    spdlog::debug("{}:{} QueueImpl::ClearFinished",
        LOG_FILE_PATH(__FILE__), __LINE__);

    UNUSED(ctx);
    UNUSED(res);
    if (!req)
    {
        spdlog::error("{}:{} invalid input", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    auto queue = sqliteQueueList->getQueue(req->name());
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND,
            "Fail to get queue");
    }

    u8 code = queue->clearFinished();
    if (code)
    {
        spdlog::error("{}:{} Fail to clean finished",
            LOG_FILE_PATH(__FILE__), __LINE__);
        sqliteQueueList->returnQueue(queue);
        return Model::ErrMsg::toGRPCStatus(code, "Fail to clean finished");
    }

    sqliteQueueList->returnQueue(queue);
    return grpc::Status::OK;
}

grpc::Status
QueueImpl::CurrentTask(grpc::ServerContext *ctx,
                       const ff::QueueReq *req,
                       ff::TaskDetailsRes *res)
{
    spdlog::debug("{}:{} QueueImpl::CurrentTask", LOG_FILE_PATH(__FILE__), __LINE__);
    UNUSED(ctx);
    if (!req || !res)
    {
        spdlog::error("{}:{} invalid input", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    auto queue = sqliteQueueList->getQueue(req->name());
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND,
            "Fail to get queue");
    }

    Model::Proc::Task out;
    u8 code = queue->currentTask(out);
    if (code)
    {
        spdlog::error("{}:{} Fail to get current task",
            LOG_FILE_PATH(__FILE__), __LINE__);
        sqliteQueueList->returnQueue(queue);
        return Model::ErrMsg::toGRPCStatus(code, "Fail to get current task");
    }

    buildTaskDetailsRes(out, res);
    sqliteQueueList->returnQueue(queue);
    return grpc::Status::OK;
}

grpc::Status
QueueImpl::AddTask(grpc::ServerContext *ctx,
                   const ff::AddTaskReq *req,
                   ff::ListTaskRes *res)
{
    spdlog::debug("{}:{} QueueImpl::AddTask", LOG_FILE_PATH(__FILE__), __LINE__);
    UNUSED(ctx);
    if (!req || !res)
    {
        spdlog::error("{}:{} invalid input", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    auto queue = sqliteQueueList->getQueue(req->name());
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Fail to get queue");
    }

    Model::Proc::Task in;
    in.execName = req->execname();
    in.workDir = req->workdir();
    in.args.reserve(req->args_size());
    for (auto it = req->args().begin(); it != req->args().end(); ++it)
    {
        in.args.push_back(*it);
    }

    u8 code = queue->addTask(in);
    if (code)
    {
        spdlog::error("{}:{} Fail to add task", LOG_FILE_PATH(__FILE__), __LINE__);
        sqliteQueueList->returnQueue(queue);
        return Model::ErrMsg::toGRPCStatus(code, "Fail to add task");
    }

    res->set_id(in.ID);
    sqliteQueueList->returnQueue(queue);
    return grpc::Status::OK;
}

grpc::Status
QueueImpl::RemoveTask(grpc::ServerContext *ctx,
                      const ff::TaskDetailsReq *req,
                      ff::Empty *res)
{
    spdlog::debug("{}:{} QueueImpl::RemoveTask", LOG_FILE_PATH(__FILE__), __LINE__);

    UNUSED(ctx);
    UNUSED(res);
    if (!req)
    {
        spdlog::error("{}:{} invalid input", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    auto queue = sqliteQueueList->getQueue(req->name());
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Fail to get queue");
    }

    u8 code = queue->removeTask(req->id());
    if (code)
    {
        spdlog::error("{}:{} Fail to remove task", LOG_FILE_PATH(__FILE__), __LINE__);
        sqliteQueueList->returnQueue(queue);
        return Model::ErrMsg::toGRPCStatus(code, "Fail to remove task");
    }

    sqliteQueueList->returnQueue(queue);
    return grpc::Status::OK;
}

grpc::Status
QueueImpl::IsRunning(grpc::ServerContext *ctx,
                     const ff::QueueReq *req,
                     ff::IsRunningRes *res)
{
    spdlog::debug("{}:{} QueueImpl::IsRunning", LOG_FILE_PATH(__FILE__), __LINE__);

    UNUSED(ctx);
    if (!req || !res)
    {
        spdlog::error("{}:{} invalid input", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    auto queue = sqliteQueueList->getQueue(req->name());
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Fail to get queue");
    }

    res->set_isrunning(queue->isRunning());
    sqliteQueueList->returnQueue(queue);
    return grpc::Status::OK;
}

grpc::Status
QueueImpl::ReadCurrentOutput(grpc::ServerContext *ctx,
                             const ff::QueueReq *req,
                             grpc::ServerWriter<ff::Msg> *writer)
{
    spdlog::debug("{}:{} QueueImpl::ReadCurrentOutput",
        LOG_FILE_PATH(__FILE__), __LINE__);

    UNUSED(ctx);
    if (!req || !writer)
    {
        spdlog::error("{}:{} invalid input", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    auto queue = sqliteQueueList->getQueue(req->name());
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Fail to get queue");
    }

    std::vector<std::string> output;
    queue->readCurrentOutput(output);

    ff::Msg res;
    for (auto it = output.begin(); it != output.end(); ++it)
    {
        res.set_msg(*it);
        writer->Write(res);
    }

    sqliteQueueList->returnQueue(queue);
    return grpc::Status::OK;
}

grpc::Status
QueueImpl::Start(grpc::ServerContext *ctx,
                const ff::QueueReq *req,
                ff::Empty *res)
{
    spdlog::debug("{}:{} QueueImpl::Start", LOG_FILE_PATH(__FILE__), __LINE__);

    UNUSED(ctx);
    UNUSED(res);
    if (!req)
    {
        spdlog::error("{}:{} invalid input", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    auto queue = sqliteQueueList->getQueue(req->name());
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Fail to get queue");
    }

    u8 code = queue->start();
    if (code)
    {
        spdlog::error("{}:{} Fail to start queue", LOG_FILE_PATH(__FILE__), __LINE__);
        sqliteQueueList->returnQueue(queue);
        return Model::ErrMsg::toGRPCStatus(code, "Fail to start queue");
    }

    sqliteQueueList->returnQueue(queue);
    return grpc::Status::OK;
}

grpc::Status
QueueImpl::Stop(grpc::ServerContext *ctx,
                const ff::QueueReq *req,
                ff::Empty *res)
{
    spdlog::debug("{}:{} QueueImpl::Stop", LOG_FILE_PATH(__FILE__), __LINE__);

    UNUSED(ctx);
    UNUSED(res);
    if (!req)
    {
        spdlog::error("{}:{} invalid input", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL, "Invalid input");
    }

    auto queue = sqliteQueueList->getQueue(req->name());
    if (!queue)
    {
        spdlog::error("{}:{} Fail to get queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Fail to get queue");
    }

    queue->stop();
    sqliteQueueList->returnQueue(queue);
    return grpc::Status::OK;
}

} // end namespace GRPCServer

} // end namespace Controller
