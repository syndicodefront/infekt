use std::env;
use std::path::{Path, PathBuf};

fn main() {
    let crate_path = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());

    cxx_build::bridge("src/infekt_core.rs")
        .cpp(true)
        .std("c++20")
        .include(crate_path.join(Path::new("infekt-core-cpp/include")))
        .include(crate_path.join(Path::new("../../src/lib")))
        .include(crate_path.join(Path::new("../../src/lib-posix")))
        .include(crate_path.join(Path::new("../../src/lib-win32")))
        .file("../../src/lib/ansi_art.cpp")
        .file("../../src/lib/nfo_colormap.cpp")
        .file("../../src/lib/nfo_data.cpp")
        .file("../../src/lib/nfo_hyperlink.cpp")
        .file("../../src/lib/util.cpp")
        .compile("infekt-core-cpp");

    cc::Build::new()
        .cpp(false)
        .std("c11")
        .include(crate_path.join(Path::new("../../src/lib")))
        .include(crate_path.join(Path::new("../../src/lib-posix")))
        .include(crate_path.join(Path::new("../../src/lib-win32")))
        .file("../../src/lib/forgiving_utf8.c")
        .file("../../src/lib/gutf8.c")
        .file("../../src/lib-posix/iconv_string.c")
        .compile("infekt-core-c");

    tauri_build::build();
}
