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

use crate::loadlib::api;
use crate::ffmodeldef as def;

#[tauri::command]
pub fn queue_list_init(connect_handle: def::Handle) -> Result<def::Handle, u8>
{
    let mut out = 0u32;
    let ret = (api().queue_list.init)(connect_handle, &mut out);
    if ret == 0 { Ok(out) } else { Err(ret) }
}

#[tauri::command]
pub fn queue_list_destroy(h: def::Handle) -> Result<(), u8>
{
    let ret = (api().queue_list.destroy)(h);
    if ret == 0 { Ok(()) } else { Err(ret) }
}

#[tauri::command]
pub async fn queue_list_create_queue(h: def::Handle, name: &str)
-> Result<(), u8>
{
    let name = name.to_owned();
    return tokio::task::spawn_blocking(move ||
    {
        let c_name = std::ffi::CString::new(name).map_err(|_| 255u8)?;
        let ret = (api().queue_list.create_queue)(h, c_name.as_ptr());
        if ret == 0 { Ok(()) } else { Err(ret) }
    })
    .await
    .unwrap_or(Err(255));
}

#[tauri::command]
pub async fn queue_list_list_queue(h: def::Handle) -> Result<Vec<String>, u8>
{
    return tokio::task::spawn_blocking(move ||
    {
        let mut out_ptr: *mut *mut i8 = std::ptr::null_mut();
        let mut out_size: usize = 0;

        let ret = (api().queue_list.list_queue)
        (h, &mut out_ptr as *mut *mut *mut i8, &mut out_size);
        if ret != 0
        {
            return Err(ret);
        }

        let slice =
        unsafe { std::slice::from_raw_parts(out_ptr, out_size) };
        let mut result = Vec::with_capacity(out_size);

        for &ptr in slice {
            let cstr = unsafe { std::ffi::CStr::from_ptr(ptr) } ;
            let s = cstr.to_string_lossy().into_owned();
            result.push(s);

            unsafe { libc::free(ptr as *mut libc::c_void) };
        }

        unsafe { libc::free(out_ptr as *mut libc::c_void) };

        Ok(result)
    })
    .await
    .unwrap_or(Err(255));
}

#[tauri::command]
pub async fn queue_list_delete_queue(h: def::Handle, name: &str)
-> Result<(), u8>
{
    let name = name.to_owned();
    return tokio::task::spawn_blocking(move ||
    {
        let c_name = std::ffi::CString::new(name).map_err(|_| 255u8)?;
        let ret = (api().queue_list.delete_queue)(h, c_name.as_ptr());
        if ret == 0 { Ok(()) } else { Err(ret) }
    })
    .await
    .unwrap_or(Err(255));
}

#[tauri::command]
pub async fn queue_list_rename_queue
(h: def::Handle, old: &str, new: &str) -> Result<(), u8>
{
    let old = old.to_owned();
    let new = new.to_owned();
    return tokio::task::spawn_blocking(move ||
    {
        let c_old = std::ffi::CString::new(old).map_err(|_| 255u8)?;
        let c_new = std::ffi::CString::new(new).map_err(|_| 255u8)?;
        let ret = (api().queue_list.rename_queue)
        (h, c_old.as_ptr(), c_new.as_ptr());
        if ret == 0 { Ok(()) } else { Err(ret) }
    })
    .await
    .unwrap_or(Err(255));
}

#[tauri::command]
pub fn queue_list_get_queue(h: def::Handle, name: &str)
-> Result<def::Handle, u8>
{
    let c_name = std::ffi::CString::new(name).map_err(|_| 255u8)?;
    let mut out = 0u32;
    let ret = (api().queue_list.get_queue)(h, c_name.as_ptr(), &mut out);
    if ret == 0 { Ok(out) } else { Err(ret) }
}

#[tauri::command]
pub fn queue_list_return_queue
(h: def::Handle, queue: def::Handle) -> Result<(), u8>
{
    let ret = (api().queue_list.return_queue)(h, queue);
    if ret == 0 { Ok(()) } else { Err(ret) }
}
