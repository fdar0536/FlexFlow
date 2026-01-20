/*
 * Simple Task Queue
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

#ifndef _MODEL_DAO_ICONNECT_HPP_
#define _MODEL_DAO_ICONNECT_HPP_

#include <string>

#include "controller/global/defines.h"

namespace Model
{

namespace DAO
{

/**
 * @brief Define the connect way to server
 */
class IConnect
{

public:

    virtual ~IConnect() = 0;

    /**
     * @brief Initialize the class
     * @return u8 return 0 if success
     */
    virtual u8 init() = 0;

    /**
     * @brief Start connect to target
     * 
     * @param target Target host
     * @param port Target port
     * @return u8 return 0 if success
     */
    virtual u8 startConnect(const std::string &target,
                            const i32 port = 0) = 0;
    
    /**
     * @brief Get the connect for further operation
     * @return void* return nullptr if failed
     */
    void *connectToken() const;

    /**
     * @brief Get the target host
     * @return std::string target host
     */
    std::string targetPath() const;

protected:

    /**
     * @brief the pointer to store connect token
     */
    void *m_connectToken = nullptr;

    /**
     * @brief target path
     */
    std::string m_targetPath = "";

    /**
     * @brief destroy connect token
     * @tparam T the acturl type or class of connect token
     */
    template<class T>
    void freeConnectToken()
    {
        if (m_connectToken)
        {
            T *token = reinterpret_cast<T *>(m_connectToken);
            delete token;
            m_connectToken = nullptr;
        }
    }

}; // end class IConnect

} // end namespace DAO

} // end namespace Model

#endif // _MODEL_DAO_ICONNECT_HPP_
