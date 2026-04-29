/*
 * Flex Flow
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

#include "connect.hpp"
#include "queue.hpp"
#include "queuelist.hpp"
#include "utils.hpp"

namespace Model
{

namespace DAO
{

namespace GRPC
{

QueueList::QueueList() :
    m_stub(nullptr)
{}

QueueList::~QueueList()
{
    if (m_conn) delete m_conn;
}

u8 QueueList::init(IConnect *connect)
{
    spdlog::debug("{}:{} QueueList::init", LOG_FILE_PATH(__FILE__), __LINE__);

    if (!connect)
    {
        spdlog::error("{}:{} connect is nullptr", LOG_FILE_PATH(__FILE__), __LINE__);
        return ErrCode_INVALID_ARGUMENT;
    }

    if (!connect->connectToken())
    {
        spdlog::error("{}:{} connect token is nullptr",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return ErrCode_INVALID_ARGUMENT;
    }

    Token *token = reinterpret_cast<Token *>(connect->connectToken());
    try
    {
        m_stub = ff::QueueList::NewStub(token->channel);
        if (m_stub == nullptr)
        {
            spdlog::error("{}:{} Fail to get stub",
                LOG_FILE_PATH(__FILE__), __LINE__);
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

u8 QueueList::createQueue(const std::string &name)
{
    spdlog::debug("{}:{} QueueList::createQueue",
        LOG_FILE_PATH(__FILE__), __LINE__);

    ff::QueueReq req;
    req.set_name(name);

    ff::Empty res;
    grpc::ClientContext ctx;

    Utils::setupCtx(ctx);
    grpc::Status status = m_stub->Create(&ctx, req, &res);
    if (status.ok())
    {
        return ErrCode_OK;
    }

    Utils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return ErrCode_OS_ERROR;
}

u8 QueueList::listQueue(std::vector<std::string> &out)
{
    spdlog::debug("{}:{} QueueList::listQueue",
        LOG_FILE_PATH(__FILE__), __LINE__);

    out.clear();
    out.reserve(100);

    ff::Empty req;
    ff::ListQueueRes res;
    grpc::ClientContext ctx;

    Utils::setupCtx(ctx);
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

    Utils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return ErrCode_OS_ERROR;
}

u8 QueueList::deleteQueue(const std::string &name)
{
    spdlog::debug("{}:{} QueueList::deleteQueue",
        LOG_FILE_PATH(__FILE__), __LINE__);

    ff::QueueReq req;
    req.set_name(name);

    ff::Empty res;
    grpc::ClientContext ctx;

    Utils::setupCtx(ctx);
    grpc::Status status = m_stub->Delete(&ctx, req, &res);
    if (status.ok())
    {
        return ErrCode_OK;
    }

    Utils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return ErrCode_OS_ERROR;
}

u8 QueueList::renameQueue(const std::string &oldName,
                              const std::string &newName)
{
    spdlog::debug("{}:{} QueueList::renameQueue",
        LOG_FILE_PATH(__FILE__), __LINE__);

    ff::RenameQueueReq req;
    req.set_oldname(oldName);
    req.set_newname(newName);

    ff::Empty res;
    grpc::ClientContext ctx;

    Utils::setupCtx(ctx);
    grpc::Status status = m_stub->Rename(&ctx, req, &res);
    if (status.ok())
    {
        return ErrCode_OK;
    }

    Utils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return ErrCode_OS_ERROR;
}

std::shared_ptr<IQueue> QueueList::getQueue(const std::string &name)
{
    spdlog::debug("{}:{} QueueList::getQueue",
        LOG_FILE_PATH(__FILE__), __LINE__);

    ff::QueueReq req;
    req.set_name(name);

    ff::Empty res;
    grpc::ClientContext ctx;

    Utils::setupCtx(ctx);
    grpc::Status status = m_stub->GetQueue(&ctx, req, &res);
    if (status.ok())
    {
        Queue *queue = new (std::nothrow) Queue;
        if (!queue)
        {
            spdlog::error("{}:{} Fail to allocate memory",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return nullptr;
        }

        if (queue->init(m_conn, nullptr, name))
        {
            spdlog::error("{}:{} Fail to initialize queue",
                LOG_FILE_PATH(__FILE__), __LINE__);
            delete queue;
            return nullptr;
        }

        return std::shared_ptr<IQueue>(queue);
    }

    Utils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return nullptr;
}

} // end namespace GRPC

} // end namespace DAO

} // end namespace Model
