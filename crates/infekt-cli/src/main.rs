use clap::{ArgAction, Parser};
use infekt_core as core;
use infekt_core::nfo_data::{NFO_SIZE_LIMIT_BYTES, UTF8_SIGNATURE, UTF16_LE_BOM};
use infekt_core::nfo_html_exporter::{NfoHtmlColor, NfoHtmlExporter, NfoHtmlSettings};
use std::collections::HashSet;
use std::fs::File;
use std::io::{Read, Write};
use std::path::{Path, PathBuf};
use std::process;

#[cfg(test)]
mod regression_samples;

#[derive(Debug, Parser)]
#[command(name = "infekt-cli")]
#[command(about = "Rust CLI for iNFekt (first-iteration compatibility)")]
struct CliArgs {
    #[arg(
        short = 'f',
        long = "utf-8",
        value_name = "PATH",
        num_args = 0..=1,
        require_equals = true
    )]
    utf8: Option<Option<PathBuf>>,

    #[arg(short = 'S', long = "text-only", action = ArgAction::SetTrue)]
    text_only: bool,

    #[arg(short = 'O', long = "out-file", value_name = "PATH")]
    out_file: Option<PathBuf>,

    #[arg(long = "compression", action = ArgAction::SetTrue)]
    compression: bool,

    #[arg(short = 'P', long = "png", action = ArgAction::SetTrue, hide = true)]
    png: bool,
    #[arg(short = 'p', long = "png-classic", action = ArgAction::SetTrue, hide = true)]
    png_classic: bool,
    #[arg(
        short = 'e',
        long = "cp-437",
        value_name = "PATH",
        num_args = 0..=1,
        require_equals = true,
        hide = true
    )]
    cp_437: Option<Option<PathBuf>>,
    #[arg(
        short = 'm',
        long = "html",
        value_name = "PATH",
        num_args = 0..=1,
        require_equals = true
    )]
    html: Option<Option<PathBuf>>,
    #[arg(
        short = 'M',
        long = "html-canvas",
        value_name = "PATH",
        num_args = 0..=1,
        require_equals = true
    )]
    html_canvas: Option<Option<PathBuf>>,
    #[arg(
        short = 'J',
        long = "json",
        value_name = "PATH",
        num_args = 0..=1,
        require_equals = true
    )]
    json: Option<Option<PathBuf>>,
    #[arg(short = 'd', long = "pdf", action = ArgAction::SetTrue, hide = true)]
    pdf: bool,
    #[arg(short = 'D', long = "pdf-din", action = ArgAction::SetTrue, hide = true)]
    pdf_din: bool,
    #[arg(
        short = 't',
        long = "utf-16",
        value_name = "PATH",
        num_args = 0..=1,
        require_equals = true
    )]
    utf16: Option<Option<PathBuf>>,

    #[arg(short = 'T', long = "text-color", value_name = "RRGGBB")]
    text_color: Option<String>,
    #[arg(short = 'B', long = "back-color", value_name = "RRGGBB")]
    back_color: Option<String>,
    #[arg(short = 'A', long = "block-color", value_name = "RRGGBB")]
    block_color: Option<String>,
    #[arg(short = 'g', long = "no-glow", action = ArgAction::SetTrue)]
    no_glow: bool,
    #[arg(short = 'G', long = "glow-color", value_name = "RRGGBB")]
    glow_color: Option<String>,
    #[arg(short = 'L', long = "hilight-links", action = ArgAction::SetTrue)]
    hilight_links: bool,
    #[arg(short = 'U', long = "link-color", value_name = "RRGGBB")]
    link_color: Option<String>,
    #[arg(short = 'u', long = "no-link-underl", action = ArgAction::SetTrue)]
    no_link_underl: bool,
    #[arg(short = 'W', long = "block-width", value_name = "PIXELS")]
    block_width: Option<String>,
    #[arg(short = 'H', long = "block-height", value_name = "PIXELS")]
    block_height: Option<String>,
    #[arg(short = 'R', long = "glow-radius", value_name = "PIXELS")]
    glow_radius: Option<String>,
    #[arg(
        short = 'c',
        long = "compound-whitespace",
        action = ArgAction::SetTrue,
        hide = true
    )]
    compound_whitespace: bool,
    #[arg(short = 'w', long = "wrap", action = ArgAction::SetTrue, hide = true)]
    wrap: bool,

    input_file: PathBuf,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum OutputMode {
    ClassicHtml,
    CanvasHtml,
    CanvasJson,
    Cp437Text,
    Utf8StrippedText,
    Utf8ClassicText,
    Utf16StrippedText,
    Utf16ClassicText,
}

#[derive(Clone, Debug, PartialEq, Eq)]
struct OutputSelection {
    mode: OutputMode,
    option_name: &'static str,
    explicit_out_file: Option<PathBuf>,
}

