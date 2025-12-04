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

    println!("cargo:rerun-if-changed=../src-cpp");

    let mut cfg = cmake::Config::new("../src-cpp");

    // Windows use vcpkg
    if cfg!(target_os = "windows")
    {
        let vcpkg_root = env::var("VCPKG_ROOT")
            .unwrap_or(r"D:\libs\vcpkg".to_string());

        let triplet = if cfg!(target_arch = "x86_64") {
            "x64-windows"
        }
        else
        {
            "x86-windows"
        };

        cfg.define("CMAKE_TOOLCHAIN_FILE",
            format!("{vcpkg_root}/scripts/buildsystems/vcpkg.cmake"))
           .define("VCPKG_TARGET_TRIPLET", triplet);

        cfg.generator("Ninja");
    }

    let dst = cfg.profile("Release").no_build_target(true).build();

    let profile = env::var("PROFILE").unwrap();
    let bundled_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap())
    .join("target")
    .join(profile);

    std::fs::create_dir_all(&bundled_dir).unwrap();

    let build_dir = dst.join("build");

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
                        let src_bak = src.clone();
                        let src_mod = std::fs::metadata(src_bak)
                        .unwrap().modified().unwrap();
                        let dst_bak = dst.clone();
                        let dst_mod = std::fs::metadata(dst_bak)
                        .unwrap().modified().unwrap();
                        
                        if src_mod > dst_mod
                        {
                            std::fs::copy(&src, &dst).unwrap();
                            println!("cargo:warning=lib copy skipped: {} → /", file_name.to_string_lossy());
                        }
                    }

                    std::fs::copy(&src, &dst).unwrap();
                    println!("cargo:warning=native lib copied: {} → /", file_name.to_string_lossy());
                }
            }
        }
    }
} // fn main()
