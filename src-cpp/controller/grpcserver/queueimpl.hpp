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

#ifndef _CONTROLLER_GRPCSERVER_QUEUEIMPL_HPP_
#define _CONTROLLER_GRPCSERVER_QUEUEIMPL_HPP_

#include "queue.grpc.pb.h"

namespace Controller
{

namespace GRPCServer
{

class QueueImpl : public ff::Queue::Service
{
public:

    grpc::Status
    ListPending(grpc::ServerContext *ctx,
                const ff::QueueReq *req,
                grpc::ServerWriter<ff::ListTaskRes> *writer) override;

    grpc::Status
    ListFinished(grpc::ServerContext *ctx,
                 const ff::QueueReq *req,
                 grpc::ServerWriter<ff::ListTaskRes> *writer) override;

    grpc::Status
    PendingDetails(grpc::ServerContext *ctx,
                   const ff::TaskDetailsReq *req,
                   ff::TaskDetailsRes *res) override;

    grpc::Status
    FinishedDetails(grpc::ServerContext *ctx,
                    const ff::TaskDetailsReq *req,
                    ff::TaskDetailsRes *res) override;

    grpc::Status
    ClearPending(grpc::ServerContext *ctx,
                 const ff::QueueReq *req,
                 ff::Empty *res) override;

    grpc::Status
    ClearFinished(grpc::ServerContext *ctx,
                  const ff::QueueReq *req,
                  ff::Empty *res) override;

    grpc::Status
    CurrentTask(grpc::ServerContext *ctx,
                const ff::QueueReq *req,
                ff::TaskDetailsRes *res) override;

    grpc::Status
    AddTask(grpc::ServerContext *ctx,
            const ff::AddTaskReq *req,
            ff::ListTaskRes *res) override;

    grpc::Status
    RemoveTask(grpc::ServerContext *ctx,
               const ff::TaskDetailsReq *req,
               ff::Empty *res) override;

    grpc::Status
    IsRunning(grpc::ServerContext *ctx,
              const ff::QueueReq *req,
              ff::IsRunningRes *res) override;

    grpc::Status
    ReadCurrentOutput(grpc::ServerContext *ctx,
                      const ff::QueueReq *req,
                      grpc::ServerWriter<ff::Msg> *writer) override;

    grpc::Status
    Start(grpc::ServerContext *ctx,
          const ff::QueueReq *req,
          ff::Empty *res) override;

    grpc::Status
    Stop(grpc::ServerContext *ctx,
         const ff::QueueReq *req,
         ff::Empty *res) override;
};

} // end namespace GRPCServer

} // end namespace Controller

#endif // _CONTROLLER_GRPCSERVER_QUEUEIMPL_HPP_
