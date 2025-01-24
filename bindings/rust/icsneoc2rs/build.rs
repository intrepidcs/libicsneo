use std::env;
use std::path::PathBuf;

pub fn main() {
    // Tell cargo to look for shared libraries in the specified directory
    println!("cargo:rustc-link-search=../../../build");

    // Tell cargo to tell rustc to link the system
    // shared library.
    println!("cargo:rustc-link-lib=icsneoc2");

    // The bindgen::Builder is the main entry point
    // to bindgen, and lets you build up options for
    // the resulting bindings.
    let bindings = bindgen::Builder::default()
        // The input header we would like to generate
        // bindings for.
        .header("../../../include/icsneo/icsneoc2.h")
        .clang_arg("-I../../../include")
        // Tell cargo to invalidate the built crate whenever any of the
        // included header files changed.
        .parse_callbacks(Box::new(bindgen::CargoCallbacks::new()))
        .derive_default(true)
        .derive_debug(true)
        .derive_partialeq(true)
        .derive_copy(true)
        .default_enum_style(bindgen::EnumVariation::Rust { non_exhaustive: true })
        .default_alias_style(bindgen::AliasVariation::TypeAlias)
        .generate_cstr(true)
        // Finish the builder and generate the bindings.
        .generate()
        // Unwrap the Result and panic on failure.
        .expect("Unable to generate bindings");

    // Write the bindings to the $OUT_DIR/bindings.rs file.
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
}