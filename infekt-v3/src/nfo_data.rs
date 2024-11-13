// This is a rust-native wrapper around the C++ FFI stuff from infekt_core.
// Later, this can become a rust-native implementation :-)

use crate::infekt_core;
use crate::nfo_renderer_grid::*;
use crate::nfo_to_html::nfo_to_html_classic;
use cxx::{let_cxx_string, UniquePtr};
use infekt_core::ffi::ENfoCharset;
use std::path::Path;

pub struct NfoData {
    nfo: UniquePtr<infekt_core::ffi::CNFOData>,
    renderer_grid: Option<NfoRendererGrid>,
}

impl NfoData {
    pub fn new() -> NfoData {
        NfoData {
            nfo: (UniquePtr::null()),
            renderer_grid: None,
        }
    }

    pub fn is_loaded(&self) -> bool {
        !self.nfo.is_null()
    }

    pub fn load_from_file(&mut self, path: &Path) -> Result<(), String> {
        let mut load_nfo = infekt_core::ffi::new_nfo_data();
        let_cxx_string!(path_cxx = String::from(path.to_string_lossy()));

        if load_nfo.pin_mut().LoadFromFile(&path_cxx) {
            self.nfo = load_nfo;
            self.renderer_grid = None;

            return Ok(());
        }

        Err(load_nfo.pin_mut().GetLastErrorDescription().to_string())
    }

    pub fn get_charset_name(&self) -> &str {
        if self.nfo.is_null() {
            return "(none)";
        }

        match self.nfo.GetCharset() {
            ENfoCharset::NFOC_UTF16 => "UTF-16",
            ENfoCharset::NFOC_UTF8_SIG => "UTF-8 (Signature)",
            ENfoCharset::NFOC_UTF8 => "UTF-8",
            ENfoCharset::NFOC_CP437 => "CP 437",
            ENfoCharset::NFOC_CP437_IN_UTF8 => "CP 437 (in UTF-8)",
            ENfoCharset::NFOC_CP437_IN_UTF16 => "CP 437 (in UTF-16)",
            ENfoCharset::NFOC_CP437_STRICT => "CP 437 (strict mode)",
            ENfoCharset::NFOC_WINDOWS_1252 => "Windows-1252",
            ENfoCharset::NFOC_CP437_IN_CP437 => "CP 437 (double encoded)",
            ENfoCharset::NFOC_CP437_IN_CP437_IN_UTF8 => "CP 437 (double encoded + UTF-8)",
            _ => "(unknown)",
        }
    }

    pub fn get_renderer_grid(&mut self) -> Option<&NfoRendererGrid> {
        if self.nfo.is_null() {
            return None;
        }

        if self.renderer_grid.is_none() {
            self.renderer_grid = Some(make_renderer_grid(&self.nfo));
        }

        self.renderer_grid.as_ref()
    }

    pub fn get_classic_html(&self) -> String {
        if self.nfo.is_null() {
            return String::new();
        }

        nfo_to_html_classic(&self.nfo)
    }

    pub fn get_classic_text(&self) -> String {
        if self.nfo.is_null() {
            return String::new();
        }

        self.nfo.GetTextUtf8().to_string()
    }

    pub fn get_stripped_html(&self) -> String {
        if self.nfo.is_null() {
            return String::new();
        }

        let mut load_stripped_nfo = infekt_core::ffi::new_nfo_data();

        if load_stripped_nfo.pin_mut().LoadStripped(&self.nfo) {
            return nfo_to_html_classic(&load_stripped_nfo);
        }

        String::new()
    }

    pub fn get_stripped_text(&self) -> String {
        if self.nfo.is_null() {
            return String::new();
        }

        let mut load_stripped_nfo = infekt_core::ffi::new_nfo_data();

        if load_stripped_nfo.pin_mut().LoadStripped(&self.nfo) {
            return load_stripped_nfo.GetTextUtf8().to_string();
        }

        String::new()
    }
}
