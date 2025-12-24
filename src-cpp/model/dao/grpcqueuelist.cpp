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

#include "grpcconnect.hpp"
#include "grpcqueue.hpp"
#include "grpcqueuelist.hpp"
#include "grpcutils.hpp"

namespace Model
{

namespace DAO
{

GRPCQueueList::GRPCQueueList() :
    m_stub(nullptr)
{}

GRPCQueueList::~GRPCQueueList()
{
    if (m_conn) delete m_conn;
}

u8 GRPCQueueList::init(IConnect *connect)
{
    if (!connect)
    {
        spdlog::error("{}:{} connect is nullptr", LOG_FILE_PATH(__FILE__), __LINE__);
        return ErrCode_INVALID_ARGUMENT;
    }

    if (!connect->connectToken())
    {
        spdlog::error("{}:{} connect token is nullptr", LOG_FILE_PATH(__FILE__), __LINE__);
        return ErrCode_INVALID_ARGUMENT;
    }

    GRPCToken *token = reinterpret_cast<GRPCToken *>(connect->connectToken());
    try
    {
        m_stub = ff::QueueList::NewStub(token->channel);
        if (m_stub == nullptr)
        {
            spdlog::error("{}:{} Fail to get stub", LOG_FILE_PATH(__FILE__), __LINE__);
            return ErrCode_OS_ERROR;
        }
    }
    catch (...)
    {
        spdlog::error("{}:{} Fail to get stub", LOG_FILE_PATH(__FILE__), __LINE__);
        return ErrCode_OS_ERROR;
    }

    m_conn = connect;
    return ErrCode_OK;
}

u8 GRPCQueueList::createQueue(const std::string &name)
{
    ff::QueueReq req;
    req.set_name(name);

    ff::Empty res;
    grpc::ClientContext ctx;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->Create(&ctx, req, &res);
    if (status.ok())
    {
        return ErrCode_OK;
    }

    GRPCUtils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return ErrCode_OS_ERROR;
}

u8 GRPCQueueList::listQueue(std::vector<std::string> &out)
{
    out.clear();
    out.reserve(100);

    ff::Empty req;
    ff::ListQueueRes res;
    grpc::ClientContext ctx;

    GRPCUtils::setupCtx(ctx);
    auto reader = m_stub->List(&ctx, req);
    if (reader == nullptr)
    {
        spdlog::error("{}:{} reader is nullptr", LOG_FILE_PATH(__FILE__), __LINE__);
        return ErrCode_OS_ERROR;
    }

    while (reader->Read(&res))
    {
        out.push_back(res.name());
    }

    grpc::Status status = reader->Finish();
    if (status.ok())
    {
        return ErrCode_OK;
    }

    GRPCUtils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return ErrCode_OS_ERROR;
}

u8 GRPCQueueList::deleteQueue(const std::string &name)
{
    ff::QueueReq req;
    req.set_name(name);

    ff::Empty res;
    grpc::ClientContext ctx;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->Delete(&ctx, req, &res);
    if (status.ok())
    {
        return ErrCode_OK;
    }

    GRPCUtils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return ErrCode_OS_ERROR;
}

u8 GRPCQueueList::renameQueue(const std::string &oldName,
                              const std::string &newName)
{
    ff::RenameQueueReq req;
    req.set_oldname(oldName);
    req.set_newname(newName);

    ff::Empty res;
    grpc::ClientContext ctx;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->Rename(&ctx, req, &res);
    if (status.ok())
    {
        return ErrCode_OK;
    }

    GRPCUtils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return ErrCode_OS_ERROR;
}

IQueue *GRPCQueueList::getQueue(const std::string &name)
{
    ff::QueueReq req;
    req.set_name(name);

    ff::Empty res;
    grpc::ClientContext ctx;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->GetQueue(&ctx, req, &res);
    if (status.ok())
    {
        GRPCQueue *queue = new (std::nothrow) GRPCQueue;
        if (!queue)
        {
            spdlog::error("{}:{} Fail to allocate memory", LOG_FILE_PATH(__FILE__), __LINE__);
            return nullptr;
        }

        if (queue->init(m_conn, nullptr, name))
        {
            spdlog::error("{}:{} Fail to initialize queue", LOG_FILE_PATH(__FILE__), __LINE__);
            delete queue;
            return nullptr;
        }

        return queue;
    }

    GRPCUtils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return nullptr;
}

void GRPCQueueList::returnQueue(IQueue *queue)
{
    if (!queue) return;

    delete queue;
    queue = nullptr;
}

} // end namespace DAO

} // end namespace Model
