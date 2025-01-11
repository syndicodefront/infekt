use std::env;
use std::path::{Path, PathBuf};

fn compile_infekt_cpp() {
    let crate_path = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());

    cxx_build::bridge("src/core/cpp.rs")
        .cpp(true)
        .std("c++20")
        .include(crate_path.join(Path::new("infekt-core-cpp/include")))
        .include(crate_path.join(Path::new("../src/lib")))
        .include(crate_path.join(Path::new("../src/lib-posix")))
        .include(crate_path.join(Path::new("../src/lib-win32")))
        .file("../src/lib/ansi_art.cpp")
        .file("../src/lib/nfo_colormap.cpp")
        .file("../src/lib/nfo_data.cpp")
        .file("../src/lib/nfo_hyperlink.cpp")
        .file("../src/lib/util.cpp")
        .flag_if_supported("-Wno-type-limits")
        .flag_if_supported("-fopenmp")
        .compile("infekt-core-cpp");

    println!("cargo:rustc-link-arg=-fopenmp");

    let mut c_build = cc::Build::new();
    let c_build = c_build
        .cpp(false)
        .std("c11")
        .include(crate_path.join(Path::new("../src/lib")))
        .include(crate_path.join(Path::new("../src/lib-posix")))
        .include(crate_path.join(Path::new("../src/lib-win32")))
        .file("../src/lib/forgiving_utf8.c")
        .file("../src/lib/gutf8.c");

    #[cfg(not(target_os = "windows"))]
    {
        c_build.file("../src/lib-posix/iconv_string.c");
    }

    c_build.compile("infekt-core-c");
}

fn main() {
    println!("cargo:rustc-env=MACOSX_DEPLOYMENT_TARGET=10.12");

    compile_infekt_cpp();
}
