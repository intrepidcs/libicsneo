use cmake::Config;
use path_clean::{clean, PathClean};
use std::{env, path::PathBuf};

fn libicsneo_path() -> PathBuf {
    // Get the path of libicsneo
    let path = std::env::var("LIBICSNEO_PATH")
        .unwrap_or(format!("{}/../../../", env!("CARGO_MANIFEST_DIR")));
    let libicsneo_path = std::path::PathBuf::from(clean(&path));

    libicsneo_path
}

fn libicsneo_include_path() -> PathBuf {
    let path = libicsneo_path().join("include");
    path.clean()
}

fn libicsneo_header_path() -> PathBuf {
    let path = libicsneo_include_path().join("icsneo").join("icsneo.h");
    path.clean()
}

// Detects the cargo build profile, true = release, otherwise false
fn is_release_build() -> bool {
    let profile = std::env::var("PROFILE").unwrap();
    match profile.as_str() {
        "debug" => return false,
        "release" => return true,
        _ => return false,
    }
}

// returns the cmake build string that is normally passed to -DCMAKE_BUILD_TYPE=
fn cmake_build_config_type() -> String {
    let build_config_type = if is_release_build() {
        "Release"
    } else {
        if cfg!(target_os = "windows") {
            // Rust runtime is linked with /MD on windows MSVC... MSVC takes Debug and forces /MDd
            // https://www.reddit.com/r/rust/comments/dvmzo2/cargo_external_c_library_windows_debugrelease_hell/
            "RelWithDebInfo"
        } else {
            "Debug"
        }
    };
    build_config_type.to_string()
}

// Build libicsneo through cmake, returns the build directory
fn build_libicsneo() -> PathBuf {
    let libicsneo_path = libicsneo_path();
    // Check to make sure CMakeLists.txt exists
    if !libicsneo_path.join("CMakeLists.txt").exists() {
        panic!("CMakeLists.txt not found at {}", libicsneo_path.display());
    }
    let build_config_type = cmake_build_config_type();
    // Run cmake on libicsneo
    let mut config = Config::new(libicsneo_path.clone());
    let config = config
        .build_target("ALL_BUILD")
        // .define("LIBICSNEO_BUILD_ICSNEOC_STATIC:BOOL", "ON")
        // .define("LIBICSNEO_BUILD_EXAMPLES:BOOL", "OFF")
        // .define("LIBICSNEO_BUILD_ICSNEOLEGACY:BOOL", "OFF")
        .profile(&build_config_type);
    // Lets use ninja if it exists
    let config = match which::which("ninja") {
        Ok(_) => config.generator("Ninja Multi-Config").build_target("all"),
        Err(_e) => config,
    };
    config.build()
}

fn setup_linker_libs(build_path: &PathBuf) {
    let build_config_type = cmake_build_config_type();
    // output for lib path
    println!(
        "cargo:warning=build search path: {:?}",
        build_path
            .join(format!("build/{build_config_type}"))
            .display()
    );
    // icsneo lib/dll linker search path
    println!(
        "cargo:rustc-link-search=native={}",
        build_path
            .join(format!("build/{build_config_type}"))
            .display()
    );
    // fatfs linker search path and addition
    println!(
        "cargo:rustc-link-search=native={}/build/third-party/fatfs/{build_config_type}",
        build_path.display()
    );
    // libicsneo libraries
    println!("cargo:rustc-link-lib=fatfs");
    println!("cargo:rustc-link-lib=static=icsneocpp");
    if cfg!(feature = "static") {
        println!("cargo:rustc-link-lib=static=icsneo-static");
    } else {
        println!("cargo:rustc-link-lib=dylib=icsneo");
    }
    // Platform specific libraries
    match env::var("CARGO_CFG_TARGET_OS").unwrap().as_str() {
        "windows" => {
            // FTD3xx linker search path and addition
            println!(
                "cargo:rustc-link-search=native={}/build/_deps/ftdi3xx-src",
                build_path.display()
            );
            println!("cargo:rustc-link-lib=FTD3XX");
        }
        "linux" => {}
        "macos" => {
            println!("cargo:rustc-link-lib=static=icsneo-static");
            println!("cargo:rustc-link-lib=framework=IOKit");
            println!("cargo:rustc-link-lib=framework=CoreFoundation");
        }
        target_os => panic!("Target OS not supported: {target_os}"),
    }
}

fn prepare_git_submodule() {
    // We don't need to checkout the submodule if we are using a custom libicsneo path
    if std::env::var("LIBICSNEO_PATH").is_ok() {
        println!("cargo:warning=Using custom LIBICSNEO_PATH, skipping checking out submodules");
        return;
    }
    let libicsneo_path = libicsneo_path();
    // This seems to not be needed when including this as a dependency? Why?
    // checkout the submodule if needed
    let output = std::process::Command::new("git")
        .args(["submodule", "update", "--init"])
        .current_dir(libicsneo_path)
        .output()
        .expect("Failed to fetch git submodules!");
    // Make sure git was successful!
    if !output.status.success() {
        println!("cargo:warning=git return code: {}", output.status);
        let stdout = std::str::from_utf8(&output.stdout).unwrap();
        for line in stdout.split("\n") {
            println!("cargo:warning=git stdout: {}", line);
        }
        let stderr = std::str::from_utf8(&output.stderr).unwrap();
        for line in stderr.split("\n") {
            println!("cargo:warning=git stderr: {}", line);
        }
    }
}

fn generate_bindings() {
    let header = libicsneo_header_path();
    let bindings = bindgen::Builder::default()
        .header(header.to_str().unwrap())
        .default_enum_style(bindgen::EnumVariation::Rust {
            non_exhaustive: false,
        })
        .clang_args(&[format!("-I{}", libicsneo_include_path().display()).as_str()])
        .blocklist_file("stdint.h")
        .blocklist_file("stdbool.h")
        .use_core()
        // .allowlist_function("icsneo_.*")
        // .allowlist_type("neodevice_t")
        // .allowlist_type("neonetid_t")
        // .allowlist_type("neomessage_.*")
        // .allowlist_type("neoversion_t")
        // .allowlist_type("neoevent_t")
        .formatter(bindgen::Formatter::Rustfmt)
        .derive_default(true)
        .derive_debug(true)
        .derive_partialeq(true)
        .derive_copy(true)
        .parse_callbacks(Box::new(bindgen::CargoCallbacks::new()))
        //.clang_args(clang_args())
        .generate()
        .expect("Unable to generate bindings");
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    println!("cargo:warning=out_path: {:?}", out_path.display());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings");

    let out_path = std::path::PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
}

fn main() {
    let header = libicsneo_header_path();
    println!("cargo:rerun-if-changed=build.rs");
    println!("cargo:rerun-if-changed={}", header.to_str().unwrap());
    println!("cargo:rerun-if-env-changed=LIBMSVC_PATH");

    prepare_git_submodule();
    generate_bindings();
    // We can skip building if its for docs.rs
    if std::env::var("DOCS_RS").is_err() {
        let build_directory = build_libicsneo();
        setup_linker_libs(&build_directory);
    }
}
