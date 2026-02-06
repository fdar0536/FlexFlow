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

#include <new>

#include "spdlog/spdlog.h"

#include "winproc.hpp"

#include "controller/global/global.hpp"
#include "model/utils.hpp"

namespace Model
{

namespace Proc
{

WinProc::WinProc() :
    m_childStdoutRead(nullptr),
    m_childStdoutWrite(nullptr),
    m_procInfo(PROCESS_INFORMATION())
{}

WinProc::~WinProc()
{
    stopImpl();
}

u8 WinProc::init()
{
    spdlog::debug("{}:{} WinProc::init", LOG_FILE_PATH(__FILE__), __LINE__);

    m_exitCode.store(0, std::memory_order_relaxed);
    m_procInfo.hProcess = NULL;
    m_procInfo.hThread = NULL;
    resetHandle();
    m_deque.clear();
    return 0;
}

u8 WinProc::start(const Task &task)
{
    spdlog::debug("{}:{} WinProc::start", LOG_FILE_PATH(__FILE__), __LINE__);

    if (isRunning())
    {
        spdlog::error("{}:{} {}",
            LOG_FILE_PATH(__FILE__), __LINE__, "Process is running");
        return 1;
    }

    if (Controller::Global::isAdmin())
    {
        spdlog::error("{}:{} Refuse to run as administrator", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    resetHandle();

    if (!CreatePipe(&m_childStdoutRead, &m_childStdoutWrite, NULL, 0))
    {
        Utils::writeLastError(LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (!CreatePipe(&m_childStdinRead, &m_childStdinWrite, NULL, 0))
    {
        Utils::writeLastError(LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    COORD consoleSize{};
    consoleSize.X = 80;
    consoleSize.Y = 24;
    HRESULT res = CreatePseudoConsole(consoleSize, m_childStdinRead,
                                      m_childStdoutWrite, 0, &m_pseudoConsole);

    if (FAILED(res))
    {
        spdlog::error("{}:{} CreatePseudoConsole failed: {}",
                      LOG_FILE_PATH(__FILE__), __LINE__, res);
        resetHandle();
        return 1;
    }

    // Create the child process.
    if (CreateChildProcess(task))
    {
        spdlog::error("{}:{} {}",
            LOG_FILE_PATH(__FILE__), __LINE__, "Fail to start process");
        resetHandle();
        return 1;
    }

    CloseHandle(m_childStdoutWrite);
    m_childStdoutWrite = NULL;

    m_exitCode.store(STILL_ACTIVE, std::memory_order_relaxed);
    m_thread = std::jthread(&WinProc::readOutputLoop, this);
    return 0;
}

void WinProc::stop()
{
    stopImpl();
}

bool WinProc::isRunning()
{
    spdlog::debug("{}:{} WinProc::isRunning", LOG_FILE_PATH(__FILE__), __LINE__);
    if (m_procInfo.hProcess == NULL)
    {
        return false;
    }

    DWORD currentExitCode = 0;
    if (GetExitCodeProcess(m_procInfo.hProcess, &currentExitCode) == FALSE)
    {
        Utils::writeLastError(LOG_FILE_PATH(__FILE__), __LINE__);
        return true;
    }

    if (currentExitCode == STILL_ACTIVE)
    {
        return true;
    }

    if (m_thread.joinable())
    {
        m_thread.join();
    }

    m_exitCode.store(currentExitCode, std::memory_order_relaxed);
    return false;
}

void WinProc::readCurrentOutput(std::vector<std::string> &out)
{
    spdlog::debug("{}:{} WinProc::readCurrentOutput",
        LOG_FILE_PATH(__FILE__), __LINE__);

    out.clear();

    {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (m_deque.empty())
        {
            spdlog::debug("{}:{} nothing to read",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return;
        }

        out.reserve(m_deque.size());
        for (size_t index = 0; index < m_deque.size(); ++index)
        {
            out.push_back(std::move(m_deque.front()));
            m_deque.pop_front();
        }
    }
}

u8 WinProc::exitCode(i32 &out)
{
    spdlog::debug("{}:{} WinProc::exitCode", LOG_FILE_PATH(__FILE__), __LINE__);

    if (isRunning())
    {
        spdlog::error("{}:{} {}",
            LOG_FILE_PATH(__FILE__), __LINE__, "Process is running");
        return 1;
    }

    out = m_exitCode.load(std::memory_order_relaxed);
    return 0;
}

// private member functions
u8 WinProc::prepareStartupInformation(STARTUPINFOEXA *output)
{
    spdlog::debug("{}:{} WinProc::prepareStartupInformation",
        LOG_FILE_PATH(__FILE__), __LINE__);

    if (!output)
    {
        spdlog::error("{}:{} {}",
            LOG_FILE_PATH(__FILE__), __LINE__, "invalid input");
        return 1;
    }

    ZeroMemory(output, sizeof(STARTUPINFOEXA));
    output->StartupInfo.cb = sizeof(STARTUPINFOEXA);

    size_t bytesRequired;
    InitializeProcThreadAttributeList(NULL, 1, 0, &bytesRequired);

    output->lpAttributeList =
        (PPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(),0, bytesRequired);
    if (!output->lpAttributeList)
    {
        spdlog::error("{}:{} {}",
            LOG_FILE_PATH(__FILE__), __LINE__, "no enough memory");
        return 1;
    }

    if (!InitializeProcThreadAttributeList(
            output->lpAttributeList, 1, 0, &bytesRequired))
    {
        HeapFree(GetProcessHeap(), 0, output->lpAttributeList);
        Utils::writeLastError(LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    if (!UpdateProcThreadAttribute(output->lpAttributeList,
                                   0,
                                   PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
                                   m_pseudoConsole,
                                   sizeof(m_pseudoConsole),
                                   NULL,
                                   NULL))
    {
        HeapFree(GetProcessHeap(), 0, output->lpAttributeList);
        Utils::writeLastError(LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    return 0;
}

u8 WinProc::CreateChildProcess(const Task &task)
{
    spdlog::debug("{}:{} WinProc::CreateChildProcess",
        LOG_FILE_PATH(__FILE__), __LINE__);

    if (task.execName.empty())
    {
        spdlog::error("{}:{} {}",
            LOG_FILE_PATH(__FILE__), __LINE__, "task.execName is empty");
        return 1;
    }

    STARTUPINFOEXA siStartInfoEX;
    if (prepareStartupInformation(&siStartInfoEX))
    {
        spdlog::error("{}:{} {}", LOG_FILE_PATH(__FILE__), __LINE__,
                      "prepareStartupInformation failed");
        return 1;
    }

    BOOL bSuccess = FALSE;

    std::string cmdLine = task.execName;
    if (task.args.size())
    {
        cmdLine += " ";
        size_t lastIndex = task.args.size() - 1;
        for (size_t i = 0; i <= lastIndex; ++i)
        {
            cmdLine += task.args.at(i);
            cmdLine += " ";
        }
    }

    char *cmdPtr = new ( std::nothrow )char[cmdLine.length() + 1]();
    if (!cmdPtr)
    {
        spdlog::error("{}:{} {}",
            LOG_FILE_PATH(__FILE__), __LINE__, "Fail to allocate memory");
        return 1;
    }

    memcpy(cmdPtr, cmdLine.c_str(), cmdLine.length() + 1);

    // Create the child process.
    bSuccess = CreateProcessA(NULL,
        cmdPtr,        // command line
        NULL,          // process security attributes
        NULL,          // primary thread security attributes
        FALSE,         // no inherited handle
        EXTENDED_STARTUPINFO_PRESENT, // creation flags
        NULL,          // use parent's environment
        task.workDir.c_str(),
        &siStartInfoEX.StartupInfo,  // STARTUPINFO pointer
        &m_procInfo);  // receives PROCESS_INFORMATION

    u8 ret(0);

    // If an error occurs, exit the application.
    if (!bSuccess)
    {
        Utils::writeLastError(LOG_FILE_PATH(__FILE__), __LINE__);
        ret = 1;
    }

    delete[] cmdPtr;
    return ret;
}

void WinProc::resetHandle()
{
    spdlog::debug("{}:{} WinProc::resetHandle", LOG_FILE_PATH(__FILE__), __LINE__);

    if (m_childStdoutRead)
    {
        CloseHandle(m_childStdoutRead);
        m_childStdoutRead = nullptr;
    }

    if (m_childStdoutWrite)
    {
        CloseHandle(m_childStdoutWrite);
        m_childStdoutWrite = nullptr;
    }

    if (m_childStdinWrite)
    {
        CloseHandle(m_childStdinWrite);
        m_childStdinWrite = nullptr;
    }

    if (m_childStdinRead)
    {
        CloseHandle(m_childStdinRead);
        m_childStdinRead = nullptr;
    }

    if (m_procInfo.hProcess)
    {
        CloseHandle(m_procInfo.hProcess);
        m_procInfo.hProcess = NULL;
    }

    if (m_procInfo.hThread)
    {
        CloseHandle(m_procInfo.hThread);
        m_procInfo.hThread = NULL;
    }

    if (m_pseudoConsole)
    {
        ClosePseudoConsole(m_pseudoConsole);
        m_pseudoConsole = nullptr;
    }

    memset(&m_procInfo, 0, sizeof(PROCESS_INFORMATION));
}

void WinProc::stopImpl()
{
    spdlog::debug("{}:{} WinProc::stopImpl", LOG_FILE_PATH(__FILE__), __LINE__);

    if (m_pseudoConsole)
    {
        ClosePseudoConsole(m_pseudoConsole);
        m_pseudoConsole = nullptr;
    }

    resetHandle();
}

void WinProc::readOutputLoop()
{
    spdlog::debug("{}:{} WinProc::readOutputLoop", LOG_FILE_PATH(__FILE__), __LINE__);

    BOOL bSuccess;
    DWORD dwRead;

    while(1)
    {
        std::string buf;
        buf.resize(FF_READ_BUFFER_SIZE);
        bSuccess = ReadFile(m_childStdoutRead, buf.data(),
                            static_cast<DWORD>(buf.size()), &dwRead, NULL);
        if (!bSuccess || dwRead == 0)
        {
            // because the pipe is sync, it will not return ERROR_IO_PENDING
            // dwRead == 0 means pipe is closed, it also means child process is exited
            break;
        }

        // set correct buffer size
        buf.resize(dwRead);

        {
            std::unique_lock<std::mutex> lock(m_mutex);

            if (m_deque.size() == FF_MAX_READ_QUEUE_SIZE)
            {
                m_deque.pop_front();
            }

            m_deque.push_back(std::move(buf));
        }
    } // end while(true)
}

} // end namespace Proc

} // end namespace Model
