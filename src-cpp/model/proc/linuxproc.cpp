/*
 * Simple Task Queue
 * Copyright (c) 2023-present fdar0536
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

#include <cerrno>
#include <signal.h>
#include <mutex>
#include <string.h>

#include "model/proc/posixproc.hpp"
#include "fcntl.h"

#include "spdlog/spdlog.h"

#include "controller/global/global.hpp"

#include "config.h"

#include "linuxproc.hpp"

namespace Model
{

namespace Proc
{

LinuxProc::LinuxProc() :
    PosixProc()
{}

LinuxProc::~LinuxProc()
{}

// protected member functions
u8 LinuxProc::asioInit()
{
    int fileFlag(fcntl(m_masterFD, F_GETFL));
    if (fileFlag == -1)
    {
        spdlog::error("{}:{} {}", LOG_FILE_PATH(__FILE__), __LINE__, strerror(errno));
        kill(m_pid, SIGKILL);
        return 1;
    }

    if (fcntl(m_masterFD, F_SETFL, fileFlag | O_NONBLOCK) == -1)
    {
        spdlog::error("{}:{} {}", LOG_FILE_PATH(__FILE__), __LINE__, strerror(errno));
        kill(m_pid, SIGKILL);
        return 1;
    }

    if (epollInit())
    {
        spdlog::error("{}:{} {}", LOG_FILE_PATH(__FILE__), __LINE__, strerror(errno));
        kill(m_pid, SIGKILL);
        epollFin();
        return 1;
    }

    m_thread = std::jthread(&LinuxProc::readOutputLoop, this);
    return 0;
}

void LinuxProc::asioFin()
{
    return epollFin();
}

void LinuxProc::readOutputLoop()
{
    ssize_t count(0);
    while(1)
    {
        int event_count = epoll_wait(m_epoll_fd, m_events, 10, -1);
        if (event_count == -1)
        {
            if (errno == EINTR) continue; // 被訊號中斷則繼續
            // epoll failed
            spdlog::error("{}:{} epoll_wait failed: {}",
                LOG_FILE_PATH(__FILE__), __LINE__, strerror(errno));
            break;
        }

        for (int i = 0; i < event_count; ++i)
        {
            if (m_events[i].data.fd == m_masterFD &&
                (m_events[i].events & EPOLLIN))
            {
                std::string buf;
                buf.resize(FF_READ_BUFFER_SIZE);
                while (1)
                {
                    count = read(m_events[i].data.fd, buf.data(), buf.size());
                    if (count == -1)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                            // back to epoll_wait
                            break;
                        }

                        if (errno == EINTR)
                        {
                            // read again
                            spdlog::debug("{}:{} {}", LOG_FILE_PATH(__FILE__), __LINE__, strerror(errno));
                            continue;
                        }
                        else
                        {
                            // others error, include EIO
                            spdlog::error("{}:{} {}", LOG_FILE_PATH(__FILE__), __LINE__, strerror(errno));
                            return;
                        }
                    }
                    else if (count == 0)
                    {
                        // pipe is closed or child process is exited
                        spdlog::debug("{}:{} {}", LOG_FILE_PATH(__FILE__), __LINE__, "Nothing to read");
                        return;
                    }
                    else // count != 0
                    {
                        buf.resize(count);
                        {
                            std::unique_lock<std::mutex> lock(m_mutex);
                            if (m_deque.size() >= FF_MAX_READ_QUEUE_SIZE)
                            {
                                m_deque.pop_front();
                            }

                            m_deque.push_back(std::move(buf));
                        }
                        // continue to read until EAGAIN
                    }
                } // end while(1)
            }

            // others error
            if (m_events[i].events & (EPOLLHUP | EPOLLERR))
            {
                spdlog::debug("{}:{} Epoll HUP/ERR on fd {}",
                    LOG_FILE_PATH(__FILE__), __LINE__,
                    static_cast<int>(m_events[i].data.fd));
                return;
            }
        } // end for (int i = 0; i < event_count; ++i)
    } // end while(1)
}

// private member functions
u8 LinuxProc::epollInit()
{
    m_epoll_fd = epoll_create1(0);
    if (m_epoll_fd == -1)
    {
        spdlog::error("{}:{} {}", LOG_FILE_PATH(__FILE__), __LINE__, strerror(errno));
        return 1;
    }

    m_event.events = EPOLLIN;
    m_event.data.fd = m_masterFD;
    if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_masterFD, &m_event))
    {
        spdlog::error("{}:{} {}", LOG_FILE_PATH(__FILE__), __LINE__, strerror(errno));
        return 1;
    }

    return 0;
}

void LinuxProc::epollFin()
{
    closeFile(&m_epoll_fd);
    closeFile(&m_masterFD);
}

} // end namespace Proc

} // end namespace Model
