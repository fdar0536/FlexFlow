/*
 * Simple Task Queue
 * Copyright (c) 2023 fdar0536
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

#ifndef _MODEL_PROC_LINUXPROC_HPP_
#define _MODEL_PROC_LINUXPROC_HPP_

#include <atomic>
#include <deque>
#include <thread>

#include "sys/epoll.h"

#include "iproc.hpp"

namespace Model
{

namespace Proc
{

class LinuxProc : public IProc
{
public:

    LinuxProc();

    ~LinuxProc();

    virtual u8 init() override;

    virtual u8 start(const Task &task) override;

    virtual void stop() override;

    virtual bool isRunning() override;

    virtual void readCurrentOutput(std::vector<std::string> &out) override;

    virtual u8 exitCode(i32 &out) override;

private:

    pid_t m_pid;

    int m_masterFD = -1;

    std::atomic<i32> m_exitCode;

    void startChild(const Task &);

    char **buildChildArgv(const Task &);

    void stopImpl();

    // epoll
    struct epoll_event m_event, m_events[10];

    int m_epoll_fd = -1;

    u8 epollInit();

    void closeFile(int *);

    void epollFin();

    // for reading current output
    std::jthread m_thread;

    std::mutex m_mutex;

    std::deque<std::string> m_deque;

    void readOutputLoop();
};

} // end namespace Proc

} // end namespace Model

#endif // _MODEL_PROC_LINUXPROC_HPP_
