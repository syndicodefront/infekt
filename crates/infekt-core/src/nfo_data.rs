use std::path::{Path, PathBuf};
use std::sync::OnceLock;

use regex::Regex;

use super::codepage_437;
use super::nfo_renderer_grid::{NfoRendererGrid, make_renderer_grid};

const SIZE_LIMIT: u64 = 1024 * 1024 * 3;
const LINES_LIMIT: usize = 10_000;
const WIDTH_LIMIT: usize = 2_000;
const SAUCE_RECORD_SIZE: usize = 128;
const SAUCE_COMMENT_LINE_SIZE: usize = 64;
const SAUCE_HEADER_ID_SIZE: usize = 5;
const SAUCE_MAX_COMMENTS: u8 = 255;
const SAUCE_EOF: u8 = 0x1A;
pub const UTF8_SIGNATURE: [u8; 3] = [0xEF, 0xBB, 0xBF];
pub const UTF16_LE_BOM: [u8; 2] = [0xFF, 0xFE];
pub const UTF16_BE_BOM: [u8; 2] = [0xFE, 0xFF];

#[allow(dead_code)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum NfoCharset {
    Auto,
    Utf16,
    Utf8Signature,
    Utf8,
    Cp437,
    Cp437InUtf8,
    Cp437InUtf16,
    Cp437Strict,
    Windows1252,
    Cp437InCp437,
    Cp437InCp437InUtf8,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum DecodeApproach {
    False,
    Try,
    Force,
}

#[derive(Clone, Debug)]
pub(super) struct NfoHyperLink {
    row: usize,
    col_start: usize,
    col_end: usize,
    href: String,
}

#[derive(Clone, Debug)]
struct SauceInfo {
    data_len: usize,
    is_ansi: bool,
    ansi_hint_width: usize,
    ansi_hint_height: usize,
}

pub struct NfoData {
    loaded: bool,
    text_content: String,
    grid: Vec<Vec<char>>,
    grid_width: usize,
    renderer_grid: Option<NfoRendererGrid>,
    file_path: Option<PathBuf>,
    source_charset: NfoCharset,
    line_wrap: bool,
    is_ansi: bool,
    ansi_hint_width: usize,
    ansi_hint_height: usize,
    hyperlinks: Vec<NfoHyperLink>,
    cached_classic_text: Option<String>,
    cached_stripped_text: Option<String>,
    last_error: String,
}

impl NfoData {
    pub fn new() -> NfoData {
        NfoData {
            loaded: false,
            text_content: String::new(),
            grid: Vec::new(),
            grid_width: 0,
            renderer_grid: None,
            file_path: None,
            source_charset: NfoCharset::Auto,
            line_wrap: false,
            is_ansi: false,
            ansi_hint_width: 0,
            ansi_hint_height: 0,
            hyperlinks: Vec::new(),
            cached_classic_text: None,
            cached_stripped_text: None,
            last_error: String::new(),
        }
    }

    pub fn is_loaded(&self) -> bool {
        self.loaded
    }

    pub fn has_blocks(&self) -> bool {
        self.renderer_grid
            .as_ref()
            .is_some_and(|grid| grid.has_blocks)
    }

    pub fn get_file_path(&self) -> Option<&Path> {
        self.file_path.as_deref()
    }

    pub fn get_file_name(&self) -> Option<String> {
        self.file_path.as_ref().map(|p| {
            p.file_name()
                .unwrap_or_default()
                .to_string_lossy()
                .to_string()
        })
    }

    pub fn load_from_file(&mut self, path: &Path) -> Result<(), String> {
        self.clear_loaded_state();

        let metadata = std::fs::metadata(path).map_err(|err| {
            format!(
                "Unable to open NFO file '{}' (error {err})",
                path.to_string_lossy()
            )
        })?;

        if !metadata.is_file() {
            return Err("stat() on NFO file failed.".to_string());
        }

        if metadata.len() > SIZE_LIMIT {
            return Err("NFO file is too large (> 3 MB)".to_string());
        }

        let data = std::fs::read(path)
            .map_err(|_| "An error occured while reading from the NFO file.".to_string())?;

        self.file_path = Some(path.to_path_buf());

        match self.load_from_memory_internal(&data) {
            Ok(()) => {
                self.loaded = true;
                self.cached_classic_text = Some(self.text_content.clone());
                self.cached_stripped_text = Some(make_stripped_text(&self.text_content));
                self.renderer_grid = Some(make_renderer_grid(self));
                Ok(())
            }
            Err(err) => {
                self.file_path = None;
                self.loaded = false;
                self.last_error = err.clone();
                Err(err)
            }
        }
    }

    pub fn get_charset_name(&self) -> &str {
        if !self.loaded {
            return "(none)";
        }

        match self.source_charset {
            NfoCharset::Auto => "(auto)",
            NfoCharset::Utf16 => "UTF-16",
            NfoCharset::Utf8Signature => "UTF-8 (Signature)",
            NfoCharset::Utf8 => "UTF-8",
            NfoCharset::Cp437 => "CP 437",
            NfoCharset::Cp437InUtf8 => "CP 437 (in UTF-8)",
            NfoCharset::Cp437InUtf16 => "CP 437 (in UTF-16)",
            NfoCharset::Cp437Strict => "CP 437 (strict mode)",
            NfoCharset::Windows1252 => "Windows-1252",
            NfoCharset::Cp437InCp437 => "CP 437 (double encoded)",
            NfoCharset::Cp437InCp437InUtf8 => "CP 437 (double encoded + UTF-8)",
        }
    }

    pub fn get_renderer_grid(&self) -> Option<&NfoRendererGrid> {
        if !self.loaded {
            return None;
        }

        self.renderer_grid.as_ref()
    }

    pub fn get_classic_text(&self) -> String {
        if !self.loaded {
            return String::new();
        }

        self.cached_classic_text.clone().unwrap_or_default()
    }

    pub fn get_stripped_text(&self) -> String {
        if !self.loaded {
            return String::new();
        }

        self.cached_stripped_text.clone().unwrap_or_default()
    }

