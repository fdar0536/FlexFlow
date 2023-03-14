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

#ifndef _MODEL_DAO_SQLITEQUEUELIST_HPP_
#define _MODEL_DAO_SQLITEQUEUELIST_HPP_

#include <unordered_map>

#include "sqliteconnect.hpp"
#include "iqueuelist.hpp"

namespace Model
{

namespace DAO
{

class SQLiteQueueList: public IQueueList<SQLiteToken>
{
public:

    SQLiteQueueList();

    ~SQLiteQueueList();

    virtual uint_fast8_t
    init(std::shared_ptr<IConnect<SQLiteToken>> &connect) override;

    virtual uint_fast8_t createQueue(const std::string &name) override;

    virtual uint_fast8_t listQueue(std::vector<std::string> &out) override;

    virtual uint_fast8_t deleteQueue(const std::string &name) override;

    virtual uint_fast8_t renameQueue(const std::string &oldName,
                                const std::string &newName) override;

    virtual std::shared_ptr<IQueue<SQLiteToken>>
    getQueue(const std::string &name) override;

private:

    std::unordered_map<std::string,
    std::shared_ptr<IQueue<SQLiteToken>>> m_queueList;
};

} // end namespace DAO

} // end namespace Model

#endif // _MODEL_DAO_SQLITEQUEUELIST_HPP_