#[derive(Clone, Debug, PartialEq, Eq)]
struct OutputTarget {
    mode: OutputMode,
    out_file: PathBuf,
}

impl OutputMode {
    fn uses_html_settings(self) -> bool {
        matches!(
            self,
            Self::ClassicHtml | Self::CanvasHtml | Self::CanvasJson
        )
    }

    fn default_extension(self) -> Option<&'static str> {
        match self {
            Self::ClassicHtml | Self::CanvasHtml => Some("html"),
            Self::CanvasJson => Some("json"),
            Self::Utf8StrippedText
            | Self::Utf8ClassicText
            | Self::Cp437Text
            | Self::Utf16StrippedText
            | Self::Utf16ClassicText => None,
        }
    }

    fn default_nfo_suffix(self) -> &'static str {
        match self {
            Self::Utf8StrippedText | Self::Utf8ClassicText => "-utf8.nfo",
            Self::Utf16StrippedText | Self::Utf16ClassicText => "-utf16.nfo",
            Self::Cp437Text => "-dos.nfo",
            Self::ClassicHtml | Self::CanvasHtml | Self::CanvasJson => unreachable!(),
        }
    }
}

fn main() {
    let args = CliArgs::parse();

    if let Some(option_name) = first_unimplemented_output_option(&args) {
        eprintln!("ERROR: Option {option_name} is not implemented yet.");
        process::exit(1);
    }

    let output_selections = output_selections_from_args(&args).unwrap_or_else(|err| {
        eprintln!("ERROR: {err}");
        process::exit(1);
    });

    if let Err(err) = validate_args(&args, &output_selections) {
        eprintln!("ERROR: {err}");
        process::exit(1);
    }

    let output_targets = resolve_output_targets(&args, &output_selections).unwrap_or_else(|err| {
        eprintln!("ERROR: {err}");
        process::exit(1);
    });

    let html_settings = if output_selections
        .iter()
        .any(|selection| selection.mode.uses_html_settings())
    {
        Some(make_html_settings(&args).unwrap_or_else(|err| {
            eprintln!("ERROR: {err}");
            process::exit(1);
        }))
    } else {
        None
    };

    let nfo_data = load_nfo_data(&args.input_file, args.compression).unwrap_or_else(|err| {
        eprintln!("ERROR: Unable to load NFO file: {err}");
        process::exit(1);
    });

    for target in output_targets {
        let output =
            render_output(target.mode, &nfo_data, html_settings.clone()).unwrap_or_else(|err| {
                eprintln!("ERROR: {err}");
                process::exit(1);
            });

        let output_bytes =
            maybe_compress_output(&output.bytes, args.compression).unwrap_or_else(|err| {
                eprintln!(
                    "ERROR: Unable to compress output for `{}`: {err}",
                    target.out_file.display()
                );
                process::exit(1);
            });

        let write_result =
            File::create(&target.out_file).and_then(|mut f| f.write_all(&output_bytes));

        if let Err(err) = write_result {
            eprintln!(
                "ERROR: Unable to write to `{}`: {err}",
                target.out_file.display()
            );
            process::exit(1);
        }

        if output.cp437_chars_not_converted > 0 {
            eprintln!(
                "WARNING: {} characters in NFO do not have a CP 437 equivalent and were dropped.",
                output.cp437_chars_not_converted
            );
        }

        println!(
            "Saved `{}` to `{}`!",
            args.input_file.display(),
            target.out_file.display()
        );
    }
}

fn load_nfo_data(input_file: &Path, compression: bool) -> Result<core::nfo_data::NfoData, String> {
    let mut nfo_data = core::nfo_data::NfoData::new();

    if compression {
        let compressed = File::open(input_file).map_err(|err| {
            format!(
                "Unable to open compressed NFO file '{}' (error {err})",
                input_file.to_string_lossy()
            )
        })?;
        let decompressed = decompress_zstd_input(&compressed)?;
        nfo_data.load_from_bytes(&logical_input_file(input_file), &decompressed)?;
    } else {
        nfo_data.load_from_file(input_file)?;
    }

    Ok(nfo_data)
}

fn decompress_zstd_input<R: Read>(compressed: R) -> Result<Vec<u8>, String> {
    let mut decoder = zstd::stream::read::Decoder::new(compressed)
        .map_err(|err| format!("Unable to initialize zstd decompressor: {err}"))?;
    let mut decompressed = Vec::new();
    let mut buffer = [0u8; 8192];

    loop {
        let bytes_read = decoder
            .read(&mut buffer)
            .map_err(|err| format!("Unable to decompress zstd input: {err}"))?;

        if bytes_read == 0 {
            break;
        }

        if decompressed.len() + bytes_read > NFO_SIZE_LIMIT_BYTES {
            return Err("NFO file is too large (> 3 MB)".to_string());
        }

        decompressed.extend_from_slice(&buffer[..bytes_read]);
    }

    Ok(decompressed)
}