    pub fn get_cp437_bytes(&self) -> (Vec<u8>, usize) {
        if !self.loaded {
            return (Vec::new(), 0);
        }

        encode_cp437_text(&self.text_content)
    }

    pub(super) fn grid_width(&self) -> usize {
        self.grid_width
    }

    pub(super) fn grid_height(&self) -> usize {
        self.grid.len()
    }

    pub(super) fn grid_char(&self, row: usize, col: usize) -> Option<char> {
        self.grid.get(row).and_then(|line| line.get(col)).copied()
    }

    pub(super) fn link_url(&self, row: usize, col: usize) -> Option<&str> {
        self.hyperlinks
            .iter()
            .find(|link| link.row == row && col >= link.col_start && col <= link.col_end)
            .map(|link| link.href.as_str())
    }

    fn clear_loaded_state(&mut self) {
        self.loaded = false;
        self.text_content.clear();
        self.grid.clear();
        self.grid_width = 0;
        self.renderer_grid = None;
        self.file_path = None;
        self.source_charset = NfoCharset::Auto;
        self.is_ansi = false;
        self.ansi_hint_width = 0;
        self.ansi_hint_height = 0;
        self.hyperlinks.clear();
        self.cached_classic_text = None;
        self.cached_stripped_text = None;
        self.last_error.clear();
    }

    fn load_from_memory_internal(&mut self, data: &[u8]) -> Result<(), String> {
        let sauce = self.read_sauce(data)?;
        self.is_ansi = sauce.is_ansi;
        self.ansi_hint_width = sauce.ansi_hint_width;
        self.ansi_hint_height = sauce.ansi_hint_height;
        let data = &data[..sauce.data_len];

        let loaded = match self.source_charset {
            NfoCharset::Auto => {
                self.try_load_utf8_signature(data)
                    || self.try_load_utf16_le(data, DecodeApproach::Try)
                    || self.try_load_utf16_be(data)
                    || (self.has_nfo_or_diz_extension()
                        && self.try_load_utf8(data, DecodeApproach::Try))
                    || self.try_load_cp437(data, DecodeApproach::Try)
            }
            NfoCharset::Utf16 => {
                self.try_load_utf16_le(data, DecodeApproach::False) || self.try_load_utf16_be(data)
            }
            NfoCharset::Utf8Signature => self.try_load_utf8_signature(data),
            NfoCharset::Utf8 => self.try_load_utf8(data, DecodeApproach::False),
            NfoCharset::Cp437 => self.try_load_cp437(data, DecodeApproach::False),
            NfoCharset::Windows1252 => self.try_load_windows_1252(data),
            NfoCharset::Cp437InUtf8 => self.try_load_utf8(data, DecodeApproach::Force),
            NfoCharset::Cp437InUtf16 => self.try_load_utf16_le(data, DecodeApproach::Force),
            NfoCharset::Cp437InCp437 => self.try_load_cp437(data, DecodeApproach::Force),
            NfoCharset::Cp437Strict => self.try_load_cp437_strict(data),
            NfoCharset::Cp437InCp437InUtf8 => false,
        };

        if !loaded {
            self.text_content.clear();
            return Err(if self.last_error.is_empty() {
                "There appears to be a charset/encoding problem.".to_string()
            } else {
                self.last_error.clone()
            });
        }

        self.post_process_loaded_content()
    }

    fn post_process_loaded_content(&mut self) -> Result<(), String> {
        if self.is_ansi {
            return Err("Internal problem during ANSI processing. This could be a bug, please file a report and attach the file you were trying to open.".to_string());
        }

        if self.source_charset != NfoCharset::Cp437Strict {
            normalize_whitespace(&mut self.text_content);
            fix_ansi_escape_codes(&mut self.text_content);
        }

        let (mut lines, mut max_line_len) = split_into_lines(&self.text_content);

        if self.source_charset != NfoCharset::Cp437Strict {
            fix_lflf(&mut self.text_content, &mut lines);
            max_line_len = lines
                .iter()
                .map(|line| line.chars().count())
                .max()
                .unwrap_or(1);
        }

        if self.line_wrap {
            wrap_long_lines(&mut lines);
            max_line_len = lines
                .iter()
                .map(|line| line.chars().count())
                .max()
                .unwrap_or(1);
        }

        if lines.is_empty() || max_line_len == 0 {
            return Err("Unable to find any lines in this file.".to_string());
        }

        if max_line_len > WIDTH_LIMIT {
            return Err(format!(
                "This file contains a line longer than {WIDTH_LIMIT} chars. To prevent damage and lock-ups, we do not load it."
            ));
        }

        if lines.len() > LINES_LIMIT {
            return Err(format!(
                "This file contains more than {LINES_LIMIT} lines. To prevent damage and lock-ups, we do not load it."
            ));
        }

        self.grid_width = max_line_len;
        self.grid = lines.iter().map(|line| line.chars().collect()).collect();
        self.hyperlinks = find_hyperlinks(&lines);

        Ok(())
    }

    fn try_load_utf8_signature(&mut self, data: &[u8]) -> bool {
        if data.len() < UTF8_SIGNATURE.len() || data[..UTF8_SIGNATURE.len()] != UTF8_SIGNATURE {
            return false;
        }

        if self.try_load_utf8(&data[UTF8_SIGNATURE.len()..], DecodeApproach::Try) {
            if self.source_charset == NfoCharset::Utf8 {
                self.source_charset = NfoCharset::Utf8Signature;
            }
            true
        } else {
            false
        }
    }

