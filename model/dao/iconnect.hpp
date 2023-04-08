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

#ifndef _MODEL_DAO_ICONNECT_HPP_
#define _MODEL_DAO_ICONNECT_HPP_

#include <string>
#include <memory>

#include "model/errmsg.hpp"

namespace Model
{

namespace DAO
{

class IConnect
{

public:

    virtual ~IConnect() = 0;

    virtual void init(ErrMsg &) = 0;

    virtual void startConnect(ErrMsg &,
                              const std::string &target,
                              const int_fast32_t port = 0) = 0;

    void *connectToken() const;

    std::string targetPath() const;

protected:

    void *m_connectToken = nullptr;

    std::string m_targetPath = "";

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
