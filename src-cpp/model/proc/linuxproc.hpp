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

#include <thread>

#include "sys/epoll.h"

#include "posixproc.hpp"

namespace Model
{

namespace Proc
{

class LinuxProc : public PosixProc
{
public:

    LinuxProc();

    ~LinuxProc();

protected:

    virtual u8 asioInit() override;

    virtual void asioFin() override;

    virtual void readOutputLoop() override;

private:

    // epoll
    struct epoll_event m_event, m_events[10];

    int m_epoll_fd = -1;

    u8 epollInit();

    void epollFin();

    // for reading current output
    std::jthread m_thread;
};

} // end namespace Proc

} // end namespace Model

#endif // _MODEL_PROC_LINUXPROC_HPP_
