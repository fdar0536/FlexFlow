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

#ifndef _MODEL_DAO_IQUEUELIST_HPP_
#define _MODEL_DAO_IQUEUELIST_HPP_

#include <string>
#include <vector>

#include "iqueue.hpp"

namespace Model
{

namespace DAO
{

/**
 * @brief Defines the interface for managing queues.
 *
 * This interface provides operations for creating, listing, deleting,
 * renaming, and retrieving queues.
 *
 * @note All member functions are thread-safe.
 */
class IQueueList
{
public:
    virtual ~IQueueList() = default;

    /**
     * @brief Creates a queue.
     *
     * @param name Name of the queue to create.
     * @return 0 on success; otherwise, a non-zero value.
     */
    virtual u8 create(const std::string &name) = 0;

    /**
     * @brief Lists all queues.
     *
     * @param[out] out Receives the names of all queues. The vector is cleared before being populated.
     * @return 0 on success; otherwise, a non-zero value.
     */
    virtual u8 list(std::vector<std::string> &out) = 0;

    /**
     * @brief Deletes a queue.
     *
     * @param name Name of the queue to delete.
     * @return 0 on success; otherwise, a non-zero value.
     */
    virtual u8 delete_(const std::string &name) = 0;

    /**
     * @brief Renames an existing queue.
     *
     * @param oldName Current name of the queue.
     * @param newName New name of the queue.
     * @return 0 on success; otherwise, a non-zero value.
     */
    virtual u8 rename(const std::string &oldName,
                      const std::string &newName) = 0;

    /**
     * @brief Retrieves a queue by name.
     *
     * @param name Name of the queue to retrieve.
     * @return A shared pointer to the queue, or nullptr if it cannot be
     *         retrieved.
     */
    virtual std::shared_ptr<IQueue> get(const std::string &name) = 0;
};

} // end namespace DAO

} // end namespace Model

#endif // _MODEL_DAO_IQUEUELIST_HPP_