    fn try_load_utf8(&mut self, data: &[u8], approach: DecodeApproach) -> bool {
        let Ok(utf) = std::str::from_utf8(data) else {
            return false;
        };

        let looks_like_cp437_in_utf8 = (approach == DecodeApproach::Force)
            || (approach == DecodeApproach::Try
                && (utf.contains('\u{00DF}') || utf.contains('\u{00CD}'))
                && (utf.contains("\u{00DC}\u{00DC}") || utf.contains("\u{00DB}\u{00DB}"))
                && (utf.contains('\u{00B1}') || utf.contains('\u{00B2}')))
            || (approach == DecodeApproach::Try
                && utf.contains("\u{009A}\u{009A}")
                && utf.contains("\u{00E1}\u{00E1}"));

        if looks_like_cp437_in_utf8 {
            let cp437 = utf8_to_latin9(data);
            if !cp437.is_empty() && self.try_load_cp437(&cp437, DecodeApproach::Try) {
                self.source_charset = if self.source_charset == NfoCharset::Cp437InCp437 {
                    NfoCharset::Cp437InCp437InUtf8
                } else {
                    NfoCharset::Cp437InUtf8
                };
                return true;
            }

            return false;
        }

        self.text_content = utf.to_string();
        self.source_charset = NfoCharset::Utf8;
        true
    }

    fn try_load_utf16_le(&mut self, data: &[u8], approach: DecodeApproach) -> bool {
        if data.len() < UTF16_LE_BOM.len() || data[..UTF16_LE_BOM.len()] != UTF16_LE_BOM {
            return false;
        }

        let raw = &data[UTF16_LE_BOM.len()..];

        // The current C++ reference is built on a platform where wchar_t is 32-bit.
        // For UTF-16LE ASCII files it rejects the UTF-16 path and falls back to CP437,
        // producing the historically expected spaced output. Preserve that behavior.
        let raw_has_ascii_letters = raw.iter().any(|b| b.is_ascii_alphabetic());
        let faux_wchar_has_ascii_letters = raw
            .chunks_exact(4)
            .filter_map(|chunk| {
                char::from_u32(u32::from_le_bytes([chunk[0], chunk[1], chunk[2], chunk[3]]))
            })
            .any(|c| c.is_ascii_alphabetic());

        if raw_has_ascii_letters && !faux_wchar_has_ascii_letters {
            return false;
        }

        let units: Vec<u16> = raw
            .chunks_exact(2)
            .map(|chunk| u16::from_le_bytes([chunk[0], chunk[1]]))
            .collect();

        let Ok(decoded) = String::from_utf16(&units) else {
            self.last_error = "Unrecognized file format or broken file.".to_string();
            return false;
        };

        if decoded.contains('\0') {
            self.last_error = "Unrecognized file format or broken file.".to_string();
            return false;
        }

        if approach == DecodeApproach::Force
            || (approach == DecodeApproach::Try
                && (decoded.contains('\u{00DF}') || decoded.contains('\u{00CD}'))
                && (decoded.contains("\u{00DC}\u{00DC}") || decoded.contains("\u{00DB}\u{00DB}"))
                && (decoded.contains('\u{00B1}') || decoded.contains('\u{00B2}')))
        {
            let cp437 = decoded
                .chars()
                .filter_map(|c| {
                    if (c as u32) <= 0xFF {
                        Some(c as u8)
                    } else {
                        None
                    }
                })
                .collect::<Vec<_>>();

            if !cp437.is_empty() && self.try_load_cp437(&cp437, DecodeApproach::False) {
                self.source_charset = NfoCharset::Cp437InUtf16;
                return true;
            }

            return false;
        }

        self.text_content = decoded;
        self.source_charset = NfoCharset::Utf16;
        true
    }

    fn try_load_utf16_be(&mut self, _data: &[u8]) -> bool {
        // Matches the current non-Windows C++ reference, where this path is disabled
        // because wchar_t is not a 16-bit type.
        false
    }

    fn try_load_cp437(&mut self, data: &[u8], mut approach: DecodeApproach) -> bool {
        let mut contains_lf = false;
        let mut contains_crlf = false;

        for (i, &byte) in data.iter().enumerate() {
            if byte == b'\n' {
                contains_lf = true;
                if i > 0 && data[i - 1] == b'\r' {
                    contains_crlf = true;
                }
            } else if approach == DecodeApproach::Try
                && i > 0
                && data.first() != Some(&0x1B)
                && ((byte == 0x9A && data[i - 1] == 0x9A)
                    || (byte == 0xFD && data[i - 1] == 0xFD)
                    || (byte == 0xE1 && data[i - 1] == 0xE1))
            {
                approach = DecodeApproach::Force;
            }

            if contains_crlf && approach != DecodeApproach::Try {
                break;
            }
        }

        let mut end = data.len();
        while end > 0 && data[end - 1] == 0 {
            end -= 1;
        }

        let mut start = 0;
        if end >= UTF8_SIGNATURE.len()
            && approach == DecodeApproach::Try
            && data[..UTF8_SIGNATURE.len()] == UTF8_SIGNATURE
        {
            start = UTF8_SIGNATURE.len();
        }

        let data = &data[start..end];
        let mut found_binary = false;
        let mut text = String::with_capacity(data.len());

        for (i, &byte) in data.iter().enumerate() {
            let ch = if byte >= 0x7F {
                if approach != DecodeApproach::Force {
                    cp437_high(byte)
                } else {
                    let temp = cp437_high(byte);
                    if (temp as u32) >= 0x7F && (temp as u32) <= 0xFF {
                        cp437_high(temp as u8)
                    } else {
                        temp
                    }
                }
            } else if byte <= 0x1F {
                if byte == 0 {
                    found_binary = true;
                    ' '
                } else if byte == 0x0D && i < data.len() - 1 && data[i + 1] == 0x0A {
                    '\r'
                } else if byte == 0x0D
                    && i < data.len() - 2
                    && data[i + 1] == 0x0D
                    && data[i + 2] == 0x0A
                {
                    ' '
                } else if byte == 0x0D && (!contains_lf || contains_crlf) {
                    '\n'
                } else {
                    cp437_control(byte)
                }
            } else {
                let mut ch = byte as char;
                if approach == DecodeApproach::Force && matches!(byte, 0x55 | 0x59 | 0x5F) {
                    let next = data.get(i + 1).copied().unwrap_or(0);
                    let prev = if i > 0 { data[i - 1] } else { 0 };

                    let next_or_prev_wordish = next.is_ascii_lowercase()
                        || prev.is_ascii_lowercase()
                        || (next.is_ascii_uppercase() && !matches!(next, b'U' | b'Y' | b'_'))
                        || (prev.is_ascii_uppercase() && !matches!(prev, b'U' | b'Y' | b'_'))
                        || next.is_ascii_digit()
                        || prev.is_ascii_digit();

                    if !next_or_prev_wordish {
                        ch = match byte {
                            0x55 => '\u{2588}',
                            0x59 => '\u{258C}',
                            0x5F => '\u{2590}',
                            _ => ch,
                        };
                    }
                }
                ch
            };

            text.push(ch);
        }

        if found_binary && !self.is_ansi && binary_text_re().is_match(&text) {
            self.last_error = "Unrecognized file format or broken file.".to_string();
            false
        } else {
            self.text_content = text;
            self.source_charset = if approach == DecodeApproach::Force {
                NfoCharset::Cp437InCp437
            } else {
                NfoCharset::Cp437
            };
            true
        }
    }