fn maybe_compress_output(bytes: &[u8], compression: bool) -> Result<Vec<u8>, String> {
    if compression {
        zstd::stream::encode_all(bytes, 0).map_err(|err| err.to_string())
    } else {
        Ok(bytes.to_vec())
    }
}

fn make_html_settings(args: &CliArgs) -> Result<NfoHtmlSettings, String> {
    let mut settings = NfoHtmlSettings::default();

    if let Some(color) = &args.text_color {
        settings.color_text = parse_color("--text-color/-T", color)?;
    }
    if let Some(color) = &args.back_color {
        settings.color_back = parse_color("--back-color/-B", color)?;
    }
    if let Some(color) = &args.block_color {
        settings.color_art = parse_color("--block-color/-A", color)?;
    }
    if args.no_glow {
        settings.shadow_enable = false;
    }
    if let Some(color) = &args.glow_color {
        settings.shadow_color = parse_color("--glow-color/-G", color)?;
    }
    if args.hilight_links {
        settings.hyperlinks_highlight = true;
    }
    if let Some(color) = &args.link_color {
        settings.hyperlinks_color = parse_color("--link-color/-U", color)?;
    }
    if args.no_link_underl {
        settings.hyperlinks_underline = false;
    }
    if let Some(width) = &args.block_width {
        settings.block_width = parse_limited_usize("--block-width/-W", width, 1, 199)?;
    }
    if let Some(height) = &args.block_height {
        settings.block_height = parse_limited_usize("--block-height/-H", height, 1, 199)?;
    }
    if let Some(radius) = &args.glow_radius {
        settings.shadow_radius = parse_limited_usize("--glow-radius/-R", radius, 0, 100)?;
    }

    Ok(settings)
}

fn render_output(
    mode: OutputMode,
    nfo_data: &core::nfo_data::NfoData,
    html_settings: Option<NfoHtmlSettings>,
) -> Result<RenderedOutput, String> {
    if mode == OutputMode::Cp437Text {
        let (bytes, cp437_chars_not_converted) = nfo_data.get_cp437_bytes();
        return Ok(RenderedOutput {
            bytes,
            cp437_chars_not_converted,
        });
    }

    if mode.uses_html_settings() {
        let settings = html_settings.ok_or_else(|| "Missing HTML export settings".to_string())?;
        let exporter = NfoHtmlExporter::new(nfo_data, settings);

        let text = match mode {
            OutputMode::ClassicHtml => exporter.export_html(),
            OutputMode::CanvasHtml => exporter.export_canvas_html(),
            OutputMode::CanvasJson => exporter.export_canvas_json(),
            OutputMode::Cp437Text
            | OutputMode::Utf8StrippedText
            | OutputMode::Utf8ClassicText
            | OutputMode::Utf16StrippedText
            | OutputMode::Utf16ClassicText => unreachable!(),
        };

        return Ok(RenderedOutput::from_text(mode, &text));
    }

    let text = match mode {
        OutputMode::Utf8StrippedText | OutputMode::Utf16StrippedText => {
            nfo_data.get_stripped_text()
        }
        OutputMode::Utf8ClassicText | OutputMode::Utf16ClassicText => nfo_data.get_classic_text(),
        OutputMode::Cp437Text
        | OutputMode::ClassicHtml
        | OutputMode::CanvasHtml
        | OutputMode::CanvasJson => unreachable!(),
    };

    Ok(RenderedOutput::from_text(mode, &text))
}

struct RenderedOutput {
    bytes: Vec<u8>,
    cp437_chars_not_converted: usize,
}

impl RenderedOutput {
    fn from_text(mode: OutputMode, text: &str) -> Self {
        Self {
            bytes: encode_output(mode, text),
            cp437_chars_not_converted: 0,
        }
    }
}

fn encode_output(mode: OutputMode, text: &str) -> Vec<u8> {
    match mode {
        OutputMode::Utf8StrippedText | OutputMode::Utf8ClassicText => {
            let mut bytes = Vec::with_capacity(UTF8_SIGNATURE.len() + text.len());
            bytes.extend_from_slice(&UTF8_SIGNATURE);
            bytes.extend_from_slice(text.as_bytes());
            bytes
        }
        OutputMode::Utf16StrippedText | OutputMode::Utf16ClassicText => {
            let mut bytes = Vec::with_capacity(UTF16_LE_BOM.len() + text.len() * 2);
            bytes.extend_from_slice(&UTF16_LE_BOM);
            bytes.extend(text.encode_utf16().flat_map(u16::to_le_bytes));
            bytes
        }
        OutputMode::ClassicHtml | OutputMode::CanvasHtml | OutputMode::CanvasJson => {
            text.as_bytes().to_vec()
        }
        OutputMode::Cp437Text => unreachable!(),
    }
}

