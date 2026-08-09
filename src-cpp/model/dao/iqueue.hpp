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

#include <vector>

#include "model/proc/task.hpp"

namespace Model
{

namespace DAO
{
/**
 * @brief Defines the interface for a task queue.
 *
 * This interface manages pending, running, and finished tasks in a queue.
 *
 * @note All member functions are thread-safe.
 */
class IQueue
{
public:
    virtual ~IQueue() = default;

    /**
     * @brief Lists all pending tasks in this queue.
     *
     * @param[out] out Receives the pending task IDs.
     *                 The vector is cleared before being populated.
     * @return 0 on success; otherwise, a non-zero value.
     */
    virtual u8 listPending(std::vector<int> &out) = 0;

    /**
     * @brief Lists all finished tasks in this queue.
     *
     * @param[out] out Receives the finished task IDs.
     *                 The vector is cleared before being populated.
     * @return 0 on success; otherwise, a non-zero value.
     */
    virtual u8 listFinished(std::vector<int> &out) = 0;

    /**
     * @brief Retrieves the details of a pending task.
     *
     * @param id ID of the pending task to retrieve.
     * @param[out] out Receives the task details.
     * @return 0 on success; otherwise, a non-zero value.
     */
    virtual u8 pendingDetails(const int id,
                              Proc::Task &out) = 0;

    /**
     * @brief Retrieves the details of a finished task.
     *
     * @param id ID of the finished task to retrieve.
     * @param[out] out Receives the task details.
     * @return 0 on success; otherwise, a non-zero value.
     */
    virtual u8 finishedDetails(const int id,
                               Proc::Task &out) = 0;

    /**
     * @brief Clears all tasks from the pending list.
     *
     * @return 0 on success; otherwise, a non-zero value.
     */
    virtual u8 clearPending() = 0;

    /**
     * @brief Clears all tasks from the finished list.
     *
     * @return 0 on success; otherwise, a non-zero value.
     */
    virtual u8 clearFinished() = 0;

    /**
     * @brief Retrieves the currently running task.
     *
     * @param[out] out Receives the currently running task.
     * @return 0 on success; otherwise, a non-zero value.
     */
    virtual u8 currentTask(Proc::Task &out) = 0;

    /**
     * @brief Adds a task to the pending list.
     *
     * @param[in,out] in Task to add. The task ID is assigned on success.
     * @return 0 on success; otherwise, a non-zero value.
     */
    virtual u8 addTask(Proc::Task &in) = 0;

    /**
     * @brief Removes a task from the pending list.
     *
     * @param in ID of the pending task to remove.
     * @return 0 on success; otherwise, a non-zero value.
     */
    virtual u8 removeTask(const i32 in) = 0;

    /**
     * @brief Checks whether this queue is running.
     *
     * @return true if the queue is running; otherwise, false.
     */
    virtual bool isRunning() const = 0;

    /**
     * @brief Reads the current output of this queue.
     *
     * @param[out] out Receives the current queue output.
     *                 The vector is cleared before being populated.
     *
     * @note Reading the output clears the internal output buffer.
     */
    virtual void readCurrentOutput(std::vector<std::string> &out) = 0;

    /**
     * @brief Starts this queue.
     *
     * @return 0 on success; otherwise, a non-zero value.
     */
    virtual u8 start() = 0;

    /**
     * @brief Stops this queue.
     */
    virtual void stop() = 0;
}; // end class IQueue

} // end namespace DAO

} // end namespace Model

#endif // _MODEL_DAO_IQUEUE_HPP_