    fn try_load_cp437_strict(&mut self, data: &[u8]) -> bool {
        let mut text = String::with_capacity(data.len());

        for (i, &byte) in data.iter().enumerate() {
            let ch = if byte >= 0x7F {
                cp437_strict_high(byte)
            } else if byte <= 0x1F {
                if byte == 0 {
                    self.last_error = "Unrecognized file format or broken file.".to_string();
                    return false;
                } else if byte == 0x0D && i < data.len() - 1 && data[i + 1] == 0x0A {
                    '\r'
                } else if byte == 0x0A && i > 0 && data[i - 1] == 0x0D {
                    '\n'
                } else {
                    cp437_strict_control(byte)
                }
            } else {
                byte as char
            };

            text.push(ch);
        }

        self.text_content = text;
        self.source_charset = NfoCharset::Cp437Strict;
        true
    }

    fn try_load_windows_1252(&mut self, data: &[u8]) -> bool {
        self.text_content = data.iter().map(|&b| b as char).collect();
        !self.text_content.is_empty()
    }

    fn read_sauce(&mut self, data: &[u8]) -> Result<SauceInfo, String> {
        if data.len() <= SAUCE_RECORD_SIZE {
            return Ok(SauceInfo {
                data_len: data.len(),
                is_ansi: false,
                ansi_hint_width: 0,
                ansi_hint_height: 0,
            });
        }

        let mut record = [0u8; SAUCE_RECORD_SIZE];
        record.copy_from_slice(&data[data.len() - SAUCE_RECORD_SIZE..]);
        let mut incomplete = false;
        let mut record_len = SAUCE_RECORD_SIZE;

        if &record[0..5] != b"SAUCE" {
            for i in 0..SAUCE_RECORD_SIZE - b"SAUCE00".len() {
                let start = data.len() - SAUCE_RECORD_SIZE + i;
                if data[start..].starts_with(b"SAUCE00") {
                    incomplete = true;
                    record_len = SAUCE_RECORD_SIZE - i;
                    record = [0u8; SAUCE_RECORD_SIZE];
                    record[..record_len].copy_from_slice(&data[start..start + record_len]);
                    break;
                }
            }

            if !incomplete {
                return Ok(SauceInfo {
                    data_len: data.len(),
                    is_ansi: false,
                    ansi_hint_width: 0,
                    ansi_hint_height: 0,
                });
            }
        }

        if &record[5..7] != b"00" {
            return Err("SAUCE: Unsupported file version.".to_string());
        }

        let data_type = record[94];
        let file_type = record[95];
        let comments = record[104];

        if data_type != 1 || (file_type != 0 && file_type != 1) {
            if incomplete && data_type == 0 && file_type == 0 {
                let mut len = data.len() - record_len;
                if len > 0 && data[len - 1] == SAUCE_EOF {
                    len -= 1;
                }
                return Ok(SauceInfo {
                    data_len: len,
                    is_ansi: true,
                    ansi_hint_width: 0,
                    ansi_hint_height: 0,
                });
            }

            if data_type == 1 && file_type == b' ' && comments == b' ' {
                let mut len = data.len() - record_len;
                if len > 0 && data[len - 1] == SAUCE_EOF {
                    len -= 1;
                }
                return Ok(SauceInfo {
                    data_len: len,
                    is_ansi: false,
                    ansi_hint_width: 0,
                    ansi_hint_height: 0,
                });
            }

            return Err("SAUCE: Unsupported file format type.".to_string());
        }

        let bytes_to_trim = record_len
            + if comments > 0 {
                comments as usize * SAUCE_COMMENT_LINE_SIZE + SAUCE_HEADER_ID_SIZE
            } else {
                0
            };

        if comments == SAUCE_MAX_COMMENTS || data.len() < bytes_to_trim {
            return Err("SAUCE: Bad comments definition.".to_string());
        }

        let mut data_len = data.len() - bytes_to_trim;
        while data_len > 0 && data[data_len - 1] == SAUCE_EOF {
            data_len -= 1;
        }

        let width = u16::from_le_bytes([record[96], record[97]]) as usize;
        let height = u16::from_le_bytes([record[98], record[99]]) as usize;

        Ok(SauceInfo {
            data_len,
            is_ansi: file_type == 1 || (incomplete && file_type == 0),
            ansi_hint_width: if width > 0 && width < WIDTH_LIMIT * 2 {
                width
            } else {
                0
            },
            ansi_hint_height: if height > 0 && height < LINES_LIMIT * 2 {
                height
            } else {
                0
            },
        })
    }

    fn has_nfo_or_diz_extension(&self) -> bool {
        self.file_path
            .as_ref()
            .and_then(|path| path.extension())
            .and_then(|extension| extension.to_str())
            .map(|extension| {
                extension.eq_ignore_ascii_case("nfo") || extension.eq_ignore_ascii_case("diz")
            })
            .unwrap_or(false)
    }
}

impl Default for NfoData {
    fn default() -> Self {
        Self::new()
    }
}