fn parse_color(option_name: &str, value: &str) -> Result<NfoHtmlColor, String> {
    let trimmed = value.trim();
    let prefixed;
    let color = if trimmed.starts_with('#') {
        trimmed
    } else {
        prefixed = format!("#{trimmed}");
        &prefixed
    };

    NfoHtmlColor::parse_rgb(color)
        .map_err(|_| format!("{option_name} expects an RGB hex color such as ff00aa or #ff00aa"))
}

fn parse_limited_usize(
    option_name: &str,
    value: &str,
    min: usize,
    max: usize,
) -> Result<usize, String> {
    let parsed = value
        .parse::<usize>()
        .map_err(|_| format!("{option_name} expects an integer"))?;

    if parsed < min || parsed > max {
        return Err(format!("{option_name} must be between {min} and {max}"));
    }

    Ok(parsed)
}

fn validate_args(args: &CliArgs, output_selections: &[OutputSelection]) -> Result<(), String> {
    if let Some(option_name) = first_unimplemented_option(args) {
        return Err(format!("Option {option_name} is not implemented yet."));
    }

    if args.text_only
        && !output_selections.iter().any(|selection| {
            matches!(
                selection.mode,
                OutputMode::Utf8StrippedText | OutputMode::Utf16StrippedText
            )
        })
    {
        return Err("Option --text-only/-S requires --utf-8/-f or --utf-16/-t.".to_string());
    }

    if !output_selections.iter().any(|selection| {
        matches!(
            selection.mode,
            OutputMode::CanvasHtml | OutputMode::CanvasJson
        )
    }) && let Some(option_name) = first_canvas_only_option(args)
    {
        return Err(format!(
            "Option {option_name} is only supported for --html-canvas/-M and --json/-J."
        ));
    }

    if !output_selections
        .iter()
        .any(|selection| selection.mode.uses_html_settings())
        && let Some(option_name) = first_html_style_option(args)
    {
        return Err(format!(
            "Option {option_name} is only supported for HTML and JSON exports."
        ));
    }

    Ok(())
}

fn output_selections_from_args(args: &CliArgs) -> Result<Vec<OutputSelection>, String> {
    let modes = enabled_output_modes(args);

    if modes.is_empty() {
        return Err(
            "Choose an output mode: --utf-8/-f, --utf-16/-t, --cp-437/-e, --html/-m, --html-canvas/-M, or --json/-J."
                .to_string(),
        );
    }

    Ok(modes)
}

fn enabled_output_modes(args: &CliArgs) -> Vec<OutputSelection> {
    let mut modes = Vec::with_capacity(6);

    if let Some(out_file) = &args.utf8 {
        modes.push(OutputSelection {
            mode: if args.text_only {
                OutputMode::Utf8StrippedText
            } else {
                OutputMode::Utf8ClassicText
            },
            option_name: "--utf-8/-f",
            explicit_out_file: out_file.clone(),
        });
    }
    if let Some(out_file) = &args.utf16 {
        modes.push(OutputSelection {
            mode: if args.text_only {
                OutputMode::Utf16StrippedText
            } else {
                OutputMode::Utf16ClassicText
            },
            option_name: "--utf-16/-t",
            explicit_out_file: out_file.clone(),
        });
    }
    if let Some(out_file) = &args.html {
        modes.push(OutputSelection {
            mode: OutputMode::ClassicHtml,
            option_name: "--html/-m",
            explicit_out_file: out_file.clone(),
        });
    }
    if let Some(out_file) = &args.html_canvas {
        modes.push(OutputSelection {
            mode: OutputMode::CanvasHtml,
            option_name: "--html-canvas/-M",
            explicit_out_file: out_file.clone(),
        });
    }
    if let Some(out_file) = &args.json {
        modes.push(OutputSelection {
            mode: OutputMode::CanvasJson,
            option_name: "--json/-J",
            explicit_out_file: out_file.clone(),
        });
    }
    if let Some(out_file) = &args.cp_437 {
        modes.push(OutputSelection {
            mode: OutputMode::Cp437Text,
            option_name: "--cp-437/-e",
            explicit_out_file: out_file.clone(),
        });
    }

    modes
}

