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

pub type Handle = u32;

#[repr(C)]
#[derive(Copy, Clone)]
pub struct FFConnect {
    pub init: extern "C" fn(out: *mut Handle, backend: u8) -> u8,

    pub destroy: extern "C" fn(h: Handle) -> u8,

    pub start_connect: extern "C" fn(h: Handle, target: *const i8, port: i32) -> u8,

    pub connect_token: extern "C" fn(h: Handle) -> *mut std::ffi::c_void,

    pub target_path: extern "C" fn(h: Handle, buf: *mut i8, buf_size: *mut usize) -> u8,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct FFQueueList {

    pub init: extern "C" fn(connect: Handle, out: *mut Handle) -> u8,

    pub destroy: extern "C" fn(h: Handle) -> u8,

    pub create_queue: extern "C" fn(h: Handle, name: *const i8) -> u8,

    pub list_queue: extern "C" fn(h: Handle, out: *mut *mut *mut i8, out_size: *mut usize) -> u8,

    pub delete_queue: extern "C" fn(h: Handle, name: *const i8) -> u8,

    pub rename_queue: extern "C" fn(h: Handle, old_name: *const i8, new_name: *const i8) -> u8,

    pub get_queue: extern "C" fn(h: Handle, name: *const i8, out: *mut Handle) -> u8,

    pub return_queue: extern "C" fn(h: Handle, queue: Handle) -> u8,
}

#[repr(C)]
#[derive(Debug, Clone)]
pub struct ProcTask {
    pub exec_name: *mut i8,      // char*
    pub argc: usize,             // size_t
    pub argv: *mut *mut i8,      // char**
    pub work_dir: *mut i8,       // char*
    pub id: i32,                 // i32
    pub exit_code: i32,          // i32
    pub is_success: u8,          // u8
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct FFQueue {
    pub destroy_proc_task: extern "C" fn(task: *mut ProcTask),

    pub list_pending: extern "C" fn(h: Handle, out: *mut *mut i32, out_size: *mut usize) -> u8,
    pub list_finished: extern "C" fn(h: Handle, out: *mut *mut i32, out_size: *mut usize) -> u8,

    pub pending_details: extern "C" fn(h: Handle, id: i32, out: *mut ProcTask) -> u8,
    pub finished_details: extern "C" fn(h: Handle, id: i32, out: *mut ProcTask) -> u8,

    pub clear_pending: extern "C" fn(h: Handle) -> u8,
    pub clear_finished: extern "C" fn(h: Handle) -> u8,

    pub current_task: extern "C" fn(h: Handle, out: *mut ProcTask) -> u8,

    pub add_task: extern "C" fn(h: Handle, task: *const ProcTask) -> u8,
    pub remove_task: extern "C" fn(h: Handle, id: i32) -> u8,

    pub is_running: extern "C" fn(h: Handle) -> u8,

    /// 讀取目前正在執行任務的 stdout+stderr（每一行一個 char*）
    pub read_current_output: extern "C" fn(h: Handle, out: *mut *mut *mut i8, out_size: *mut usize) -> u8,

    pub start: extern "C" fn(h: Handle) -> u8,
    pub stop: extern "C" fn(h: Handle) -> u8,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct FFModel {
    pub connect:     FFConnect,
    pub queue_list:  FFQueueList,
    pub queue:       FFQueue,
}