fn normalize_whitespace(text: &mut String) {
    trim_right_chars(text, "\t\r\n ");

    let mut normalized = String::with_capacity(text.len() + 1);
    for ch in text.chars() {
        match ch {
            '\r' => {}
            '\t' => normalized.push_str("        "),
            '\u{00A0}' => normalized.push(' '),
            _ => normalized.push(ch),
        }
    }

    normalized.push('\n');
    *text = normalized;
}

fn split_into_lines(text: &str) -> (Vec<String>, usize) {
    let mut lines = Vec::new();
    let mut max_line_len = 1usize;

    for line in text.split_terminator('\n') {
        let mut line = line.to_string();
        trim_right_chars(&mut line, "\t\r\n ");
        max_line_len = max_line_len.max(line.chars().count());
        lines.push(line);
    }

    (lines, max_line_len)
}

fn fix_lflf(text: &mut String, lines: &mut Vec<String>) {
    let mut even_empty = 0usize;
    let mut odd_empty = 0usize;

    for (idx, line) in lines.iter().enumerate() {
        if line.is_empty() {
            if idx % 2 == 0 {
                even_empty += 1;
            } else {
                odd_empty += 1;
            }
        }
    }

    let len = lines.len() as f64;
    let kill = if (even_empty as f64) <= 0.1 * len
        && (odd_empty as f64) > 0.4 * len
        && (odd_empty as f64) < 0.6 * len
    {
        Some(1usize)
    } else if (odd_empty as f64) <= 0.1 * len
        && (even_empty as f64) > 0.4 * len
        && (even_empty as f64) < 0.6 * len
    {
        Some(0usize)
    } else {
        None
    };

    if let Some(kill) = kill {
        let mut new_lines = Vec::new();
        let mut new_content = String::with_capacity(text.len());

        for (idx, line) in lines.iter().enumerate() {
            if !line.is_empty() || idx % 2 != kill {
                new_content.push_str(line);
                new_content.push('\n');
                new_lines.push(line.clone());
            }
        }

        *lines = new_lines;
        *text = new_content;
    }
}

fn fix_ansi_escape_codes(text: &mut String) {
    let chars: Vec<char> = text.chars().collect();
    let mut out = String::with_capacity(text.len());
    let mut idx = 0usize;

    while idx < chars.len() {
        let mut go = false;
        let mut indicator_idx = idx;

        if chars[idx] == '\u{00A2}' {
            go = true;
        } else if chars[idx] == '\u{2190}' && idx + 1 < chars.len() && chars[idx + 1] == '[' {
            go = true;
            indicator_idx += 1;
        }

        if go {
            let mut p = indicator_idx + 1;
            let mut num_buf = String::new();

            while p < chars.len() && (chars[p].is_ascii_digit() || chars[p] == ';') {
                num_buf.push(chars[p]);
                p += 1;
            }

            let final_char = chars.get(p).copied().unwrap_or('\0');

            if !num_buf.is_empty() && final_char != '\0' {
                let number = num_buf
                    .split(';')
                    .next()
                    .and_then(|n| n.parse::<usize>().ok())
                    .unwrap_or(0)
                    .clamp(1, 1024);

                if final_char == 'C' {
                    out.extend(std::iter::repeat_n(' ', number));
                }

                idx = p + 1;
                continue;
            } else if num_buf.is_empty()
                && (matches!(final_char, 'A'..='G' | 'J' | 'K' | 'S' | 'T' | 's' | 'u'))
            {
                idx = p + 1;
                continue;
            } else if chars[idx] == '\u{00A2}' {
                out.push(chars[idx]);
                idx += 1;
                continue;
            }
        }

        out.push(chars[idx]);
        idx += 1;
    }

    *text = out;
}

fn wrap_long_lines(lines: &mut Vec<String>) {
    const MAX_LEN_SOFT: usize = 100;
    const MAX_LEN_HARD: usize = 160;
    const EQUAL_CONSECUTIVE_CHARACTERS_MAX: usize = 3;

    fn is_candidate(line: &str) -> bool {
        line.chars().count() > MAX_LEN_SOFT
            && !line.chars().any(|c| {
                matches!(
                    c,
                    '\u{2580}'
                        | '\u{2584}'
                        | '\u{2588}'
                        | '\u{258C}'
                        | '\u{2590}'
                        | '\u{2591}'
                        | '\u{2592}'
                        | '\u{2593}'
                )
            })
    }

    if !lines.iter().any(|line| is_candidate(line)) {
        return;
    }

    let mut new_lines = Vec::new();

    for line in lines.iter() {
        if !is_candidate(line) {
            new_lines.push(line.clone());
            continue;
        }

        let leading_spaces = line.chars().take_while(|&c| c == ' ').count();
        let mut equal_count = 0usize;
        let mut current = '\0';

        for ch in line.chars().skip(leading_spaces) {
            if ch == current {
                equal_count += 1;
                if equal_count > EQUAL_CONSECUTIVE_CHARACTERS_MAX {
                    break;
                }
            } else {
                current = ch;
                equal_count = 1;
            }
        }

        if line.chars().count() <= MAX_LEN_HARD && equal_count > EQUAL_CONSECUTIVE_CHARACTERS_MAX {
            new_lines.push(line.clone());
            continue;
        }

        wrap_line(line, leading_spaces, &mut new_lines);
    }

    *lines = new_lines;
}

fn wrap_line(line: &str, leading_spaces: usize, out: &mut Vec<String>) {
    const MAX_LEN_SOFT: usize = 100;
    let mut remaining = line.to_string();
    let mut first_run = true;

    while !remaining.is_empty() {
        let chars: Vec<char> = remaining.chars().collect();
        let mut cut = chars
            .iter()
            .take(MAX_LEN_SOFT + 1)
            .enumerate()
            .rev()
            .find_map(|(idx, &ch)| (ch == ' ').then_some(idx));

        if cut.is_none()
            || cut.unwrap() < leading_spaces
            || cut == Some(0)
            || chars.len() < MAX_LEN_SOFT
        {
            cut = Some(MAX_LEN_SOFT.min(chars.len()));
        }

        let cut = cut.unwrap();
        let mut new_line: String = chars[..cut].iter().collect();

        if !first_run {
            trim_left_chars(&mut new_line, " ");
            new_line.insert_str(0, &" ".repeat(leading_spaces + 2));
        }

        out.push(new_line);

        let erase_count = if cut != MAX_LEN_SOFT && chars.get(cut) == Some(&' ') {
            cut + 1
        } else {
            cut
        };

        remaining = chars[erase_count..].iter().collect();
        first_run = false;
    }
}

