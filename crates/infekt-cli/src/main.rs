use clap::{ArgAction, Parser};
use infekt_core as core;
use infekt_core::nfo_data::{UTF8_SIGNATURE, UTF16_LE_BOM};
use infekt_core::nfo_html_exporter::{NfoHtmlColor, NfoHtmlExporter, NfoHtmlSettings};
use std::fs::File;
use std::io::Write;
use std::path::{Path, PathBuf};
use std::process;

#[derive(Debug, Parser)]
#[command(name = "infekt-cli")]
#[command(about = "Rust CLI for iNFekt (first-iteration compatibility)")]
struct CliArgs {
    #[arg(short = 'f', long = "utf-8", action = ArgAction::SetTrue)]
    utf8: bool,

    #[arg(short = 'S', long = "text-only", action = ArgAction::SetTrue)]
    text_only: bool,

    #[arg(short = 'O', long = "out-file", value_name = "PATH")]
    out_file: Option<PathBuf>,

    #[arg(short = 'P', long = "png", action = ArgAction::SetTrue, hide = true)]
    png: bool,
    #[arg(short = 'p', long = "png-classic", action = ArgAction::SetTrue, hide = true)]
    png_classic: bool,
    #[arg(short = 'e', long = "cp-437", action = ArgAction::SetTrue, hide = true)]
    cp_437: bool,
    #[arg(short = 'm', long = "html", action = ArgAction::SetTrue)]
    html: bool,
    #[arg(short = 'M', long = "html-canvas", action = ArgAction::SetTrue)]
    html_canvas: bool,
    #[arg(short = 'J', long = "json", action = ArgAction::SetTrue)]
    json: bool,
    #[arg(short = 'd', long = "pdf", action = ArgAction::SetTrue, hide = true)]
    pdf: bool,
    #[arg(short = 'D', long = "pdf-din", action = ArgAction::SetTrue, hide = true)]
    pdf_din: bool,
    #[arg(short = 't', long = "utf-16", action = ArgAction::SetTrue)]
    utf16: bool,

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
    Utf8StrippedText,
    Utf8ClassicText,
    Utf16StrippedText,
    Utf16ClassicText,
}

