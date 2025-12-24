/*
 * Simple Task Queue
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

#ifndef _MODEL_TEST_PARAM_HPP_
#define _MODEL_TEST_PARAM_HPP_

#ifdef _WIN32
#include "model/proc/winproc.hpp"
#else
#include "model/proc/linuxproc.hpp"
#endif

#include "model/dao/iqueuelist.hpp"

namespace Model
{

namespace Test
{

class Param
{

public:

    ~Param();

    virtual u8 setupConn() = 0;

    Model::DAO::IQueueList *list() const;

protected:

    Proc::IProc *m_proc = nullptr;

    Model::DAO::IQueueList *m_list = nullptr;

    void printLog(const std::string_view &, int line, const char *log);

    void cleanUp();

}; // class Param

} // namespace Test

} // namespace Model

#endif // _MODEL_TEST_PARAM_HPP_