fn find_hyperlinks(lines: &[String]) -> Vec<NfoHyperLink> {
    let mut result = Vec::new();
    let mut prev_link_url = String::new();
    let mut prev_link_index: Option<usize> = None;
    let mut max_link_id = 1usize;

    for (row, line) in lines.iter().enumerate() {
        let mut link_pos = None;
        let mut prev_url_copy = prev_link_url.clone();
        let mut offset = 0usize;

        while let Some(found) = find_link(line, offset, &prev_url_copy) {
            link_pos = Some(found.pos);
            let link_id = if found.continued {
                max_link_id.saturating_sub(1)
            } else {
                max_link_id
            };

            let index = result.len();
            result.push(NfoHyperLink {
                row,
                col_start: found.pos,
                col_end: found.pos + found.len.saturating_sub(1),
                href: found.url.clone(),
            });

            if !found.continued {
                max_link_id = link_id + 1;
                prev_link_url = found.url;
                prev_link_index = Some(index);
            } else {
                if let Some(prev_idx) = prev_link_index
                    && let Some(prev) = result.get_mut(prev_idx)
                {
                    prev.href = found.url.clone();
                }
                prev_link_url.clear();
            }

            offset = found.next_offset;
            prev_url_copy.clear();
        }

        if link_pos.is_none() {
            prev_link_url.clear();
        }
    }

    result
}

struct FoundLink {
    pos: usize,
    len: usize,
    next_offset: usize,
    url: String,
    continued: bool,
}

fn find_link(line: &str, offset_chars: usize, prev_line_link: &str) -> Option<FoundLink> {
    if !line.contains(['.', '/', '&', '?', '%']) {
        return None;
    }

    let offset_byte = char_to_byte_idx(line, offset_chars);
    let line_remainder = &line[offset_byte..];
    let mut link_start: Option<usize> = None;
    let mut match_continues_link = false;
    let mut match_is_mail_link = false;
    let mut match_link_len = 0usize;

    for trigger in link_triggers() {
        if trigger.continuation && (prev_line_link.is_empty() || link_start.is_some()) {
            break;
        }

        let Some(m) = trigger.regex.find(line_remainder) else {
            continue;
        };

        let start = if trigger.continuation {
            m.start()
                + trigger
                    .regex
                    .captures(line_remainder)
                    .and_then(|caps| caps.get(1))
                    .map(|group| group.start() - m.start())
                    .unwrap_or(0)
        } else {
            m.start()
        };
        let new_pos = offset_chars + line_remainder[..start].chars().count();

        if link_start.is_none_or(|old| new_pos < old) {
            link_start = Some(new_pos);
            match_continues_link = trigger.continuation;
            if trigger.mail {
                match_link_len = m.as_str().chars().count();
                match_is_mail_link = true;
            }
        }
    }

    let link_start = link_start?;
    let mut work_url = String::new();

    if match_link_len > 0 {
        work_url = line.chars().skip(link_start).take(match_link_len).collect();
    } else {
        if !prev_line_link.is_empty()
            && match_continues_link
            && prev_line_link.contains("imdb.")
            && imdb_finished_re().is_match(prev_line_link)
        {
            return None;
        }

        let mut line_remainder: String = line.chars().skip(link_start).collect();
        if line_remainder.starts_with("hxxp://") || line_remainder.starts_with("h**p://") {
            line_remainder.replace_range(0..7, "http://");
        } else if line_remainder.starts_with("hxxps://") || line_remainder.starts_with("h**ps://") {
            line_remainder.replace_range(0..8, "https://");
        }

        let min_len = if match_continues_link { 4 } else { 9 };
        let candidate: String = line_remainder
            .chars()
            .take_while(|&ch| {
                ch.is_ascii_alphanumeric()
                    || matches!(
                        ch,
                        ',' | '/'
                            | '.'
                            | '!'
                            | '#'
                            | ':'
                            | '%'
                            | ';'
                            | '?'
                            | '&'
                            | '='
                            | '~'
                            | '+'
                            | '-'
                    )
            })
            .collect();

        if candidate.chars().count() >= min_len {
            work_url = candidate;
            while work_url.ends_with('.') || work_url.ends_with(':') {
                work_url.pop();
            }
        }
    }

    if work_url.is_empty() {
        return None;
    }

    let link_len = work_url.chars().count();
    let next_offset = link_start + link_len;

    if match_is_mail_link {
        return Some(FoundLink {
            pos: link_start,
            len: link_len,
            next_offset,
            url: format!("mailto:{work_url}"),
            continued: false,
        });
    }

    if work_url.starts_with("http://http://") {
        work_url.replace_range(0..7, "");
    }

    let continued = !prev_line_link.is_empty() && match_continues_link;
    if continued {
        let mut previous = prev_line_link.to_string();
        if !previous.ends_with('.') && continuation_extension_re().is_match(&work_url) {
            previous.push('.');
        }
        work_url.insert_str(0, &previous);
    }

    if !scheme_re().is_match(&work_url) {
        work_url.insert_str(0, "http://");
    }

    if domain_re().is_match(&work_url) && !work_url.contains("...") {
        Some(FoundLink {
            pos: link_start,
            len: link_len,
            next_offset,
            url: work_url,
            continued,
        })
    } else {
        None
    }
}

struct LinkTrigger {
    regex: Regex,
    continuation: bool,
    mail: bool,
}