impl OutputMode {
    fn from_args(args: &CliArgs) -> Result<Self, String> {
        let modes = enabled_output_modes(args);

        match modes.as_slice() {
            [] if args.text_only => {
                Err("Option --text-only/-S requires --utf-8/-f or --utf-16/-t.".to_string())
            }
            [] => Err(
                "Choose an output mode: --utf-8/-f, --utf-16/-t, --html/-m, --html-canvas/-M, or --json/-J."
                    .to_string(),
            ),
            [(mode, _)] => Ok(*mode),
            _ => Err(format!(
                "Choose only one output mode; found {}.",
                modes
                    .iter()
                    .map(|(_, name)| *name)
                    .collect::<Vec<_>>()
                    .join(", ")
            )),
        }
    }

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
            | Self::Utf16StrippedText
            | Self::Utf16ClassicText => None,
        }
    }

    fn default_nfo_suffix(self) -> &'static str {
        match self {
            Self::Utf8StrippedText | Self::Utf8ClassicText => "-utf8.nfo",
            Self::Utf16StrippedText | Self::Utf16ClassicText => "-utf16.nfo",
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

    let output_mode = OutputMode::from_args(&args).unwrap_or_else(|err| {
        eprintln!("ERROR: {err}");
        process::exit(1);
    });

    if let Err(err) = validate_args(&args, output_mode) {
        eprintln!("ERROR: {err}");
        process::exit(1);
    }

    let html_settings = if output_mode.uses_html_settings() {
        Some(make_html_settings(&args).unwrap_or_else(|err| {
            eprintln!("ERROR: {err}");
            process::exit(1);
        }))
    } else {
        None
    };

    let mut nfo_data = core::nfo_data::NfoData::new();
    if let Err(err) = nfo_data.load_from_file(&args.input_file) {
        eprintln!("ERROR: Unable to load NFO file: {err}");
        process::exit(1);
    }

    let text = render_output(output_mode, &nfo_data, html_settings).unwrap_or_else(|err| {
        eprintln!("ERROR: {err}");
        process::exit(1);
    });

    let out_file = args
        .out_file
        .clone()
        .unwrap_or_else(|| make_default_out_file(&args.input_file, output_mode));

    let output_bytes = encode_output(output_mode, &text);
    let write_result = File::create(&out_file).and_then(|mut f| f.write_all(&output_bytes));

    if let Err(err) = write_result {
        eprintln!("ERROR: Unable to write to `{}`: {err}", out_file.display());
        process::exit(1);
    }

    println!(
        "Saved `{}` to `{}`!",
        args.input_file.display(),
        out_file.display()
    );
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
) -> Result<String, String> {
    if mode.uses_html_settings() {
        let settings = html_settings.ok_or_else(|| "Missing HTML export settings".to_string())?;
        let exporter = NfoHtmlExporter::new(nfo_data, settings);

        return Ok(match mode {
            OutputMode::ClassicHtml => exporter.export_html(),
            OutputMode::CanvasHtml => exporter.export_canvas_html(),
            OutputMode::CanvasJson => exporter.export_canvas_json(),
            OutputMode::Utf8StrippedText
            | OutputMode::Utf8ClassicText
            | OutputMode::Utf16StrippedText
            | OutputMode::Utf16ClassicText => unreachable!(),
        });
    }

    Ok(match mode {
        OutputMode::Utf8StrippedText | OutputMode::Utf16StrippedText => {
            nfo_data.get_stripped_text()
        }
        OutputMode::Utf8ClassicText | OutputMode::Utf16ClassicText => nfo_data.get_classic_text(),
        OutputMode::ClassicHtml | OutputMode::CanvasHtml | OutputMode::CanvasJson => unreachable!(),
    })
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

fn validate_args(args: &CliArgs, output_mode: OutputMode) -> Result<(), String> {
    if let Some(option_name) = first_unimplemented_option(args) {
        return Err(format!("Option {option_name} is not implemented yet."));
    }

    if !matches!(output_mode, OutputMode::CanvasHtml | OutputMode::CanvasJson)
        && let Some(option_name) = first_canvas_only_option(args)
    {
        return Err(format!(
            "Option {option_name} is only supported for --html-canvas/-M and --json/-J."
        ));
    }

    if !output_mode.uses_html_settings()
        && let Some(option_name) = first_html_style_option(args)
    {
        return Err(format!(
            "Option {option_name} is only supported for HTML and JSON exports."
        ));
    }

    Ok(())
}

fn enabled_output_modes(args: &CliArgs) -> Vec<(OutputMode, &'static str)> {
    let mut modes = Vec::with_capacity(5);

    if args.utf8 {
        modes.push((
            if args.text_only {
                OutputMode::Utf8StrippedText
            } else {
                OutputMode::Utf8ClassicText
            },
            "--utf-8/-f",
        ));
    }
    if args.utf16 {
        modes.push((
            if args.text_only {
                OutputMode::Utf16StrippedText
            } else {
                OutputMode::Utf16ClassicText
            },
            "--utf-16/-t",
        ));
    }
    if args.html {
        modes.push((OutputMode::ClassicHtml, "--html/-m"));
    }
    if args.html_canvas {
        modes.push((OutputMode::CanvasHtml, "--html-canvas/-M"));
    }
    if args.json {
        modes.push((OutputMode::CanvasJson, "--json/-J"));
    }

    modes
}

fn first_unimplemented_output_option(args: &CliArgs) -> Option<&'static str> {
    first_enabled(&[
        (args.png, "--png/-P"),
        (args.png_classic, "--png-classic/-p"),
        (args.cp_437, "--cp-437/-e"),
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
    let mut base = input_file.to_string_lossy().to_string();

    if base.ends_with(".nfo") {
        base.truncate(base.len() - 4);
    }

    if let Some(extension) = output_mode.default_extension() {
        base.push('.');
        base.push_str(extension);
    } else {
        base.push_str(output_mode.default_nfo_suffix());
    }

    PathBuf::from(base)
}

#[cfg(test)]
mod tests {
    use super::*;

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
}
