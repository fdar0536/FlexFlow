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

LinuxProc::LinuxProc() :
    m_pid(0)
{}

LinuxProc::~LinuxProc()
{}

u8 LinuxProc::init()
{
    m_exitCode.store(0, std::memory_order_relaxed);
    m_deque.clear();
    return 0;
}

u8 LinuxProc::start(const Task &task)
{
    if (isRunning())
    {
        spdlog::error("{}:{} Process is running", __FILE__, __LINE__);
        return 1;
    }

    if (Controller::Global::isAdmin())
    {
        spdlog::error("{}:{} Refuse to run as super user", __FILE__, __LINE__);
        return 1;
    }

    m_masterFD = -1;
    m_exitCode.store(0, std::memory_order_relaxed);

    m_pid = forkpty(&m_masterFD, NULL, NULL, NULL);
    if (m_pid == -1)
    {
        // parent process
        spdlog::error("{}:{} {}", __FILE__, __LINE__, strerror(errno));
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
        spdlog::error("{}:{} {}", __FILE__, __LINE__, strerror(errno));
        kill(m_pid, SIGKILL);
        return 1;
    }

    if (fcntl(m_masterFD, F_SETFL, fileFlag | O_NONBLOCK) == -1)
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, strerror(errno));
        kill(m_pid, SIGKILL);
        return 1;
    }

    if (epollInit())
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, strerror(errno));
        kill(m_pid, SIGKILL);
        epollFin();
        return 1;
    }

    m_thread = std::jthread(&LinuxProc::readOutputLoop, this);
    return 0;
}

void LinuxProc::stop()
{
    stopImpl();
}

bool LinuxProc::isRunning()
{
    int status;
    pid_t ret = waitpid(m_pid, &status, WNOHANG);
    if (ret == -1)
    {
        spdlog::debug("{}:{} {}", __FILE__, __LINE__, strerror(errno));
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

void LinuxProc::readCurrentOutput(std::vector<std::string> &out)
{
    out.clear();

    {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (m_deque.empty())
        {
            spdlog::debug("{}:{} nothing to read", __FILE__, __LINE__);
            return;
        }

        out.reserve(m_deque.size());
        for (size_t index = 0; index < m_deque.size(); ++index)
        {
            out.push_back(std::move(m_deque.front()));
            m_deque.pop_front();
        }
    }
}

u8 LinuxProc::exitCode(i32 &out)
{
    if (isRunning())
    {
        spdlog::error("{}:{} Process is running", __FILE__, __LINE__);
        return 1;
    }

    out = m_exitCode.load(std::memory_order_relaxed);
    return 0;
}

// private member functions
void LinuxProc::startChild(const Task &task)
{
    if (chdir(task.workDir.c_str()) == -1)
    {
        perror("chdir");
        exit(1);
    }

    char **argv = buildChildArgv(task);
    if (!argv)
    {
        exit(1);
    }

    // start child
    char *env[] = { NULL };
    execve(task.execName.c_str(), argv, env);
    perror("execve");   /* execve() returns only on error */
    exit(1);
}

char **LinuxProc::buildChildArgv(const Task &task)
{
    char **argv(nullptr);
    if (task.args.size())
    {
        argv = new ( std::nothrow ) char *[task.args.size() + 2](); // process name + null for tail
    }
    else
    {
        argv = new ( std::nothrow ) char *[2]();
    }

    if (!argv)
    {
        fprintf(stderr, "Fail to allocate child process' argv.");
        return nullptr;
    }

    size_t nameLen = task.execName.length() + 1;
    argv[0] = new ( std::nothrow ) char[nameLen]();
    if (!argv[0])
    {
        delete[] argv;
        fprintf(stderr, "Fail to allocate child process' argv[0].");
        return nullptr;
    }

    memcpy(argv[0], task.execName.c_str(), nameLen);
    if (task.args.size())
    {
        size_t index(0);
        for (size_t i = 0; i < task.args.size(); ++i)
        {
            index = i + 1;
            argv[index] = new ( std::nothrow ) char[task.args[i].size() + 1](); // +1 for '\0'
            if (!argv[index])
            {
                for (size_t j = 0; j < index; ++j)
                {
                    delete[] argv[j];
                }

                delete[] argv;
                fprintf(stderr, "Fail to allocate child process' argv[%zu].", index);
                return nullptr;
            }

            memcpy(argv[index], task.args[i].c_str(), task.args[i].size());
            argv[index][task.args[i].size()] = '\0';
        }

        argv[task.args.size() + 1] = NULL;
    }
    else
    {
        argv[1] = NULL;
    }

    return argv;
}

void LinuxProc::stopImpl()
{
    if (kill(m_pid, SIGKILL) == -1)
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, strerror(errno));
        return;
    }

    sleep(2);
}

u8 LinuxProc::epollInit()
{
    m_epoll_fd = epoll_create1(0);
    if (m_epoll_fd == -1)
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, strerror(errno));
        return 1;
    }

    m_event.events = EPOLLIN;
    m_event.data.fd = m_masterFD;
    if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_masterFD, &m_event))
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, strerror(errno));
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
                __FILE__, __LINE__, strerror(errno));
            break;
        }

        for (int i = 0; i < event_count; ++i)
        {
            if (m_events[i].data.fd == m_masterFD &&
                (m_events[i].events & EPOLLIN))
            {
                std::string buf;
                buf.resize(STQ_READ_BUFFER_SIZE);
                while (1)
                {
                    count = read(m_events[i].data.fd, buf.data(), buf.size());
                    if (count == -1)
                    {
                        if (errno == EINTR || errno == EAGAIN || errno == EIO)
                        {
                            spdlog::debug("{}:{} {}", __FILE__, __LINE__, strerror(errno));
                            continue;
                        }
                        else
                        {
                            spdlog::error("{}:{} {}", __FILE__, __LINE__, strerror(errno));
                            return;
                        }
                    }
                    else if (count == 0)
                    {
                        // pipe is closed or child process is exited
                        spdlog::debug("{}:{} {}", __FILE__, __LINE__, "Nothing to read");
                        return;
                    }
                    else // count != 0
                    {
                        buf.resize(count);

                        {
                            std::unique_lock<std::mutex> lock(m_mutex);

                            if (m_deque.size() == STQ_MAX_READ_QUEUE_SIZE)
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