fn link_triggers() -> &'static [LinkTrigger] {
    static TRIGGERS: OnceLock<Vec<LinkTrigger>> = OnceLock::new();
    TRIGGERS
        .get_or_init(|| {
            [
                ("h(?:tt|xx|\\*\\*)p://", false, false),
                ("h(?:tt|xx|\\*\\*)ps://", false, false),
                ("www\\.", false, false),
                ("\\w+\\.imdb\\.com", false, false),
                ("imdb\\.com", false, false),
                ("(imdb|ofdb|cinefacts|zelluloid|kino)\\.de", false, false),
                (
                    "(tinyurl|twitter|facebook|imgur|youtube)\\.com",
                    false,
                    false,
                ),
                ("\\b(bit\\.ly|goo\\.gl|t\\.co|youtu\\.be)/", false, false),
                (
                    "[a-zA-Z0-9]+(?:[a-zA-Z0-9._=-]*)@[a-zA-Z](?:[a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,}",
                    false,
                    true,
                ),
                ("^\\s*(/)", true, false),
                (
                    "(\\S+\\.(?:html?|php|aspx?|jpe?g|png|gif)\\S*)",
                    true,
                    false,
                ),
                ("(\\S+/dp/\\S*)", true, false),
                ("(\\S*dp/[A-Z]\\S+)", true, false),
                ("(\\S*/\\w+=\\S+)", true, false),
                ("(\\S+[&?]\\w+=\\S*)", true, false),
                ("(\\S{4,}/\\S*)", true, false),
                ("(\\S+%28\\S+%29\\S*)", true, false),
            ]
            .into_iter()
            .map(|(pattern, continuation, mail)| LinkTrigger {
                regex: Regex::new(pattern).unwrap(),
                continuation,
                mail,
            })
            .collect()
        })
        .as_slice()
}

fn make_stripped_text(text_content: &str) -> String {
    let mut text = String::with_capacity(text_content.len() / 2);
    let mut line = String::with_capacity(200);

    for ch in text_content.chars() {
        if (ch < '\u{80}') || ch.is_alphanumeric() || ch.is_whitespace() {
            if ch == '\n' {
                trim_right_chars(&mut line, "\t\r\n ");
                text.push_str(&strip_single_line(&line));
                text.push('\n');
                line.clear();
            } else {
                line.push(ch);
            }
        } else {
            line.push(' ');
        }
    }

    trim_left_chars(&mut text, "\n");
    text = collapse_blank_lines_re()
        .replace_all(&text, "\n\n")
        .to_string();

    let mut new_text = String::new();
    let mut prev = 0usize;
    while let Some(pos) = text[prev..].find("\n\n") {
        let abs = prev + pos;
        new_text.push_str(&trim_paragraph(&text[prev..abs]));
        new_text.push_str("\n\n");
        prev = abs + 2;
    }

    if prev < text.len() {
        new_text.push_str(&trim_paragraph(&text[prev..]));
    }

    new_text
}

fn strip_single_line(line: &str) -> String {
    if !line.is_empty() && line.chars().all(|c| !c.is_ascii_alphanumeric()) {
        return String::new();
    }

    let mut chars = line.chars();
    if let Some(first) = chars.next()
        && chars.all(|c| c == first)
    {
        return String::new();
    }

    let mut work = strip_repeated_prefix(line).unwrap_or_else(|| line.to_string());
    work = strip_repeated_suffix(&work).unwrap_or(work);
    work = trailing_junk_re().replace(&work, "").to_string();

    if work.is_empty() || work.trim().chars().count() <= 3 {
        String::new()
    } else {
        work
    }
}

fn strip_repeated_prefix(line: &str) -> Option<String> {
    let chars: Vec<char> = line.chars().collect();
    let first = *chars.first()?;
    if first.is_whitespace() {
        return None;
    }

    let mut idx = 0usize;
    while idx < chars.len() && chars[idx] == first {
        idx += 1;
    }

    let spaces = chars[idx..]
        .iter()
        .take_while(|&&c| c.is_whitespace())
        .count();
    if idx > 1 && spaces >= 3 && idx + spaces < chars.len() {
        Some(chars[idx + spaces..].iter().collect())
    } else {
        None
    }
}

fn strip_repeated_suffix(line: &str) -> Option<String> {
    let chars: Vec<char> = line.chars().collect();
    let last = *chars.last()?;
    if last.is_whitespace() {
        return None;
    }

    let mut idx = chars.len();
    while idx > 0 && chars[idx - 1] == last {
        idx -= 1;
    }

    let mut space_start = idx;
    while space_start > 0 && chars[space_start - 1].is_whitespace() {
        space_start -= 1;
    }

    if idx < chars.len() - 1 && idx - space_start >= 3 && space_start > 0 {
        Some(chars[..space_start].iter().collect())
    } else {
        None
    }
}

fn trim_paragraph(text: &str) -> String {
    let lines: Vec<&str> = text.split('\n').collect();
    let min_white = lines
        .iter()
        .map(|line| line.chars().take_while(|&c| c == ' ').count())
        .min()
        .unwrap_or(0);

    let mut result = String::new();
    for line in lines {
        result.push_str(&line.chars().skip(min_white).collect::<String>());
        result.push('\n');
    }

    trim_right_chars(&mut result, "\n");
    result
}

fn trim_right_chars(value: &mut String, chars: &str) {
    while value.ends_with(|c| chars.contains(c)) {
        value.pop();
    }
}

fn trim_left_chars(value: &mut String, chars: &str) {
    let trimmed = value.trim_start_matches(|c| chars.contains(c)).to_string();
    *value = trimmed;
}

fn utf8_to_latin9(data: &[u8]) -> Vec<u8> {
    std::str::from_utf8(data)
        .unwrap_or_default()
        .chars()
        .filter_map(|c| match c as u32 {
            0x0000..=0x00FF => Some(c as u8),
            0x0152 => Some(188),
            0x0153 => Some(189),
            0x0160 => Some(166),
            0x0161 => Some(168),
            0x0178 => Some(190),
            0x017D => Some(180),
            0x017E => Some(184),
            0x20AC => Some(164),
            _ => None,
        })
        .collect()
}

