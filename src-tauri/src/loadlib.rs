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

use libloading::{Library, Symbol};
use tauri_plugin_log::log::{trace, debug, info, warn, error};
use std::sync::{Arc, OnceLock};
use std::ffi::c_char;

use crate::ffmodeldef::FFModel;

pub static GLOBAL_API: OnceLock<Arc<FFModel>> = OnceLock::new();

fn find_lib() -> anyhow::Result<Library>
{

    let path = if cfg!(target_os = "windows")
    {
        "ffmodel-c.dll"
    }
    else if cfg!(target_os = "macos")
    {
        "libffmodel-c.dylib"
    }
    else
    {
        "libffmodel-c.so"
    };

    match unsafe { Library::new(path) }
    {
        Ok(lib) =>
        {
            println!("loaded native library: {}", path);
            return Ok(lib);
        }

        Err(e) => println!("load {} failed: {}", path, e),
    }

    return Err(anyhow::anyhow!("Cannot find native library"));
} // fn find_lib() -> anyhow::Result<Library>

extern "C" fn rust_log_handler(level: i32, msg: *const c_char)
{
    let c_str = unsafe { std::ffi::CStr::from_ptr(msg) };
    let message = c_str.to_string_lossy();
    
    // 現在日誌會出現在 cargo tauri dev 的終端機裡，因為這是 Rust 噴出來的
    match level
    {
        0 => trace!("C++: {}", message), // spdlog::level::trace
        1 => debug!("C++: {}", message), // spdlog::level::debug
        2 => info!("C++: {}", message), // spdlog::level::info
        3 => warn!("C++: {}", message), // spdlog::level::warn
        4 => error!("C++: {}", message), // spdlog::level::err
        5 => error!("C++: {}", message), // spdlog::level::critical
        _ => println!("C++: {}", message),
    }
}

pub fn load_api() -> anyhow::Result<()>
{
    if GLOBAL_API.get().is_none()
    {
        let lib = find_lib().expect("Fail to find library");
        let lib = Box::leak(Box::new(lib));

        type GetApiFn = unsafe extern "C" fn(*mut FFModel, i32, cb: extern "C" fn(i32, *const c_char)) -> u8;

        let get_api: Symbol<GetApiFn> =
        unsafe { lib.get(b"getFFModel\0") }
        .expect("Fail to find symbol getFFModel");

        let mut api = std::mem::MaybeUninit::<FFModel>::uninit();

        if (unsafe { get_api(api.as_mut_ptr(), 1, rust_log_handler) } != 0)
        {
            return Err(anyhow::anyhow!("Fail to load lib"));
        }

        if GLOBAL_API.set(Arc::new(unsafe { api.assume_init() })).is_err()
        {
            return Err(anyhow::anyhow!("Cannot get native library"));
        }
    }

    return Ok(());
} // pub fn load_api() -> anyhow::Result<()>

pub fn api() -> &'static FFModel
{
    return GLOBAL_API.get().expect("API is not ready");
}
