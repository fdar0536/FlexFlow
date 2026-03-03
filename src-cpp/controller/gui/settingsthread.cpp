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

#include "controller/global/defines.h"
#include "spdlog/spdlog.h"

#include "controller/global/global.hpp"
#include "controller/gui/global.hpp"
#include <mutex>

#include "settingsthread.hpp"

namespace G = Controller::Gui::Global;

namespace Controller
{

namespace Gui
{

SettingsThread::SettingsThread(QObject *parent):
    QThread(parent)
{}

SettingsThread::~SettingsThread()
{}

// public member functions
u8 SettingsThread::startConnect()
{
    spdlog::debug("{}:{} SettingsThread::startConnect",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_mode != INVALID)
        {
            spdlog::warn("{}:{} This thread is running",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        m_mode = CONNECT;
    }
    
    this->start();
    return 0;
}

u8 SettingsThread::startDisconnect()
{
    spdlog::debug("{}:{} SettingsThread::startDisconnect",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_mode != INVALID)
        {
            spdlog::warn("{}:{} This thread is running",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        m_mode = DISCONNECT;
    }
    
    return 0;
}

void SettingsThread::run()
{
    Mode mode = INVALID;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_mode == INVALID)
        {
            spdlog::error("{}:{} Invalid mode",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return;
        }

        mode = m_mode;
    }

    (this->*handler[mode])();

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_mode = INVALID;
    }
}

// private member functions
void SettingsThread::doConnect()
{
    spdlog::debug("{}:{} SettingsThread::doConnect",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    if (G::config.currentProfile.list() != nullptr)
    {
        spdlog::warn("{}:{} Already connected",
            LOG_FILE_PATH(__FILE__), __LINE__);
        emit doConnectDone(0);
        return;
    }

    // list == nullptr
    Model::DAO::IQueueList *list = nullptr;
    switch (G::config.currentProfile.mode())
    {
    case BACKEND_GRPC:
    {
        if (Controller::Global::grpcInit(&list,
            G::config.currentProfile.target(),
            G::config.currentProfile.port())
        )
        {
            spdlog::error("{}:{} Fail to init grpc",
                LOG_FILE_PATH(__FILE__), __LINE__);
            
            emit doConnectDone(1);
            return;
        }

        break;
    }
    case BACKEND_SQLITE:
    {
        if (Controller::Global::sqliteInit(&list,
            G::config.currentProfile.target()))
        {
            spdlog::error("{}:{} Fail to init sqlite",
                LOG_FILE_PATH(__FILE__), __LINE__);
            emit doConnectDone(1);
            return;
        }

        break;
    }
    default:
    {
        spdlog::error("{}:{} unknown mode", LOG_FILE_PATH(__FILE__), __LINE__);
        emit doConnectDone(1);
        return;
    }
    } // switch (G::config.currentProfile.mode())

    G::config.currentProfile.setList(std::shared_ptr<Model::DAO::IQueueList>(list));
    emit doConnectDone(0);
}

void SettingsThread::doDisconnect()
{
    spdlog::debug("{}:{} SettingsThread::doDisconnect",
        LOG_FILE_PATH(__FILE__), __LINE__);
    G::config.currentProfile.setList(nullptr);
    emit doDisconnectDone();
}

} // namespace Gui

} // namespace Controller