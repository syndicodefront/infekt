// This is a rust-native wrapper around the C++ FFI stuff from infekt_core.
// Later, this can become a rust-native implementation :-)

use crate::infekt_core;
use cxx::{let_cxx_string, UniquePtr};
use std::path::Path;

pub struct NfoData {
    nfo: UniquePtr<infekt_core::ffi::CNFOData>,
}

impl NfoData {
    pub fn new() -> NfoData {
        NfoData {
            nfo: (UniquePtr::null()),
        }
    }

    pub fn load_from_file(&mut self, path: &Path) -> Result<(), String> {
        let mut load_nfo = infekt_core::ffi::new_nfo_data();
        let_cxx_string!(path_cxx = String::from(path.to_string_lossy()));

        if load_nfo.pin_mut().LoadFromFile(&path_cxx) {
            self.nfo = load_nfo;

            return Ok(());
        }

        return Err(load_nfo.pin_mut().GetLastErrorDescription().to_string());
    }

    /*
    pub fn get_charset_name(&self) -> &str {
        if self.nfo.is_null() {
            return "(none)";
        }

        match self.nfo.GetCharset() {
            NFOC_UTF16 => "UTF-16",
            NFOC_UTF8_SIG => "UTF-8 (Signature)",
            NFOC_UTF8 => "UTF-8",
            NFOC_CP437 => "CP 437",
            NFOC_CP437_IN_UTF8 => "CP 437 (in UTF-8)",
            NFOC_CP437_IN_UTF16 => "CP 437 (in UTF-16)",
            NFOC_CP437_STRICT => "CP 437 (strict mode)",
            NFOC_WINDOWS_1252 => "Windows-1252",
            NFOC_CP437_IN_CP437 => "CP 437 (double encoded)",
            NFOC_CP437_IN_CP437_IN_UTF8 => "CP 437 (double encoded + UTF-8)",
            _ => "(unknown)",
        }
    }
    */
}
