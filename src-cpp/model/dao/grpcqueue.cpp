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

#include "model/errmsg.hpp"
#include "model/dao/grpcconnect.hpp"
#include "spdlog/spdlog.h"

#include "controller/global/global.hpp"
#include "grpcutils.hpp"
#include "grpcqueue.hpp"

namespace Model
{

namespace DAO
{

GRPCQueue::GRPCQueue() :
    m_stub(nullptr),
    m_queueName("")
{}

GRPCQueue::~GRPCQueue()
{}

u8
GRPCQueue::init(IConnect *connect,
                Proc::IProc *process,
                const std::string &name)
{
    spdlog::debug("{}:{} GRPCQueue::init", LOG_FILE_PATH(__FILE__), __LINE__);

    UNUSED(process);
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

    if (name.empty())
    {
        spdlog::error("{}:{} name is empty", LOG_FILE_PATH(__FILE__), __LINE__);
        return ErrCode_INVALID_ARGUMENT;
    }

    GRPCToken *token = reinterpret_cast<GRPCToken *>(connect->connectToken());

    try
    {
        m_stub = ff::Queue::NewStub(token->channel);
        if (m_stub == nullptr)
        {
            spdlog::error("{}:{} Fail to get stub",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return ErrCode_OS_ERROR;
        }
    }
    catch (...)
    {
        spdlog::error("{}:{} Fail to get stub",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return ErrCode_OS_ERROR;
    }

    m_queueName = name;
    return ErrCode_OK;
}

u8 GRPCQueue::listPending(std::vector<int> &out)
{
    spdlog::debug("{}:{} GRPCQueue::listPending", LOG_FILE_PATH(__FILE__), __LINE__);

    out.clear();
    out.reserve(128);

    ff::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    ff::ListTaskRes res;

    GRPCUtils::setupCtx(ctx);
    auto reader = m_stub->ListPending(&ctx, req);
    if (reader == nullptr)
    {
        spdlog::error("{}:{} reader is nullptr", LOG_FILE_PATH(__FILE__), __LINE__);
        return ErrCode_OS_ERROR;
    }

    while(reader->Read(&res))
    {
        out.push_back(res.id());
    }

    UNUSED(reader->Finish());
    return ErrCode_OK;
}

u8 GRPCQueue::listFinished(std::vector<int> &out)
{
    spdlog::debug("{}:{} GRPCQueue::listFinished",
        LOG_FILE_PATH(__FILE__), __LINE__);

    out.clear();
    out.reserve(128);

    ff::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    ff::ListTaskRes res;

    GRPCUtils::setupCtx(ctx);
    auto reader = m_stub->ListFinished(&ctx, req);
    if (reader == nullptr)
    {
        spdlog::error("{}:{} reader is nullptr", LOG_FILE_PATH(__FILE__), __LINE__);
        return ErrCode_OS_ERROR;
    }

    while(reader->Read(&res))
    {
        out.push_back(res.id());
    }

    UNUSED(reader->Finish());
    return ErrCode_OK;
}

u8 GRPCQueue::pendingDetails(const int id,
                             Proc::Task &out)
{
    spdlog::debug("{}:{} GRPCQueue::pendingDetails",
        LOG_FILE_PATH(__FILE__), __LINE__);

    ff::TaskDetailsReq req;
    req.set_name(m_queueName);
    req.set_id(id);

    grpc::ClientContext ctx;
    ff::TaskDetailsRes res;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->PendingDetails(&ctx, req, &res);
    if (status.ok())
    {
        buildTask(res, out);
        return ErrCode_OK;
    }

    GRPCUtils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return ErrCode_OS_ERROR;
}

u8 GRPCQueue::finishedDetails(const int id,
                              Proc::Task &out)
{
    spdlog::debug("{}:{} GRPCQueue::finishedDetails",
        LOG_FILE_PATH(__FILE__), __LINE__);

    ff::TaskDetailsReq req;
    req.set_name(m_queueName);
    req.set_id(id);

    grpc::ClientContext ctx;
    ff::TaskDetailsRes res;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->FinishedDetails(&ctx, req, &res);
    if (status.ok())
    {
        buildTask(res, out);
        return ErrCode_OK;
    }

    GRPCUtils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return ErrCode_OS_ERROR;
}

u8 GRPCQueue::clearPending()
{
    spdlog::debug("{}:{} GRPCQueue::clearPending",
        LOG_FILE_PATH(__FILE__), __LINE__);

    ff::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    ff::Empty res;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->ClearPending(&ctx, req, &res);
    if (status.ok())
    {
        return ErrCode_OK;
    }

    GRPCUtils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return ErrCode_OS_ERROR;
}

u8 GRPCQueue::clearFinished()
{
    spdlog::debug("{}:{} GRPCQueue::clearFinished",
        LOG_FILE_PATH(__FILE__), __LINE__);

    ff::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    ff::Empty res;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->ClearFinished(&ctx, req, &res);
    if (status.ok())
    {
        return ErrCode_OK;
    }

    GRPCUtils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return ErrCode_OS_ERROR;
}

u8 GRPCQueue::currentTask(Proc::Task &out)
{
    spdlog::debug("{}:{} GRPCQueue::currentTask",
        LOG_FILE_PATH(__FILE__), __LINE__);

    ff::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    ff::TaskDetailsRes res;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->CurrentTask(&ctx, req, &res);
    if (status.ok())
    {
        buildTask(res, out);
        return ErrCode_OK;
    }

    GRPCUtils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return ErrCode_OS_ERROR;
}

u8 GRPCQueue::addTask(Proc::Task &in)
{
    spdlog::debug("{}:{} GRPCQueue::addTask",
        LOG_FILE_PATH(__FILE__), __LINE__);

    ff::AddTaskReq req;
    req.set_name(m_queueName);
    req.set_workdir(in.workDir);
    req.set_execname(in.execName);
    for (auto it = in.args.begin();
         it != in.args.end();
         ++it)
    {
        req.add_args(*it);
    }

    grpc::ClientContext ctx;
    ff::ListTaskRes res;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->AddTask(&ctx, req, &res);
    if (status.ok())
    {
        in.ID = res.id();
        return ErrCode_OK;
    }

    GRPCUtils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return ErrCode_OS_ERROR;
}

u8 GRPCQueue::removeTask(const i32 in)
{
    spdlog::debug("{}:{} GRPCQueue::removeTask",
        LOG_FILE_PATH(__FILE__), __LINE__);

    ff::TaskDetailsReq req;
    req.set_name(m_queueName);
    req.set_id(in);

    grpc::ClientContext ctx;
    ff::Empty res;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status;

    status = m_stub->RemoveTask(&ctx, req, &res);
    if (!status.ok())
    {
        GRPCUtils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
        return ErrCode_OS_ERROR;
    }

    return ErrCode_OK;
}

bool GRPCQueue::isRunning() const
{
    spdlog::debug("{}:{} GRPCQueue::isRunning",
        LOG_FILE_PATH(__FILE__), __LINE__);

    ff::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    ff::IsRunningRes res;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->IsRunning(&ctx, req, &res);
    if (status.ok())
    {
        return res.isrunning();
    }

    GRPCUtils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return false;
}

void GRPCQueue::readCurrentOutput(std::vector<std::string> &out)
{
    spdlog::debug("{}:{} GRPCQueue::readCurrentOutput",
        LOG_FILE_PATH(__FILE__), __LINE__);

    out.clear();
    out.reserve(FF_MAX_READ_QUEUE_SIZE);

    ff::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    ff::Msg res;
    GRPCUtils::setupCtx(ctx);

    auto reader = m_stub->ReadCurrentOutput(&ctx, req);
    if (reader == nullptr)
    {
        spdlog::error("{}:{} reader is nullptr", LOG_FILE_PATH(__FILE__), __LINE__);
        return;
    }

    while (reader->Read(&res))
    {
        out.push_back(std::move(res.msg()));
    }

    UNUSED(reader->Finish());
}

u8 GRPCQueue::start()
{
    spdlog::debug("{}:{} GRPCQueue::start", LOG_FILE_PATH(__FILE__), __LINE__);

    ff::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    ff::Empty res;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->Start(&ctx, req, &res);
    if (status.ok())
    {
        return ErrCode_OK;
    }

    GRPCUtils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return ErrCode_OS_ERROR;
}

void GRPCQueue::stop()
{
    spdlog::debug("{}:{} GRPCQueue::stop", LOG_FILE_PATH(__FILE__), __LINE__);

    ff::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    ff::Empty res;

    GRPCUtils::setupCtx(ctx);
    grpc::Status status = m_stub->Stop(&ctx, req, &res);
    if (status.ok())
    {
        return;
    }

    GRPCUtils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
}

// private member functions
void GRPCQueue::buildTask(ff::TaskDetailsRes &res, Proc::Task &task)
{
    spdlog::debug("{}:{} GRPCQueue::buildTask", LOG_FILE_PATH(__FILE__), __LINE__);

    task.workDir = res.workdir();
    task.execName = res.execname();
    task.args.clear();
    task.args.reserve(res.args_size());
    for (auto i = 0; i < res.args_size(); ++i)
    {
        task.args.push_back(res.args().at(i));
    }

    task.exitCode = res.exitcode();
    task.ID = res.id();
}

} // end namespace DAO

} // end namespace Model
