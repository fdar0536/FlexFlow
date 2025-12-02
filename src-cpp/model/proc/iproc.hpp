/*
 * Simple Task Queue
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

#ifndef _MODEL_PROC_IPROC_HPP_
#define _MODEL_PROC_IPROC_HPP_

#include "task.hpp"

namespace Model
{

namespace Proc
{

class IProc
{
public:

    virtual ~IProc();

    virtual u8 init() = 0;

    virtual u8 start(const Task &task) = 0;

    virtual void stop() = 0;

    virtual bool isRunning() = 0;

    virtual void readCurrentOutput(std::vector<std::string> &out) = 0;

    virtual u8 exitCode(i32 &out) = 0;

}; // end class IProc

} // end namespace Proc

} // end namespace Model

#endif // _MODEL_PROC_IPROC_HPP_
