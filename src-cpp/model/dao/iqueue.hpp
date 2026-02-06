/*
 * Flex Flow
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

#ifndef _MODEL_DAO_IQUEUE_HPP_
#define _MODEL_DAO_IQUEUE_HPP_

#include <memory>
#include <vector>

#include "model/proc/iproc.hpp"
#include "model/proc/task.hpp"
#include "iconnect.hpp"

namespace Model
{

namespace DAO
{

class IQueue
{
public:

    virtual ~IQueue() {}

    virtual u8 init(IConnect *connect,
                    Proc::IProc *process,
                    const std::string &name) = 0;

    virtual u8 listPending(std::vector<int> &out) = 0;

    virtual u8 listFinished(std::vector<int> &out) = 0;

    virtual u8 pendingDetails(const int id,
                              Proc::Task &out) = 0;

    virtual u8 finishedDetails(const int id,
                               Proc::Task &out) = 0;

    virtual u8 clearPending() = 0;

    virtual u8 clearFinished() = 0;

    virtual u8 currentTask(Proc::Task &out) = 0;

    virtual u8 addTask(Proc::Task &in) = 0;

    virtual u8 removeTask(const i32 in) = 0;

    virtual bool isRunning() const = 0;

    virtual void readCurrentOutput(std::vector<std::string> &out) = 0;

    virtual u8 start() = 0;

    virtual void stop() = 0;

protected:

    Model::Proc::IProc *m_proc;

}; // end class IQueue

} // end namespace DAO

} // end namespace Model

#endif // _MODEL_DAO_IQUEUE_HPP_