fn cp437_high(byte: u8) -> char {
    codepage_437::decode_high(byte)
}

fn cp437_control(byte: u8) -> char {
    codepage_437::decode_control(byte)
}

fn cp437_strict_high(byte: u8) -> char {
    codepage_437::decode_strict_high(byte)
}

fn cp437_strict_control(byte: u8) -> char {
    codepage_437::decode_strict_control(byte)
}

fn encode_cp437_text(text: &str) -> (Vec<u8>, usize) {
    let mut bytes = Vec::with_capacity(text.len());
    let mut chars_not_converted = 0usize;

    for ch in text.chars() {
        let code = ch as u32;
        if (code > 0x1F && code < 0x7F) || ch == '\n' || ch == '\r' {
            bytes.push(ch as u8);
        } else if let Some(byte) = codepage_437::encode(ch) {
            bytes.push(byte);
        } else {
            bytes.push(b' ');
            chars_not_converted += 1;
        }
    }

    (bytes, chars_not_converted)
}

fn char_to_byte_idx(s: &str, char_idx: usize) -> usize {
    s.char_indices()
        .nth(char_idx)
        .map(|(idx, _)| idx)
        .unwrap_or(s.len())
}

fn binary_text_re() -> &'static Regex {
    static RE: OnceLock<Regex> = OnceLock::new();
    RE.get_or_init(|| Regex::new(r"^\s+[A-Z][a-z]+\s+$").unwrap())
}

fn imdb_finished_re() -> &'static Regex {
    static RE: OnceLock<Regex> = OnceLock::new();
    RE.get_or_init(|| Regex::new(r"/[a-z]{2}\d{6,}/?$").unwrap())
}

fn continuation_extension_re() -> &'static Regex {
    static RE: OnceLock<Regex> = OnceLock::new();
    RE.get_or_init(|| Regex::new(r"^(html?|php|aspx?|jpe?g|png|gif)").unwrap())
}

fn scheme_re() -> &'static Regex {
    static RE: OnceLock<Regex> = OnceLock::new();
    RE.get_or_init(|| Regex::new(r"(?i)^(http|ftp)s?://").unwrap())
}

fn domain_re() -> &'static Regex {
    static RE: OnceLock<Regex> = OnceLock::new();
    RE.get_or_init(|| Regex::new(r"(?i)^(http|ftp)s?://([\w-]+)\.([\w.-]+)/?").unwrap())
}

fn collapse_blank_lines_re() -> &'static Regex {
    static RE: OnceLock<Regex> = OnceLock::new();
    RE.get_or_init(|| Regex::new(r"\n{2,}").unwrap())
}

fn trailing_junk_re() -> &'static Regex {
    static RE: OnceLock<Regex> = OnceLock::new();
    RE.get_or_init(|| Regex::new(r"\s+[\\/:.#_|()\[\]*@=+ \t-]{3,}$").unwrap())
}

#[cfg(test)]
mod tests {
    use super::*;

    fn load_bytes(name: &str, bytes: &[u8]) -> Result<NfoData, String> {
        let mut data = NfoData::new();
        data.file_path = Some(PathBuf::from(name));
        data.load_from_memory_internal(bytes)?;
        data.loaded = true;
        Ok(data)
    }

    #[test]
    fn decodes_utf8_with_signature() {
        let data = load_bytes("sample.nfo", b"\xEF\xBB\xBFhello\r\n").unwrap();
        assert_eq!(data.get_charset_name(), "UTF-8 (Signature)");
        assert_eq!(data.text_content, "hello\n");
    }

    #[test]
    fn decodes_plain_utf8_for_nfo() {
        let data = load_bytes("sample.nfo", "Grüße\n".as_bytes()).unwrap();
        assert_eq!(data.get_charset_name(), "UTF-8");
        assert_eq!(data.text_content, "Grüße\n");
    }

    #[test]
    fn falls_back_to_cp437_for_utf16le_like_reference_build() {
        let data = load_bytes("sample.nfo", b"\xFF\xFEG\0o\0\n\0").unwrap();
        assert_eq!(data.get_charset_name(), "CP 437");
        assert!(data.text_content.starts_with(" \u{25A0}G o"));
    }

    #[test]
    fn decodes_cp437_blocks() {
        let data = load_bytes("sample.nfo", &[0xDB, b'\n']).unwrap();
        assert_eq!(data.text_content, "\u{2588}\n");
    }

    #[test]
    fn normalizes_tabs_nbsp_and_crlf() {
        let data = load_bytes("sample.nfo", b"a\tb\r\nc\xC2\xA0\n").unwrap();
        assert_eq!(data.text_content, "a        b\nc \n");
    }

    #[test]
    fn detects_links() {
        let data = load_bytes("sample.nfo", b"visit hxxp://example.com/test\n").unwrap();
        assert_eq!(data.link_url(0, 8), Some("http://example.com/test"));
    }

    #[test]
    fn encodes_cp437_ascii_newlines_and_table_chars() {
        let (bytes, not_converted) = encode_cp437_text("A\r\n\u{2588}\u{263A}\u{E9}");

        assert_eq!(bytes, vec![0x41, 0x0D, 0x0A, 0xDB, 0x01, 0x82]);
        assert_eq!(not_converted, 0);
    }

    #[test]
    fn encodes_cp437_unsupported_chars_as_spaces() {
        let (bytes, not_converted) = encode_cp437_text("A\u{1F600}B");

        assert_eq!(bytes, b"A B");
        assert_eq!(not_converted, 1);
    }

    #[test]
    fn encodes_cp437_apostrophe_as_ascii() {
        let (bytes, not_converted) = encode_cp437_text("'");

        assert_eq!(bytes, vec![0x27]);
        assert_eq!(not_converted, 0);
    }

    #[test]
    fn exposes_cp437_export_bytes() {
        let data = load_bytes("sample.nfo", b"A\xDB\n").unwrap();

        assert_eq!(data.get_cp437_bytes(), (vec![0x41, 0xDB, 0x0A], 0));
    }
}
