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

#include "controller/global/global.hpp"

#include "macproc.hpp"

namespace Model
{

namespace Proc
{

MacProc::MacProc():
    PosixProc()
{}

MacProc::~MacProc()
{}

// protected member functions
u8 MacProc::asioInit()
{
    int flags = fcntl(m_masterFD, F_GETFL, 0);
    if (flags == -1)
    {
        spdlog::error("{}:{} {}", LOG_FILE_PATH(__FILE__), __LINE__, strerror(errno));
        kill(m_pid, SIGKILL);
    }
    fcntl(m_masterFD, F_SETFL, flags | O_NONBLOCK);

    m_kqueue = kqueue();
    if (m_kqueue == -1)
    {
        spdlog::error("{}:{} {}", LOG_FILE_PATH(__FILE__), __LINE__, strerror(errno));
        return 1;
    }

    EV_SET(&m_change_event, m_masterFD, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    return 0;
}

void MacProc::asioFin()
{
    closeFile(&m_masterFD);
    closeFile(&m_kqueue);
}

void MacProc::readOutputLoop()
{
    while (1)
    {
        int nevents = kevent(m_kqueue, &m_change_event, 1, m_event_list, 8, NULL);
        if (nevents < 0)
        {
            if (errno == EINTR) continue;
            break;
        }
        
        for (int i = 0; i < nevents; ++i)
        {
            if (static_cast<int>(m_event_list[i].ident) == m_masterFD)
            {
                // 檢查是否結束
                if (m_event_list[i].flags & EV_EOF)
                {
                    printf("\n[Done] 子進程已退出。\n");
                    asioFin();
                    return;
                }

                // 3. 邊緣觸發風格的讀取：循環讀取直到 EAGAIN
                char buffer[FF_READ_BUFFER_SIZE];
                while (1)
                {
                    ssize_t n = read(m_masterFD, buffer, sizeof(buffer));
                    if (n > 0)
                    {
                        fwrite(buffer, 1, n, stdout);
                        fflush(stdout);
                    }
                    else if (n == -1)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                            // 資料讀完了，等待下一次 kqueue 通知
                            break; 
                        }
                        else
                        {
                            spdlog::error("{}:{} {}", LOG_FILE_PATH(__FILE__), __LINE__, strerror(errno));
                            asioFin();
                            return;
                        }
                    }
                    else
                    {
                        // n == 0 通常代表 EOF
                        asioFin();
                        return;
                    }
                } // while (1)
            } // if (m_event_list[i].ident == m_masterFD)
        } // for (int i = 0; i < nevents; ++i)
    } // end while (1)
}

} // end namespace Proc

} // end namespace Model
