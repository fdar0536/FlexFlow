/*
 * Flex Flow
 * Copyright (c) 2025-present fdar0536
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

 use crate::loadlib as lib;
 use crate::ffmodeldef as def;

#[tauri::command]
pub fn ffconnect_init(backend: u8) -> Result<def::Handle, u8>
{
    let mut handle: def::Handle = 0;
    let ret = (lib::api().connect.init)(&mut handle, backend);
    if ret == 0
    {
        Ok(handle)
    }
    else
    {
        Err(ret)
    }
}

#[tauri::command]
pub fn ffconnect_destroy(h: def::Handle) -> Result<(), u8>
{
    let ret = (lib::api().connect.destroy)(h);
    if ret == 0 { Ok(()) } else { Err(ret) }
}

#[tauri::command]
pub fn ffconnect_start_connect(h: def::Handle, target: &str, port: i32)
-> Result<(), u8>
{
    let c_target = std::ffi::CString::new(target).unwrap();
    let ret = (lib::api().connect.start_connect)(h,
        c_target.as_ptr(), port);
    if ret == 0 { Ok(()) } else { Err(ret) }
}

#[tauri::command]
pub fn ffconnect_target_path(h: def::Handle, buf_size: usize) -> Result<String, u8>
{
    let mut buffer = vec![0i8; buf_size];
    let mut size = buf_size;
    let ret = (lib::api().connect.target_path)(h,
        buffer.as_mut_ptr(), &mut size as *mut usize);

    if ret != 0
    {
        return Err(ret);
    }

    let real_len =
    buffer.iter().position(|&b| b == 0).unwrap_or(size);
    
    let s = String::from_utf8_lossy(
        unsafe { std::mem::transmute(&buffer[..real_len]) }).to_string();
    Ok(s)
}
