use std::env;
use std::path::PathBuf;

fn main() {
    println!("=== BUILD.RS IS EXECUTING ===");

    tauri_build::build();

    println!("cargo:rerun-if-changed=../src-cpp");

    let mut cfg = cmake::Config::new("../src-cpp");

    // Windows 用 vcpkg
    if cfg!(target_os = "windows") {
        let vcpkg_root = env::var("VCPKG_ROOT")
            .unwrap_or(r"D:\libs\vcpkg".to_string());

        let triplet = if cfg!(target_arch = "x86_64") {
            "x64-windows"
        } else {
            "x86-windows"
        };

        cfg.define("CMAKE_TOOLCHAIN_FILE",
            format!("{vcpkg_root}/scripts/buildsystems/vcpkg.cmake"))
           .define("VCPKG_TARGET_TRIPLET", triplet);

        cfg.generator("Ninja");
    }

    let dst = cfg.profile("Release").no_build_target(true).build();

    // 決定產出的檔名
    let lib_name = if cfg!(target_os = "windows") {
        "ffmodel-c.dll"
    } else if cfg!(target_os = "macos") {
        "libffmodel-c.dylib"
    } else {
        "libffmodel-c.so"
    };

    let mut src_path = dst.join("build").join(&lib_name);

    // 複製到 src-tauri 根目錄，Tauri 會自動 bundle 進去
    let mut target_path = PathBuf::from(".").join(&lib_name);
    std::fs::copy(&src_path, &target_path).unwrap();
    println!("cargo:warning=Copied {} → {}", src_path.display(), target_path.display());

    src_path = dst.join("build").join("compile_commands.json");
    target_path = PathBuf::from(".").join("compile_commands.json");
    std::fs::copy(&src_path, &target_path).unwrap();
    println!("cargo:warning=Copied {} → {}", src_path.display(), target_path.display());
}
