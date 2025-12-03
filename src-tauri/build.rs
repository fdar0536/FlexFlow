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
            let path = entry.unwrap().path();
            if let Some(ext) = path.extension()
            {
                let ext = ext.to_string_lossy();
                if matches!(ext.as_ref(), "dll" | "dylib" | "so")
                {
                    let file_name = path.file_name().unwrap();
                    let dest = bundled_dir.join(file_name);
                    std::fs::copy(&path, &dest).unwrap();
                    println!("cargo:warning=native lib copied: {} → /", file_name.to_string_lossy());
                }
            }
        }
    }

    // copy compile_commands.json
    let src_path = dst.join("build").join("compile_commands.json");
    let target_path = PathBuf::from("../src-cpp").join("compile_commands.json");
    std::fs::copy(&src_path, &target_path).unwrap();
    println!("cargo:warning=Copied {} → {}", src_path.display(), target_path.display());
} // fn main()
