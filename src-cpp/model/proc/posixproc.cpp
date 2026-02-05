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

#include <signal.h>

#include "unistd.h"

#ifdef __linux__
#include "sys/wait.h"
#include "pty.h"
#else
#include "util.h"
#endif

#include "spdlog/spdlog.h"

#include "controller/global/global.hpp"

#include "posixproc.hpp"

// implement PosixProc
namespace Model
{

namespace Proc
{

// implement public member functions
PosixProc::~PosixProc()
{
    stopImpl();
}

u8 PosixProc::init()
{
    m_exitCode.store(0, std::memory_order_relaxed);
    m_deque.clear();
    return 0;
}

u8 PosixProc::start(const Task &task)
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

    return asioInit();
}

void PosixProc::stop()
{
    stopImpl();
}

bool PosixProc::isRunning()
{
    int status;
    pid_t ret = waitpid(m_pid, &status, WNOHANG);
    if (ret == -1)
    {
        spdlog::debug("{}:{} {}", LOG_FILE_PATH(__FILE__), __LINE__, strerror(errno));
        asioFin();
        return false;
    }
    else if (ret == 0)
    {
        return true;
    }
    
    if (WIFEXITED(status) || WIFSIGNALED(status))
    {
        m_exitCode.store(status, std::memory_order_relaxed);
    }

    asioFin();
    return false;
}

void PosixProc::readCurrentOutput(std::vector<std::string> &out)
{
    out.clear();

    {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (m_deque.empty())
        {
            spdlog::debug("{}:{} nothing to read", LOG_FILE_PATH(__FILE__), __LINE__);
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

u8 PosixProc::exitCode(i32 &out)
{
    if (isRunning())
    {
        spdlog::error("{}:{} Process is running", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    out = m_exitCode.load(std::memory_order_relaxed);
    return 0;
}

// protected member function
void PosixProc::startChild(const Task &task)
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

char **PosixProc::buildChildArgv(const Task &task)
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

void PosixProc::stopImpl()
{
    if (kill(m_pid, SIGKILL) == -1)
    {
        perror("kill");
        return;
    }

    sleep(2);
}

void PosixProc::closeFile(int *fd)
{
    if (*fd != -1)
    {
        close(*fd);
        *fd = -1;
    }
}

} // end namespace Proc

} // end namespace Model