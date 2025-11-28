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

#ifndef _MODEL_DAO_GRPCQUEUE_HPP_
#define _MODEL_DAO_GRPCQUEUE_HPP_

#include "queue.grpc.pb.h"

#include "model/proc/task.hpp"
#include "iqueue.hpp"

namespace Model
{

namespace DAO
{

class FF_MODEL_API GRPCQueue : public IQueue
{

public:

    GRPCQueue();

    ~GRPCQueue();

    u8 init(IConnect *connect,
            Proc::IProc *process,
            const std::string &name) override;

    u8 listPending(std::vector<int> &out) override;

    u8 listFinished(std::vector<int> &out) override;

    u8 pendingDetails(const int id,
                      Proc::Task &out) override;

    u8 finishedDetails(const int id,
                       Proc::Task &out) override;

    u8 clearPending() override;

    u8 clearFinished() override;

    u8 currentTask(Proc::Task &out) override;

    u8 addTask(Proc::Task &in) override;

    u8 removeTask(const i32 in) override;

    bool isRunning() const override;

    void readCurrentOutput(std::vector<std::string> &out) override;

    u8 start() override;

    void stop() override;

private:

    std::unique_ptr<ff::Queue::Stub> m_stub;

    std::string m_queueName;

    static void buildTask(ff::TaskDetailsRes &res, Proc::Task &task);

}; // end class GRPCQueue

} // end namespace DAO

} // end namespace Model

#endif // _MODEL_DAO_GRPCQUEUE_HPP_
