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

#include "model/errmsg.hpp"
#include "connect.hpp"

#include "controller/global/global.hpp"
#include "utils.hpp"
#include "queue.hpp"

namespace Model
{

namespace DAO
{

namespace GRPC
{

Queue::Queue() :
    m_stub(nullptr),
    m_queueName("")
{}

Queue::~Queue()
{}

u8
Queue::init(IConnect *connect,
                Proc::IProc *process,
                const std::string &name)
{
    spdlog::debug("{}:{} Queue::init", LOG_FILE_PATH(__FILE__), __LINE__);

    UNUSED(process);
    if (!connect)
    {
        spdlog::error(
            "{}:{} connect is nullptr",
            LOG_FILE_PATH(__FILE__), __LINE__);
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

    Token *token = reinterpret_cast<Token *>(connect->connectToken());

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

u8 Queue::listPending(std::vector<int> &out)
{
    spdlog::debug("{}:{} Queue::listPending", LOG_FILE_PATH(__FILE__), __LINE__);

    out.clear();
    out.reserve(128);

    ff::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    ff::ListTaskRes res;

    Utils::setupCtx(ctx);
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

u8 Queue::listFinished(std::vector<int> &out)
{
    spdlog::debug("{}:{} Queue::listFinished",
        LOG_FILE_PATH(__FILE__), __LINE__);

    out.clear();
    out.reserve(128);

    ff::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    ff::ListTaskRes res;

    Utils::setupCtx(ctx);
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

u8 Queue::pendingDetails(const int id,
                             Proc::Task &out)
{
    spdlog::debug("{}:{} Queue::pendingDetails",
        LOG_FILE_PATH(__FILE__), __LINE__);

    ff::TaskDetailsReq req;
    req.set_name(m_queueName);
    req.set_id(id);

    grpc::ClientContext ctx;
    ff::TaskDetailsRes res;

    Utils::setupCtx(ctx);
    grpc::Status status = m_stub->PendingDetails(&ctx, req, &res);
    if (status.ok())
    {
        buildTask(res, out);
        return ErrCode_OK;
    }

    Utils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return ErrCode_OS_ERROR;
}

u8 Queue::finishedDetails(const int id,
                              Proc::Task &out)
{
    spdlog::debug("{}:{} Queue::finishedDetails",
        LOG_FILE_PATH(__FILE__), __LINE__);

    ff::TaskDetailsReq req;
    req.set_name(m_queueName);
    req.set_id(id);

    grpc::ClientContext ctx;
    ff::TaskDetailsRes res;

    Utils::setupCtx(ctx);
    grpc::Status status = m_stub->FinishedDetails(&ctx, req, &res);
    if (status.ok())
    {
        buildTask(res, out);
        return ErrCode_OK;
    }

    Utils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return ErrCode_OS_ERROR;
}

u8 Queue::clearPending()
{
    spdlog::debug("{}:{} Queue::clearPending",
        LOG_FILE_PATH(__FILE__), __LINE__);

    ff::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    ff::Empty res;

    Utils::setupCtx(ctx);
    grpc::Status status = m_stub->ClearPending(&ctx, req, &res);
    if (status.ok())
    {
        return ErrCode_OK;
    }

    Utils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return ErrCode_OS_ERROR;
}

u8 Queue::clearFinished()
{
    spdlog::debug("{}:{} Queue::clearFinished",
        LOG_FILE_PATH(__FILE__), __LINE__);

    ff::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    ff::Empty res;

    Utils::setupCtx(ctx);
    grpc::Status status = m_stub->ClearFinished(&ctx, req, &res);
    if (status.ok())
    {
        return ErrCode_OK;
    }

    Utils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return ErrCode_OS_ERROR;
}

u8 Queue::currentTask(Proc::Task &out)
{
    spdlog::debug("{}:{} Queue::currentTask",
        LOG_FILE_PATH(__FILE__), __LINE__);

    ff::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    ff::TaskDetailsRes res;

    Utils::setupCtx(ctx);
    grpc::Status status = m_stub->CurrentTask(&ctx, req, &res);
    if (status.ok())
    {
        buildTask(res, out);
        return ErrCode_OK;
    }

    Utils::buildErrMsg(
        LOG_FILE_PATH(__FILE__), __LINE__, status);
    return ErrCode_OS_ERROR;
}

u8 Queue::addTask(Proc::Task &in)
{
    spdlog::debug("{}:{} Queue::addTask",
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

    Utils::setupCtx(ctx);
    grpc::Status status = m_stub->AddTask(&ctx, req, &res);
    if (status.ok())
    {
        in.ID = res.id();
        return ErrCode_OK;
    }

    Utils::buildErrMsg(
        LOG_FILE_PATH(__FILE__), __LINE__, status);
    return ErrCode_OS_ERROR;
}

u8 Queue::removeTask(const i32 in)
{
    spdlog::debug("{}:{} Queue::removeTask",
        LOG_FILE_PATH(__FILE__), __LINE__);

    ff::TaskDetailsReq req;
    req.set_name(m_queueName);
    req.set_id(in);

    grpc::ClientContext ctx;
    ff::Empty res;

    Utils::setupCtx(ctx);
    grpc::Status status;

    status = m_stub->RemoveTask(&ctx, req, &res);
    if (!status.ok())
    {
        Utils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
        return ErrCode_OS_ERROR;
    }

    return ErrCode_OK;
}

bool Queue::isRunning() const
{
    spdlog::debug("{}:{} Queue::isRunning",
        LOG_FILE_PATH(__FILE__), __LINE__);

    ff::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    ff::IsRunningRes res;

    Utils::setupCtx(ctx);
    grpc::Status status = m_stub->IsRunning(&ctx, req, &res);
    if (status.ok())
    {
        return res.isrunning();
    }

    Utils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return false;
}

void Queue::readCurrentOutput(std::vector<std::string> &out)
{
    spdlog::debug("{}:{} Queue::readCurrentOutput",
        LOG_FILE_PATH(__FILE__), __LINE__);

    out.clear();
    out.reserve(FF_MAX_READ_QUEUE_SIZE);

    ff::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    ff::Msg res;
    Utils::setupCtx(ctx);

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

u8 Queue::start()
{
    spdlog::debug("{}:{} Queue::start", LOG_FILE_PATH(__FILE__), __LINE__);

    ff::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    ff::Empty res;

    Utils::setupCtx(ctx);
    grpc::Status status = m_stub->Start(&ctx, req, &res);
    if (status.ok())
    {
        return ErrCode_OK;
    }

    Utils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
    return ErrCode_OS_ERROR;
}

void Queue::stop()
{
    spdlog::debug("{}:{} Queue::stop", LOG_FILE_PATH(__FILE__), __LINE__);

    ff::QueueReq req;
    req.set_name(m_queueName);

    grpc::ClientContext ctx;
    ff::Empty res;

    Utils::setupCtx(ctx);
    grpc::Status status = m_stub->Stop(&ctx, req, &res);
    if (status.ok())
    {
        return;
    }

    Utils::buildErrMsg(LOG_FILE_PATH(__FILE__), __LINE__, status);
}

// private member functions
void Queue::buildTask(ff::TaskDetailsRes &res, Proc::Task &task)
{
    spdlog::debug("{}:{} Queue::buildTask", LOG_FILE_PATH(__FILE__), __LINE__);

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

} // end namespace GRPC

} // end namespace DAO

} // end namespace Model
