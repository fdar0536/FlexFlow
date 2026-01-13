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

use std::env;
use std::path::PathBuf;

fn main()
{
    tauri_build::build();
    println!("cargo:rerun-if-env-changed=CPP_BUILD_DIR");

    // 使用 OUT_DIR 來定位實際的構建輸出目錄 (例如 target/debug)
    // 結構通常為: target/debug/build/package-hash/out
    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());
    let bundled_dir = out_dir.ancestors().nth(3).unwrap().to_path_buf();

    std::fs::create_dir_all(&bundled_dir).unwrap();

    let build_dir = PathBuf::from(env::var("CPP_BUILD_DIR").unwrap());

    // find all .dll / .dylib / .so
    if build_dir.exists()
    {
        for entry in std::fs::read_dir(&build_dir).unwrap()
        {
            let src = entry.unwrap().path();
            if let Some(ext) = src.extension()
            {
                let ext = ext.to_string_lossy();
                if matches!(ext.as_ref(), "dll" | "dylib" | "so")
                {
                    let file_name = src.file_name().unwrap();
                    let dst = bundled_dir.join(file_name);

                    // copy the file only if modified
                    if dst.exists()
                    {
                        let src_mod = std::fs::metadata(&src).unwrap().modified().unwrap();
                        let dst_mod = std::fs::metadata(&dst).unwrap().modified().unwrap();
                        
                        if src_mod <= dst_mod {
                            println!("cargo:warning=lib copy skipped (up-to-date): {}", file_name.to_string_lossy());
                            continue;
                        }
                    }

                    std::fs::copy(&src, &dst).unwrap();
                    println!("cargo:warning=native lib copied: {} → /", file_name.to_string_lossy());
                }
            }
        }
    }
} // fn main()
