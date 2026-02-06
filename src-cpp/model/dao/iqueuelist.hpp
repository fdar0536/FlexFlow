/*
 * Flex Flow
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

#ifndef _MODEL_DAO_IQUEUELIST_HPP_
#define _MODEL_DAO_IQUEUELIST_HPP_

#include <string>
#include <vector>

#include "iconnect.hpp"
#include "iqueue.hpp"

namespace Model
{

namespace DAO
{

/**
 * @brief define the interface for queue list
 */
class IQueueList
{
public:

    virtual ~IQueueList() {}

    /**
     * @brief initialize
     * @param connect the connect object form IConnect
     * @return u8 return 0 if success
     */
    virtual u8 init(IConnect *connect) = 0;

    /**
     * @brief Create a Queue
     * @param name the name for the new queue
     * @return u8 return 0 if success
     */
    virtual u8 createQueue(const std::string &name) = 0;

    /**
     * @brief list all queue in list
     * @param[out] out the list of queue(s)
     * @return u8 return 0 if success
     */
    virtual u8 listQueue(std::vector<std::string> &out) = 0;

    /**
     * @brief delete the queue
     * @param name the name of queue you want to delete
     * @return u8 return 0 if success
     */
    virtual u8 deleteQueue(const std::string &name) = 0;

    /**
     * @brief rename the existed queue
     * 
     * @param oldName the current name of queue
     * @param newName the new name of queue
     * @return u8 return 0 if success
     */
    virtual u8 renameQueue(const std::string &oldName,
                           const std::string &newName) = 0;

    /**
     * @brief Get the Queue object
     * 
     * @param name the queue's name you want to get
     * @return IQueue* return nullptr if failed
     */
    virtual IQueue *getQueue(const std::string &name) = 0;

    /**
     * @brief return the queue back to this queue list
     * @param queue the queue you want to return
     */
    virtual void returnQueue(IQueue *queue) = 0;

protected:

    /**
     * @brief the IConnect object 
     */
    IConnect *m_conn;

}; // end class IQueueList

} // end namespace DAO

} // end namespace Model

#endif // _MODEL_DAO_IQUEUELIST_HPP_