fn resolve_output_targets(
    args: &CliArgs,
    output_selections: &[OutputSelection],
) -> Result<Vec<OutputTarget>, String> {
    if args.out_file.is_some() && output_selections.len() > 1 {
        return Err(
            "Option --out-file/-O can only be used when exactly one output mode is selected."
                .to_string(),
        );
    }

    if let Some(selection) = output_selections
        .iter()
        .find(|selection| selection.explicit_out_file.is_some())
        && args.out_file.is_some()
    {
        return Err(format!(
            "Option --out-file/-O cannot be combined with an output filename on {}.",
            selection.option_name
        ));
    }

    let mut targets = Vec::with_capacity(output_selections.len());
    let mut seen_out_files = HashSet::with_capacity(output_selections.len());
    let default_input_file = if args.compression {
        logical_input_file(&args.input_file)
    } else {
        args.input_file.clone()
    };
    let use_distinct_canvas_html_default = output_selections.iter().any(|selection| {
        selection.mode == OutputMode::ClassicHtml && selection.explicit_out_file.is_none()
    }) && output_selections.iter().any(|selection| {
        selection.mode == OutputMode::CanvasHtml && selection.explicit_out_file.is_none()
    });

    for selection in output_selections {
        let out_file = selection
            .explicit_out_file
            .clone()
            .or_else(|| args.out_file.clone())
            .unwrap_or_else(|| {
                let out_file = if selection.mode == OutputMode::CanvasHtml
                    && use_distinct_canvas_html_default
                {
                    make_default_canvas_html_out_file(&default_input_file)
                } else {
                    make_default_out_file(&default_input_file, selection.mode)
                };

                if args.compression {
                    append_zstd_extension(out_file)
                } else {
                    out_file
                }
            });

        if !seen_out_files.insert(out_file.clone()) {
            return Err(format!(
                "Output path `{}` is used by more than one selected format.",
                out_file.display()
            ));
        }

        targets.push(OutputTarget {
            mode: selection.mode,
            out_file,
        });
    }

    Ok(targets)
}

fn first_unimplemented_output_option(args: &CliArgs) -> Option<&'static str> {
    first_enabled(&[
        (args.png, "--png/-P"),
        (args.png_classic, "--png-classic/-p"),
        (args.pdf, "--pdf/-d"),
        (args.pdf_din, "--pdf-din/-D"),
    ])
}

fn first_unimplemented_option(args: &CliArgs) -> Option<&'static str> {
    first_enabled(&[
        (args.compound_whitespace, "--compound-whitespace/-c"),
        (args.wrap, "--wrap/-w"),
    ])
}

fn first_html_style_option(args: &CliArgs) -> Option<&'static str> {
    first_enabled(&[
        (args.text_color.is_some(), "--text-color/-T"),
        (args.back_color.is_some(), "--back-color/-B"),
        (args.block_color.is_some(), "--block-color/-A"),
        (args.no_glow, "--no-glow/-g"),
        (args.glow_color.is_some(), "--glow-color/-G"),
        (args.hilight_links, "--hilight-links/-L"),
        (args.link_color.is_some(), "--link-color/-U"),
        (args.no_link_underl, "--no-link-underl/-u"),
        (args.glow_radius.is_some(), "--glow-radius/-R"),
    ])
}

fn first_canvas_only_option(args: &CliArgs) -> Option<&'static str> {
    first_enabled(&[
        (args.block_width.is_some(), "--block-width/-W"),
        (args.block_height.is_some(), "--block-height/-H"),
    ])
}

