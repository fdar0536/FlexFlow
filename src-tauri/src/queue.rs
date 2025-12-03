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

use serde::Serialize;
use crate::loadlib::api;
use crate::ffmodeldef as def;

 #[derive(Serialize, serde::Deserialize, Debug, Clone)]
 pub struct ProcTaskDto
 {
     pub exec_name: String,
     pub args: Vec<String>,
     pub work_dir: Option<String>,
     pub id: i32,
     pub exit_code: i32,
     pub is_success: bool,
 }
 
 impl ProcTaskDto
 {
    pub unsafe fn from_c(task_ptr: *mut def::ProcTask) -> Self
    {
        if task_ptr.is_null()
        {
            return ProcTaskDto {
                exec_name: "".to_string(),
                args: vec![],
                work_dir: None,
                id: -1,
                exit_code: -1,
                is_success: false,
            };
        }

        let task = &*task_ptr;

        let exec_name = if !task.exec_name.is_null()
        {
            std::ffi::CStr::from_ptr(task.exec_name).to_string_lossy().into_owned()
        }
        else
        {
            "".to_string()
        };

        let args = if !task.argv.is_null() && task.argc > 0
        {
            let mut vec = Vec::with_capacity(task.argc);
            for i in 0..task.argc
            {
                let ptr = *task.argv.offset(i as isize);
                if !ptr.is_null()
                {
                    let s = std::ffi::CStr::from_ptr(ptr).to_string_lossy().into_owned();
                    vec.push(s);
                }
            }
            vec
        }
        else
        {
            vec![]
        };

        let work_dir = if !task.work_dir.is_null()
        {
            Some(std::ffi::CStr::from_ptr(task.work_dir).to_string_lossy().into_owned())
        }
        else
        {
            None
        };

        let dto = ProcTaskDto
        {
            exec_name,
            args,
            work_dir,
            id: task.id,
            exit_code: task.exit_code,
            is_success: task.is_success != 0,
        };

        (api().queue.destroy_proc_task)(task_ptr);
        dto
    }
}

pub fn proc_task_from_strings(
    exec_name: &str,
    args: &[&str],
    work_dir: Option<&str>,
) -> def::ProcTask
{
    use std::ffi::CString;

    let exec_c = CString::new(exec_name).unwrap();
    let argv_c: Vec<*mut i8> = args
        .iter()
        .map(|s| CString::new(*s).unwrap().into_raw())
        .collect();

    let exec_ptr = exec_c.into_raw();
    let argv_ptr = Box::into_raw(argv_c.into_boxed_slice()) as *mut *mut i8;
    let work_dir_ptr = work_dir
        .map(|s| CString::new(s).unwrap().into_raw())
        .unwrap_or(std::ptr::null_mut());

    def::ProcTask
    {
        exec_name: exec_ptr,
        argc: args.len(),
        argv: argv_ptr,
        work_dir: work_dir_ptr,
        id: 0,
        exit_code: 0,
        is_success: 0,
    }
}

#[tauri::command]
pub async fn queue_list_pending(h: def::Handle) -> Result<Vec<i32>, u8>
{
    return tokio::task::spawn_blocking(move ||
    {
        _list_ids(h, api().queue.list_pending)
    })
    .await
    .unwrap_or(Err(255));
}

#[tauri::command]
pub async fn queue_list_finished(h: def::Handle) -> Result<Vec<i32>, u8>
{
    return tokio::task::spawn_blocking(move ||
    {
        _list_ids(h, api().queue.list_finished)
    })
    .await
    .unwrap_or(Err(255));
}

fn _list_ids
(h: def::Handle,
func: extern "C" fn(def::Handle, *mut *mut i32, *mut usize) -> u8)
-> Result<Vec<i32>, u8>
{
    let mut out: *mut i32 = std::ptr::null_mut();
    let mut size: usize = 0;
    let ret = func(h, &mut out as *mut *mut i32, &mut size);
    if ret != 0 { return Err(ret); }

    let ids = unsafe{ std::slice::from_raw_parts(out, size).to_vec() };
    
    unsafe { libc::free(out as *mut _) };
    Ok(ids)
}

#[tauri::command]
pub async fn queue_pending_details(h: def::Handle, id: i32)
-> Result<ProcTaskDto, u8>
{
    return tokio::task::spawn_blocking(move ||
    {
        let mut task: def::ProcTask = unsafe { std::mem::zeroed() };
        let ret = (api().queue.pending_details)(h, id, &mut task);
        if ret != 0 { return Err(ret); }
        Ok(unsafe { ProcTaskDto::from_c(&mut task) })
    })
    .await
    .unwrap_or(Err(255));
}

