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
use once_cell::sync::Lazy;
use std::sync::RwLock;

use crate::ffmodeldef::FFModel;

pub static MY_API: Lazy<RwLock<Option<FFModel>>> = Lazy::new(|| RwLock::new(None));

fn find_lib() -> anyhow::Result<Library> {

    let path = if cfg!(target_os = "windows") {
            "ffmodel-c.dll"
    } else if cfg!(target_os = "macos") {
            "libffmodel-c.dylib"
    } else {
        "libffmodel-c.so"
    };

    match unsafe { Library::new(path) } {
        Ok(lib) => {
            println!("loaded native library: {}", path);
            return Ok(lib);
        }

        Err(e) => println!("load {} failed: {}", path, e),
    }

    return Err(anyhow::anyhow!("Cannot find native library"));
}

pub fn load_api_once() -> anyhow::Result<FFModel> {
    let mut guard = MY_API.write().unwrap();
    if guard.is_none() {
        let lib = find_lib()?;
        let lib = Box::leak(Box::new(lib));

        type GetApiFn = unsafe extern "C" fn(*mut FFModel) -> u8;

        let get_api: Symbol<GetApiFn> = unsafe { lib.get(b"getFFModel\0") }?;

        let mut api = std::mem::MaybeUninit::<FFModel>::uninit();

        if (unsafe { get_api(api.as_mut_ptr()) } != 0) {
            return Err(anyhow::anyhow!("Fail to load lib"));
        }

        let api = unsafe { api.assume_init() };
        *guard = Some(api);
    }

    Ok(guard.unwrap())
}
