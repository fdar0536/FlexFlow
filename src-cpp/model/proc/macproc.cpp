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

#include <csignal>

#include "unistd.h"
#include "fcntl.h"

#include "spdlog/spdlog.h"

#include "model/utils.hpp"

#include "macproc.hpp"

namespace Model
{

namespace Proc
{

MacProc::MacProc():
    PosixProc()
{}

MacProc::~MacProc()
{
    stopImpl();
}

// protected member functions
u8 MacProc::asioInit()
{
    spdlog::debug("{}:{} MacProc::asioInit", LOG_FILE_PATH(__FILE__), __LINE__);

    m_thread = std::jthread(&MacProc::readOutputLoop, this);
    return 0;
}

void MacProc::asioFin()
{
    spdlog::debug("{}:{} MacProc::asioFin", LOG_FILE_PATH(__FILE__), __LINE__);
    closeFile(&m_masterFD);
}

void MacProc::readOutputLoop()
{
    spdlog::debug("{}:{} MacProc::readOutputLoop",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    char buffer[FF_READ_BUFFER_SIZE];
    fd_set readfds;
    struct timeval timeout;

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(m_masterFD, &readfds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; // 100ms

        int result = select(m_masterFD + 1, &readfds, NULL, NULL, &timeout);
        if (result > 0 && FD_ISSET(m_masterFD, &readfds))
        {
            ssize_t bytesRead = read(m_masterFD, buffer, sizeof(buffer) - 1);
            if (bytesRead > 0)
            {
                buffer[bytesRead] = '\0';
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    {
                        if (m_deque.size() >= FF_MAX_READ_QUEUE_SIZE)
                        {
                            m_deque.pop_front();
                        }

                        m_deque.push_back(std::move(buffer));
                    }
                }
            }
            else if (bytesRead == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
            {
                break;
            }
        } // if (result > 0 && FD_ISSET(m_masterFD, &readfds))
    } // end while (1)
}

} // end namespace Proc

} // end namespace Model
