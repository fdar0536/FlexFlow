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

#include <mutex>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/base_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "controller/global/global.hpp"

#include "connect.h"
#include "queuelist.h"
#include "queue.h"

#include "capi.h"

template<typename Mutex>
class CapiSink : public spdlog::sinks::base_sink<Mutex>
{
public:
    static void setCallback(LogCallback cb)
    {
        spdlog::debug("{}:{} setCallback", LOG_FILE_PATH(__FILE__), __LINE__);
        std::lock_guard<std::mutex> lock(s_mutex);
        s_callback = cb;
    }

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (s_callback)
        {
            // 如果有註冊 Callback，就把格式化後的字串丟出去
            spdlog::memory_buf_t formatted;
            spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
            s_callback(static_cast<int>(msg.level), fmt::to_string(formatted).c_str());
        }

        // if callback is nullptr, do nothing
    }

    void flush_() override {}

private:
    static LogCallback s_callback;
    static std::mutex s_mutex;
}; // class CapiSink

template<typename Mutex> LogCallback CapiSink<Mutex>::s_callback = nullptr;
template<typename Mutex> std::mutex CapiSink<Mutex>::s_mutex;

static u8 initLogging(i32 level, LogCallback cb)
{
    spdlog::debug("{}:{} initLogging", LOG_FILE_PATH(__FILE__), __LINE__);
    
    if (level < 0 || level > 6)
    {
        spdlog::error("{}:{} invalid level", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    try
    {
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto capiSink = std::make_shared<CapiSink<std::mutex>>();
        capiSink->setCallback(cb);

        // 組合多個 Sink
        std::vector<spdlog::sink_ptr> sinks { consoleSink, capiSink };
        auto logger = std::make_shared<spdlog::logger>("multi_sink", sinks.begin(), sinks.end());
        logger->set_level(static_cast<spdlog::level::level_enum>(level));
        
        spdlog::set_default_logger(logger);
    }
    catch (...)
    {
        // it may cause memory allocate failed
        spdlog::error("{}:{} Fail to init logging", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return 0;
}

static void buildConnect(FFConnect &c)
{
    c.init = connect_init;
    c.destroy = connect_destroy;
    c.startConnect = connect_startConnect;
    c.connectToken = connect_connectToken;
    c.targetPath = connect_targetPath;
}

static void buildQueueList(FFQueueList &l)
{
    l.init = queuelist_init;
    l.destroy = queuelist_destroy;
    l.createQueue = queuelist_createQueue;
    l.listQueue = queuelist_listQueue;
    l.deleteQueue = queuelist_deleteQueue;
    l.renameQueue = queuelist_renameQueue;
    l.getQueue = queuelist_getQueue;
    l.returnQueue = queuelist_returnQueue;
}

static void buildQueue(FFQueue &q)
{
    q.destroyProcTask = queue_destroyProcTask;
    q.listPending = queue_listPending;
    q.listFinished = queue_listFinished;
    q.pendingDetails = queue_pendingDetails;
    q.finishedDetails = queue_finishedDetails;
    q.clearPending = queue_clearPending;
    q.clearFinished = queue_clearFinished;
    q.currentTask = queue_currentTask;
    q.addTask = queue_addTask;
    q.removeTask = queue_removeTask;
    q.isRunning = queue_isRunning;
    q.readCurrentOutput = queue_readCurrentOutput;
    q.start = queue_start;
    q.stop = queue_stop;
}

extern "C"
{

FF_MODEL_API u8 getFFModel(FFModel *in, int level, LogCallback cb)
{
    if (!in)
    {
        spdlog::error("{}:{} invalid input", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (initLogging(level, cb))
    {
        spdlog::error("{}:{} Fail to init logging", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    buildConnect(in->Connect);
    buildQueueList(in->QueueList);
    buildQueue(in->Queue);

    return 0;
}

} // extern "C"
