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

#ifndef _MODEL_PROC_POSIXPROC_HPP_
#define _MODEL_PROC_POSIXPROC_HPP_

#include <deque>

#include "iproc.hpp"

namespace Model
{

namespace Proc
{

class PosixProc : public IProc
{
public:

    ~PosixProc();

    virtual u8 init() override;

    virtual void stop() override;

    virtual void readCurrentOutput(std::vector<std::string> &out) override;

    virtual u8 exitCode(i32 &out) override;

protected:

    pid_t m_pid = 0;

    int m_masterFD = -1;

    std::atomic<i32> m_exitCode;

    void startChild(const Task &);

    char **buildChildArgv(const Task &);

    void stopImpl();

    // for read child process' output
    std::mutex m_mutex;

    std::deque<std::string> m_deque;

};

} // end namespace Proc

} // end namespace Model

#endif // _MODEL_PROC_POSIXPROC_HPP_