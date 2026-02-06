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

#ifndef _MODEL_DAO_SQLITEQUEUE_HPP_
#define _MODEL_DAO_SQLITEQUEUE_HPP_

#include <atomic>
#include <mutex>
#include <thread>

#include "sqliteconnect.hpp"
#include "iqueue.hpp"

namespace Model
{

namespace DAO
{

class SQLiteQueue: public IQueue
{
public:

    SQLiteQueue();

    ~SQLiteQueue();

    virtual u8 init(IConnect *connect,
                    Proc::IProc *process,
                    const std::string &name) override;

    virtual u8 listPending(std::vector<int> &out) override;

    virtual u8 listFinished(std::vector<int> &out) override;

    virtual u8 pendingDetails(const int id,
                              Proc::Task &out) override;

    virtual u8 finishedDetails(const int id,
                               Proc::Task &out) override;

    virtual u8 clearPending() override;

    virtual u8 clearFinished() override;

    virtual u8 currentTask(Proc::Task &out) override;

    virtual u8 addTask(Proc::Task &in) override;

    virtual u8 removeTask(const i32 in) override;

    virtual bool isRunning() const override;

    virtual void readCurrentOutput(std::vector<std::string> &out) override;

    virtual u8 start() override;

    virtual void stop() override;

    u8 rename(const std::string &newName, const std::string &oldName);

private:

    std::shared_ptr<SQLiteToken> m_token;

    std::mutex m_currentTaskMutex;

    Proc::Task m_currentTask;

    std::atomic<bool> m_isRunning;

    std::atomic<bool> m_start;

    std::jthread m_thread;

    std::string m_targetPath;

    u8 connectToDB(const std::string &, const std::string & = "");

    u8 createTable(const std::string &);

    u8 verifyTable(const std::string &);

    u8 verifyID();

    u8 clearTable(const std::string &);

    u8 listIDInTable(const std::string &, std::vector<int> &);

    u8 taskDetails(const std::string &,
                   const i32,
                   Proc::Task &);

    u8 addTaskToTable(const std::string &, const Proc::Task &);

    u8 removeTaskFromPending(const i32, const bool);

    void splitString(const std::string &, std::vector<std::string> &);

    std::string concatString(const std::vector<std::string> &);

    u8 getID(i32 &);

    void mainLoop();

    u8 mainLoopInit();

    void mainLoopFin();

    void stopImpl();

}; // end class DirToken

} //end namepsace DAO

} // end namespace Model

#endif // _MODEL_DAO_SQLITEQUEUE_HPP_