fn first_enabled(options: &[(bool, &'static str)]) -> Option<&'static str> {
    options
        .iter()
        .find_map(|(enabled, name)| enabled.then_some(*name))
}

fn make_default_out_file(input_file: &Path, output_mode: OutputMode) -> PathBuf {
    let mut base = make_default_out_file_base(input_file);

    if let Some(extension) = output_mode.default_extension() {
        base.push('.');
        base.push_str(extension);
    } else {
        base.push_str(output_mode.default_nfo_suffix());
    }

    PathBuf::from(base)
}

fn make_default_canvas_html_out_file(input_file: &Path) -> PathBuf {
    let mut base = make_default_out_file_base(input_file);
    base.push_str("-canvas.html");
    PathBuf::from(base)
}

fn make_default_out_file_base(input_file: &Path) -> String {
    let mut base = input_file.to_string_lossy().to_string();

    if base.ends_with(".nfo") {
        base.truncate(base.len() - 4);
    }

    base
}

fn logical_input_file(input_file: &Path) -> PathBuf {
    if !has_zstd_extension(input_file) {
        return input_file.to_path_buf();
    }

    let Some(file_stem) = input_file.file_stem() else {
        return input_file.to_path_buf();
    };

    let mut logical = input_file.to_path_buf();
    logical.set_file_name(file_stem);
    logical
}

fn has_zstd_extension(path: &Path) -> bool {
    path.extension()
        .and_then(|extension| extension.to_str())
        .is_some_and(|extension| extension.eq_ignore_ascii_case("zstd"))
}

fn append_zstd_extension(path: PathBuf) -> PathBuf {
    PathBuf::from(format!("{}.zstd", path.to_string_lossy()))
}

#[cfg(test)]
mod tests {
    use super::*;

    fn parse_args<const N: usize>(args: [&str; N]) -> CliArgs {
        CliArgs::try_parse_from(args).unwrap()
    }

    fn parse_and_select<const N: usize>(args: [&str; N]) -> (CliArgs, Vec<OutputSelection>) {
        let args = parse_args(args);
        let selections = output_selections_from_args(&args).unwrap();
        (args, selections)
    }

    fn temp_path(name: &str) -> PathBuf {
        let mut path = std::env::temp_dir();
        path.push(format!(
            "infekt-cli-test-{}-{}-{name}",
            std::process::id(),
            std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)
                .unwrap()
                .as_nanos()
        ));
        path
    }

    #[test]
    fn encodes_utf8_text_with_bom() {
        let mut expected = UTF8_SIGNATURE.to_vec();
        expected.extend_from_slice(&[0x41, 0xE2, 0x96, 0x88, 0x0A]);

        assert_eq!(
            encode_output(OutputMode::Utf8ClassicText, "A\u{2588}\n"),
            expected
        );
    }

    #[test]
    fn encodes_utf16_text_as_little_endian_with_bom() {
        let mut expected = UTF16_LE_BOM.to_vec();
        expected.extend_from_slice(&[0x41, 0x00, 0x88, 0x25, 0x0A, 0x00]);

        assert_eq!(
            encode_output(OutputMode::Utf16ClassicText, "A\u{2588}\n"),
            expected
        );
    }

    #[test]
    fn encodes_utf16_surrogate_pairs() {
        let mut expected = UTF16_LE_BOM.to_vec();
        expected.extend_from_slice(&[0x3D, 0xD8, 0x00, 0xDE]);

        assert_eq!(
            encode_output(OutputMode::Utf16ClassicText, "\u{1F600}"),
            expected
        );
    }

    #[test]
    fn uses_utf16_default_suffix_for_text_exports() {
        assert_eq!(
            make_default_out_file(Path::new("demo.nfo"), OutputMode::Utf16ClassicText),
            PathBuf::from("demo-utf16.nfo")
        );
    }

    #[test]
    fn parses_compression_flag() {
        let args = parse_args(["infekt-cli", "--compression", "--utf-8", "demo.nfo.zstd"]);

        assert!(args.compression);
    }

    #[test]
    fn strips_zstd_extension_for_logical_input_file() {
        assert_eq!(
            logical_input_file(Path::new("demo.nfo.zstd")),
            PathBuf::from("demo.nfo")
        );
        assert_eq!(
            logical_input_file(Path::new("demo.nfo.ZSTD")),
            PathBuf::from("demo.nfo")
        );
        assert_eq!(
            logical_input_file(Path::new("demo.nfo")),
            PathBuf::from("demo.nfo")
        );
    }

    #[test]
    fn selects_cp437_output_mode() {
        let (_, selections) = parse_and_select(["infekt-cli", "--cp-437", "demo.nfo"]);

        assert_eq!(
            selections,
            vec![OutputSelection {
                mode: OutputMode::Cp437Text,
                option_name: "--cp-437/-e",
                explicit_out_file: None,
            }]
        );
    }

    #[test]
    fn uses_cp437_default_suffix_for_text_exports() {
        assert_eq!(
            make_default_out_file(Path::new("demo.nfo"), OutputMode::Cp437Text),
            PathBuf::from("demo-dos.nfo")
        );
    }

    #[test]
    fn renders_cp437_without_unicode_signature() {
        let mut path = std::env::temp_dir();
        path.push(format!(
            "infekt-cli-cp437-test-{}-no-bom.nfo",
            std::process::id()
        ));
        std::fs::write(&path, b"A\xDB\n").unwrap();

        let mut data = core::nfo_data::NfoData::new();
        data.load_from_file(&path).unwrap();
        let _ = std::fs::remove_file(&path);

        let output = render_output(OutputMode::Cp437Text, &data, None).unwrap();

        assert_eq!(output.bytes, vec![0x41, 0xDB, 0x0A]);
        assert!(!output.bytes.starts_with(&UTF8_SIGNATURE));
        assert!(!output.bytes.starts_with(&UTF16_LE_BOM));
        assert_eq!(output.cp437_chars_not_converted, 0);
    }

    #[test]
    fn selects_multiple_output_modes_with_default_paths() {
        let (_, selections) =
            parse_and_select(["infekt-cli", "--utf-8", "--html", "--json", "demo.nfo"]);

        assert_eq!(
            selections,
            vec![
                OutputSelection {
                    mode: OutputMode::Utf8ClassicText,
                    option_name: "--utf-8/-f",
                    explicit_out_file: None,
                },
                OutputSelection {
                    mode: OutputMode::ClassicHtml,
                    option_name: "--html/-m",
                    explicit_out_file: None,
                },
                OutputSelection {
                    mode: OutputMode::CanvasJson,
                    option_name: "--json/-J",
                    explicit_out_file: None,
                },
            ]
        );
    }

    #[test]
    fn records_inline_output_paths() {
        let (_, selections) = parse_and_select([
            "infekt-cli",
            "--utf-8=out.nfo",
            "--html=out.html",
            "demo.nfo",
        ]);

        assert_eq!(
            selections,
            vec![
                OutputSelection {
                    mode: OutputMode::Utf8ClassicText,
                    option_name: "--utf-8/-f",
                    explicit_out_file: Some(PathBuf::from("out.nfo")),
                },
                OutputSelection {
                    mode: OutputMode::ClassicHtml,
                    option_name: "--html/-m",
                    explicit_out_file: Some(PathBuf::from("out.html")),
                },
            ]
        );
    }

    #[test]
    fn resolves_mixed_default_and_inline_paths() {
        let (args, selections) =
            parse_and_select(["infekt-cli", "--utf-8", "--html=out.html", "demo.nfo"]);

        assert_eq!(
            resolve_output_targets(&args, &selections).unwrap(),
            vec![
                OutputTarget {
                    mode: OutputMode::Utf8ClassicText,
                    out_file: PathBuf::from("demo-utf8.nfo"),
                },
                OutputTarget {
                    mode: OutputMode::ClassicHtml,
                    out_file: PathBuf::from("out.html"),
                },
            ]
        );
    }

    #[test]
    fn appends_zstd_to_default_output_paths_when_compression_is_enabled() {
        let (args, selections) =
            parse_and_select(["infekt-cli", "--compression", "--utf-8", "demo.nfo"]);

        assert_eq!(
            resolve_output_targets(&args, &selections).unwrap(),
            vec![OutputTarget {
                mode: OutputMode::Utf8ClassicText,
                out_file: PathBuf::from("demo-utf8.nfo.zstd"),
            }]
        );

        let (args, selections) =
            parse_and_select(["infekt-cli", "--compression", "--html", "demo.nfo.zstd"]);

        assert_eq!(
            resolve_output_targets(&args, &selections).unwrap(),
            vec![OutputTarget {
                mode: OutputMode::ClassicHtml,
                out_file: PathBuf::from("demo.html.zstd"),
            }]
        );
    }

    #[test]
    fn appends_zstd_to_distinct_html_defaults_when_compression_is_enabled() {
        let (args, selections) = parse_and_select([
            "infekt-cli",
            "--compression",
            "--html",
            "--html-canvas",
            "demo.nfo",
        ]);

        assert_eq!(
            resolve_output_targets(&args, &selections).unwrap(),
            vec![
                OutputTarget {
                    mode: OutputMode::ClassicHtml,
                    out_file: PathBuf::from("demo.html.zstd"),
                },
                OutputTarget {
                    mode: OutputMode::CanvasHtml,
                    out_file: PathBuf::from("demo-canvas.html.zstd"),
                },
            ]
        );
    }

    #[test]
    fn preserves_explicit_output_paths_when_compression_is_enabled() {
        let (args, selections) =
            parse_and_select(["infekt-cli", "--compression", "--utf-8=out.nfo", "demo.nfo"]);

        assert_eq!(
            resolve_output_targets(&args, &selections).unwrap(),
            vec![OutputTarget {
                mode: OutputMode::Utf8ClassicText,
                out_file: PathBuf::from("out.nfo"),
            }]
        );

        let (args, selections) = parse_and_select([
            "infekt-cli",
            "--compression",
            "--html",
            "--out-file",
            "legacy.html",
            "demo.nfo",
        ]);

        assert_eq!(
            resolve_output_targets(&args, &selections).unwrap(),
            vec![OutputTarget {
                mode: OutputMode::ClassicHtml,
                out_file: PathBuf::from("legacy.html"),
            }]
        );
    }

    #[test]
    fn preserves_legacy_out_file_for_single_output() {
        let (args, selections) = parse_and_select([
            "infekt-cli",
            "--html",
            "--out-file",
            "legacy.html",
            "demo.nfo",
        ]);

        assert_eq!(
            resolve_output_targets(&args, &selections).unwrap(),
            vec![OutputTarget {
                mode: OutputMode::ClassicHtml,
                out_file: PathBuf::from("legacy.html"),
            }]
        );
    }

    #[test]
    fn rejects_no_output_modes() {
        let args = parse_args(["infekt-cli", "demo.nfo"]);

        assert!(
            output_selections_from_args(&args)
                .unwrap_err()
                .contains("Choose an output mode")
        );
    }

    #[test]
    fn rejects_out_file_with_multiple_outputs() {
        let (args, selections) = parse_and_select([
            "infekt-cli",
            "--utf-8",
            "--html",
            "--out-file",
            "out.html",
            "demo.nfo",
        ]);

        assert!(
            resolve_output_targets(&args, &selections)
                .unwrap_err()
                .contains("exactly one output mode")
        );
    }

    #[test]
    fn rejects_out_file_with_inline_path() {
        let (args, selections) = parse_and_select([
            "infekt-cli",
            "--html=inline.html",
            "--out-file",
            "legacy.html",
            "demo.nfo",
        ]);

        assert!(
            resolve_output_targets(&args, &selections)
                .unwrap_err()
                .contains("cannot be combined")
        );
    }

    #[test]
    fn rejects_duplicate_resolved_output_paths() {
        let (args, selections) =
            parse_and_select(["infekt-cli", "--html", "--json=demo.html", "demo.nfo"]);

        assert!(
            resolve_output_targets(&args, &selections)
                .unwrap_err()
                .contains("used by more than one")
        );
    }

    #[test]
    fn rejects_duplicate_paths_after_compression_default_suffixes() {
        let (args, selections) = parse_and_select([
            "infekt-cli",
            "--compression",
            "--html",
            "--json=demo.html.zstd",
            "demo.nfo",
        ]);

        assert!(
            resolve_output_targets(&args, &selections)
                .unwrap_err()
                .contains("used by more than one")
        );
    }

    #[test]
    fn uses_distinct_defaults_for_classic_and_canvas_html() {
        let (args, selections) =
            parse_and_select(["infekt-cli", "--html", "--html-canvas", "demo.nfo"]);

        assert_eq!(
            resolve_output_targets(&args, &selections).unwrap(),
            vec![
                OutputTarget {
                    mode: OutputMode::ClassicHtml,
                    out_file: PathBuf::from("demo.html"),
                },
                OutputTarget {
                    mode: OutputMode::CanvasHtml,
                    out_file: PathBuf::from("demo-canvas.html"),
                },
            ]
        );
    }

    #[test]
    fn text_only_selects_stripped_utf_modes() {
        let (_, selections) = parse_and_select([
            "infekt-cli",
            "--text-only",
            "--utf-8",
            "--utf-16",
            "demo.nfo",
        ]);

        assert_eq!(
            selections
                .iter()
                .map(|selection| selection.mode)
                .collect::<Vec<_>>(),
            vec![OutputMode::Utf8StrippedText, OutputMode::Utf16StrippedText]
        );
    }

    #[test]
    fn text_only_requires_a_utf_output() {
        let (args, selections) =
            parse_and_select(["infekt-cli", "--text-only", "--html", "demo.nfo"]);

        assert!(
            validate_args(&args, &selections)
                .unwrap_err()
                .contains("requires --utf-8/-f or --utf-16/-t")
        );
    }

    #[test]
    fn allows_html_settings_when_any_html_output_is_selected() {
        let (args, selections) = parse_and_select([
            "infekt-cli",
            "--utf-8",
            "--html",
            "--text-color",
            "ff00aa",
            "demo.nfo",
        ]);

        validate_args(&args, &selections).unwrap();
    }

    #[test]
    fn rejects_html_settings_without_html_outputs() {
        let (args, selections) = parse_and_select([
            "infekt-cli",
            "--utf-8",
            "--text-color",
            "ff00aa",
            "demo.nfo",
        ]);

        assert!(
            validate_args(&args, &selections)
                .unwrap_err()
                .contains("only supported for HTML and JSON")
        );
    }

    #[test]
    fn allows_canvas_settings_when_any_canvas_output_is_selected() {
        let (args, selections) = parse_and_select([
            "infekt-cli",
            "--html",
            "--json",
            "--block-width",
            "8",
            "demo.nfo",
        ]);

        validate_args(&args, &selections).unwrap();
    }

    #[test]
    fn rejects_canvas_settings_without_canvas_outputs() {
        let (args, selections) =
            parse_and_select(["infekt-cli", "--html", "--block-width", "8", "demo.nfo"]);

        assert!(
            validate_args(&args, &selections)
                .unwrap_err()
                .contains("only supported for --html-canvas/-M and --json/-J")
        );
    }

    #[test]
    fn compresses_output_bytes_with_zstd() {
        let original = b"\xEF\xBB\xBFhello\n";

        let compressed = maybe_compress_output(original, true).unwrap();
        let decompressed = zstd::stream::decode_all(&compressed[..]).unwrap();

        assert_eq!(decompressed, original);
        assert_eq!(maybe_compress_output(original, false).unwrap(), original);
    }

    #[test]
    fn loads_compressed_input_and_compresses_rendered_output() {
        let path = temp_path("input.nfo.zstd");
        let compressed_input = zstd::stream::encode_all(&b"A\xDB\n"[..], 0).unwrap();
        std::fs::write(&path, compressed_input).unwrap();

        let data = load_nfo_data(&path, true).unwrap();
        let _ = std::fs::remove_file(&path);
        let output = render_output(OutputMode::Utf8ClassicText, &data, None).unwrap();
        let compressed_output = maybe_compress_output(&output.bytes, true).unwrap();
        let decompressed_output = zstd::stream::decode_all(&compressed_output[..]).unwrap();

        assert_eq!(
            decompressed_output,
            encode_output(OutputMode::Utf8ClassicText, "A\u{2588}\n")
        );
    }
}
