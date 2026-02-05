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

#include "queuelistimpl.hpp"

namespace Controller
{

namespace GRPCServer
{

grpc::Status
QueueListImpl::Create(grpc::ServerContext *ctx,
                      const ff::QueueReq *req,
                      ff::Empty *res)
{
    spdlog::debug("{}:{} QueueListImpl::Create", LOG_FILE_PATH(__FILE__), __LINE__);

    UNUSED(ctx);
    UNUSED(res);

    if (!req)
    {
        spdlog::error("{}:{} invalid input", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Internal server error");
    }

    if (req->name().empty())
    {
        spdlog::error("{}:{} \"name\" is empty string",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                            "\"name\" is empty string");
    }

    u8 code = sqliteQueueList->createQueue(req->name());
    if (code)
    {
        spdlog::error("{}:{} Fail to create queue",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return Model::ErrMsg::toGRPCStatus(code, "Fail to create queue");
    }

    return grpc::Status::OK;
}

grpc::Status
QueueListImpl::Rename(grpc::ServerContext *ctx,
                      const ff::RenameQueueReq *req,
                      ff::Empty *res)
{
    spdlog::debug("{}:{} QueueListImpl::Rename", LOG_FILE_PATH(__FILE__), __LINE__);

    UNUSED(ctx);
    UNUSED(res);

    if (!req)
    {
        spdlog::error("{}:{} invalid input", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Internal server error");
    }

    if (req->oldname().empty() || req->newname().empty())
    {
        spdlog::error("{}:{} \"oldName\" or \"newName\" is empty string", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                            "\"oldName\" or \"newName\" is empty string");
    }

    u8 code = sqliteQueueList->renameQueue(req->oldname(), req->newname());
    if (code)
    {
        spdlog::error("{}:{} Fail to rename", LOG_FILE_PATH(__FILE__), __LINE__);
        return Model::ErrMsg::toGRPCStatus(code, "Fail to rename");
    }

    return grpc::Status::OK;
}

grpc::Status
QueueListImpl::Delete(grpc::ServerContext *ctx,
                      const ff::QueueReq *req,
                      ff::Empty *res)
{
    spdlog::debug("{}:{} QueueListImpl::Delete", LOG_FILE_PATH(__FILE__), __LINE__);

    UNUSED(ctx);
    UNUSED(res);

    if (!req)
    {
        spdlog::error("{}:{} invalid input", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Internal server error");
    }

    if (req->name().empty())
    {
        spdlog::error("{}:{} \"name\" is empty string",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                            "\"name\" is empty string");
    }

    u8 code = sqliteQueueList->deleteQueue(req->name());
    if (code)
    {
        spdlog::error("{}:{} Fail to delete", LOG_FILE_PATH(__FILE__), __LINE__);
        return Model::ErrMsg::toGRPCStatus(code, "Fail to delete");
    }

    return grpc::Status::OK;
}

grpc::Status
QueueListImpl::List(grpc::ServerContext *ctx,
                    const ff::Empty *req,
                    grpc::ServerWriter<::ff::ListQueueRes> *writer)
{
    spdlog::debug("{}:{} QueueListImpl::List", LOG_FILE_PATH(__FILE__), __LINE__);

    UNUSED(ctx);
    UNUSED(req);

    if (!writer)
    {
        spdlog::error("{}:{} invalid input", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Internal server error");
    }

    std::vector<std::string> out;
    u8 code = sqliteQueueList->listQueue(out);
    if (code)
    {
        spdlog::error("{}:{} Fail to list queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return Model::ErrMsg::toGRPCStatus(code, "Fail to list queue");
    }

    ff::ListQueueRes toWrite;
    for (size_t i = 0; i < out.size(); ++i)
    {
        toWrite.set_name(out.at(i));
        writer->Write(toWrite);
    }

    return grpc::Status::OK;
}

grpc::Status
QueueListImpl::GetQueue(grpc::ServerContext *ctx,
                        const ff::QueueReq *req,
                        ff::Empty *res)
{
    spdlog::debug("{}:{} QueueListImpl::GetQueue",
        LOG_FILE_PATH(__FILE__), __LINE__);

    UNUSED(ctx);
    UNUSED(res);
    if (!req)
    {
        spdlog::error("{}:{} invalid input", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Internal server error");
    }

    auto queue = sqliteQueueList->getQueue(req->name());
    if (queue == nullptr)
    {
        spdlog::error("{}:{} No such queue", LOG_FILE_PATH(__FILE__), __LINE__);
        return grpc::Status(grpc::StatusCode::NOT_FOUND,
                            "No such queue");
    }

    sqliteQueueList->returnQueue(queue);
    return grpc::Status::OK;
}

} // end namespace GRPCServer

} // end namespace Controller
