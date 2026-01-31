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
#include <config.h>
#include <mutex>
#include <string.h>

#include "model/proc/posixproc.hpp"
#include "sys/types.h"
#include "sys/wait.h"
#include "fcntl.h"
#include "pty.h"

#include "spdlog/spdlog.h"

#include "controller/global/global.hpp"

#include "linuxproc.hpp"

namespace Model
{

namespace Proc
{

LinuxProc::LinuxProc()
{}

LinuxProc::~LinuxProc()
{}

u8 LinuxProc::start(const Task &task)
{
    if (isRunning())
    {
        spdlog::error("{}:{} Process is running", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (Controller::Global::isAdmin())
    {
        spdlog::error("{}:{} Refuse to run as super user", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    m_masterFD = -1;
    m_exitCode.store(0, std::memory_order_relaxed);

    m_pid = forkpty(&m_masterFD, NULL, NULL, NULL);
    if (m_pid == -1)
    {
        // parent process
        spdlog::error("{}:{} {}", LOG_FILE_PATH(__FILE__), __LINE__, strerror(errno));
        return 1;
    }

    if (m_pid == 0)
    {
        // child process
        startChild(task);
    }

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

bool LinuxProc::isRunning()
{
    int status;
    pid_t ret = waitpid(m_pid, &status, WNOHANG);
    if (ret == -1)
    {
        spdlog::debug("{}:{} {}", LOG_FILE_PATH(__FILE__), __LINE__, strerror(errno));
        epollFin();
        return false;
    }
    else if (ret == 0)
    {
        return true;
    }
    else
    {
        if (WIFEXITED(status) || WIFSIGNALED(status))
        {
            m_exitCode.store(status, std::memory_order_relaxed);
        }

        epollFin();
        return false;
    }
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

void LinuxProc::closeFile(int *fd)
{
    if (*fd != -1)
    {
        close(*fd);
        *fd = -1;
    }
}

void LinuxProc::epollFin()
{
    closeFile(&m_epoll_fd);
    closeFile(&m_masterFD);
}

void LinuxProc::readOutputLoop()
{
    ssize_t count(0);
    while(1)
    {
        int event_count = epoll_wait(m_epoll_fd, m_events, 10, 1000);
        if (event_count == -1)
        {
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
                        if (errno == EINTR || errno == EAGAIN || errno == EIO)
                        {
                            spdlog::debug("{}:{} {}", LOG_FILE_PATH(__FILE__), __LINE__, strerror(errno));
                            continue;
                        }
                        else
                        {
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

                            if (m_deque.size() == FF_MAX_READ_QUEUE_SIZE)
                            {
                                m_deque.pop_front();
                            }

                            m_deque.push_back(std::move(buf));
                        }

                        break;
                    }
                } // end while(1)
            }
        } // end for (int i = 0; i < event_count; ++i)

        sleep(1);
    } // end while(1)
}

} // end namespace Proc

} // end namespace Model
