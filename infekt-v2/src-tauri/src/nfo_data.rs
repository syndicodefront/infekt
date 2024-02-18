// This is a rust-native wrapper around the C++ FFI stuff from infekt_core.
// Later, this can become a rust-native implementation :-)

use crate::infekt_core;
use crate::nfo_renderer_grid::*;
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

        if !self.renderer_grid.is_none() {
            return self.renderer_grid.as_ref();
        }

        let width = self.nfo.GetGridWidth();
        let height = self.nfo.GetGridHeight();
        // let mut has_blocks = false;

        let mut renderer_grid = NfoRendererGrid {
            width,
            height,
            lines: Vec::new(),
        };

        for row in 0..height {
            let mut line = NfoRendererLine {
                row,
                block_groups: Vec::new(),
                text_flights: Vec::new(),
                links: Vec::new(),
            };

            let mut text_started = false;
            let mut text_buf_first_col = usize::MAX;
            let mut text_buf = String::with_capacity(128);
            let mut link_url: Option<String> = None;

            for col in 0..width {
                let block_shape =
                    get_block_shape(self.nfo.GetGridCharUtf32(row, col), text_started);

                if block_shape == NfoRendererBlockShape::NoBlock
                    || block_shape == NfoRendererBlockShape::WhitespaceInText
                {
                    // It's not a block, so must be text.
                    text_started = true;

                    if text_buf_first_col == usize::MAX {
                        text_buf_first_col = col;
                    }

                    // let link = self.nfo.Get // TODO: implement link extraction
                    let now_in_link = false;
                    let buffer_is_link = link_url != None;

                    if now_in_link != buffer_is_link {
                        if !text_buf.is_empty() {
                            text_buf.truncate(text_buf.trim_end_matches(' ').len());

                            if buffer_is_link {
                                line.links.push(NfoRendererLink {
                                    col: text_buf_first_col,
                                    text: text_buf,
                                    url: link_url.unwrap(),
                                });
                            } else {
                                line.text_flights.push(NfoRendererTextFlight {
                                    col: text_buf_first_col,
                                    text: text_buf,
                                });
                            }
                        }

                        text_buf = String::with_capacity(128);
                        text_buf_first_col = col;
                        link_url = now_in_link.then(|| String::new()); // TODO: take real link URL
                    }

                    text_buf.push_str(unsafe {
                        std::str::from_utf8_unchecked(self.nfo.GetGridCharUtf8(row, col).as_bytes())
                    });
                } else if block_shape != NfoRendererBlockShape::Whitespace {
                    // It's a block!
                } else if text_started {
                    // Preserve whitespace between non-whitespace chars.
                    text_buf.push(' ');
                }
            }

            renderer_grid.lines.push(line);
        }

        self.renderer_grid = Some(renderer_grid);

        self.renderer_grid.as_ref()
    }
}