#[tauri::command]
pub async fn queue_finished_details(h: def::Handle, id: i32)
-> Result<ProcTaskDto, u8>
{
    return tokio::task::spawn_blocking(move ||
    {
        let mut task: def::ProcTask = unsafe { std::mem::zeroed() };
        let ret = ((api().queue.finished_details))(h, id, &mut task);
        if ret != 0 { return Err(ret); }
        Ok(unsafe { ProcTaskDto::from_c(&mut task) })
    })
    .await
    .unwrap_or(Err(255));
}

#[tauri::command]
pub async fn queue_current_task(h: def::Handle)
-> Result<Option<ProcTaskDto>, u8>
{
    return tokio::task::spawn_blocking(move ||
    {
        let mut task: def::ProcTask = unsafe { std::mem::zeroed() };
        let ret = (api().queue.current_task)(h, &mut task);
        if ret == 0
        {
            Ok(Some(unsafe { ProcTaskDto::from_c(&mut task) }))
        }
        else
        {
            Err(ret)
        }
    })
    .await
    .unwrap_or(Err(255));
}

#[tauri::command]
pub async fn queue_add_task
(h: def::Handle, task: ProcTaskDto) -> Result<(), u8>
{
    return tokio::task::spawn_blocking(move ||
    {
        let c_task = proc_task_from_strings(
            &task.exec_name,
            &task.args.iter().map(|s| s.as_str()).collect::<Vec<&str>>(),
            task.work_dir.as_deref(),
        );
        let ret = (api().queue.add_task)(h, &c_task);
        if ret == 0 { Ok(()) } else { Err(ret) }
    })
    .await
    .unwrap_or(Err(255));
}

#[tauri::command]
pub async fn queue_read_current_output(h: def::Handle)
-> Result<Vec<String>, u8>
{
    return tokio::task::spawn_blocking(move ||
    {
        let mut out: *mut *mut i8 = std::ptr::null_mut();
        let mut size: usize = 0;

        let ret = (api().queue.read_current_output)
        (h, &mut out as *mut *mut *mut i8, &mut size);
        if ret != 0 { return Err(ret); }

        let slice = unsafe { std::slice::from_raw_parts(out, size) };
        let mut lines = Vec::with_capacity(size);

        for &ptr in slice {
            let cstr = unsafe { std::ffi::CStr::from_ptr(ptr) };
            lines.push(cstr.to_string_lossy().into_owned());
            unsafe { libc::free(ptr as *mut _) };
        }

        unsafe { libc::free(out as *mut _) };
        Ok(lines)
    })
    .await
    .unwrap_or(Err(255));
}

#[tauri::command]
pub async fn queue_start(h: def::Handle) -> Result<(), u8>
{
    return tokio::task::spawn_blocking(move ||
    {
        let ret = (api().queue.start)(h);
        if ret == 0 { Ok(()) } else { Err(ret) }
    })
    .await
    .unwrap_or(Err(255));
}

#[tauri::command]
pub async fn queue_stop(h: def::Handle) -> Result<(), u8>
{
    return tokio::task::spawn_blocking(move ||
    {
        let ret = (api().queue.stop)(h);
        if ret == 0 { Ok(()) } else { Err(ret) }
    })
    .await
    .unwrap_or(Err(255));
}

#[tauri::command]
pub async fn queue_is_running(h: def::Handle) -> bool
{
    return tokio::task::spawn_blocking(move ||
    {
        (api().queue.is_running)(h) != 0
    })
    .await
    .unwrap_or(false);
}

#[tauri::command]
pub async fn queue_clear_pending(h: def::Handle) -> Result<(), u8>
{
    return tokio::task::spawn_blocking(move ||
    {
        let ret = (api().queue.clear_pending)(h);
        if ret == 0 { Ok(()) } else { Err(ret) }
    })
    .await
    .unwrap_or(Err(255));
}

#[tauri::command]
pub async fn queue_clear_finished(h: def::Handle) -> Result<(), u8>
{
    return tokio::task::spawn_blocking(move ||
    {
        let ret = (api().queue.clear_finished)(h);
        if ret == 0 { Ok(()) } else { Err(ret) }
    })
    .await
    .unwrap_or(Err(255));
}

#[tauri::command]
pub async fn queue_remove_task(h: def::Handle, id: i32) -> Result<(), u8>
{
    return tokio::task::spawn_blocking(move ||
    {
        let ret = (api().queue.remove_task)(h, id);
        if ret == 0 { Ok(()) } else { Err(ret) }
    })
    .await
    .unwrap_or(Err(255));
}
