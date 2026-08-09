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

#ifndef _MODEL_PROC_TASK_HPP_
#define _MODEL_PROC_TASK_HPP_

#include <string>
#include <vector>

#include "model/defines.h"

namespace Model
{

namespace Proc
{
/**
 * @brief Stores the details of a task.
 */
struct Task
{
    /// Name or path of the executable.
    std::string execName;

    /// Arguments passed to the executable, excluding the executable name.
    std::vector<std::string> args;

    /// Working directory of the executable.
    std::string workDir;

    /**
     * @brief Task ID assigned when the task is added to a queue.
     *
     * A value of -1 indicates that no ID has been assigned.
     */
    i32 ID = -1;

    /**
     * @brief Exit code reported by the executed process.
     *
     * This value is valid after the task enters the finished list.
     * Its meaning is defined by the executed program.
     */
    i32 exitCode = 0;

    /**
     * @brief Indicates whether the task completed successfully.
     *
     * This value is valid after the task enters the finished list.
     */
    bool isSuccess = false;
};

/**
 * @brief Prints the task details to standard output.
 *
 * @param task Task to print.
 */
void printTask(const Task &task);

} // end namespace Proc

} // end namespace Model

#endif // _MODEL_PROC_TASK_HPP_
