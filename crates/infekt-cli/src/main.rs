use clap::{ArgAction, Parser};
use infekt_core as core;
use infekt_core::nfo_to_html_canvas::{NfoHtmlCanvasSettings, NfoHtmlColor};
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
    #[arg(short = 'm', long = "html", action = ArgAction::SetTrue, hide = true)]
    html: bool,
    #[arg(short = 'M', long = "html-canvas", action = ArgAction::SetTrue)]
    html_canvas: bool,
    #[arg(short = 'J', long = "json", action = ArgAction::SetTrue)]
    json: bool,
    #[arg(short = 'd', long = "pdf", action = ArgAction::SetTrue, hide = true)]
    pdf: bool,
    #[arg(short = 'D', long = "pdf-din", action = ArgAction::SetTrue, hide = true)]
    pdf_din: bool,
    #[arg(short = 't', long = "utf-16", action = ArgAction::SetTrue, hide = true)]
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

fn main() {
    let args = CliArgs::parse();

    if let Some(option_name) = first_not_implemented_option(&args) {
        eprintln!("ERROR: Option {option_name} is not implemented yet.");
        process::exit(1);
    }

    if !args.utf8 && !args.html_canvas && !args.json {
        eprintln!("ERROR: Only --utf-8 output mode is implemented right now.");
        process::exit(1);
    }

    let canvas_settings = make_canvas_settings(&args).unwrap_or_else(|err| {
        eprintln!("ERROR: {err}");
        process::exit(1);
    });

    let mut nfo_data = core::nfo_data::NfoData::new();
    if let Err(err) = nfo_data.load_from_file(&args.input_file) {
        eprintln!("ERROR: Unable to load NFO file: {err}");
        process::exit(1);
    }

    let (text, default_extension, write_bom) = if args.html_canvas {
        (nfo_data.get_canvas_html(&canvas_settings), "html", false)
    } else if args.json {
        (nfo_data.get_canvas_render_json(), "json", false)
    } else if args.text_only {
        (nfo_data.get_stripped_text(), "nfo", true)
    } else {
        (nfo_data.get_classic_text(), "nfo", true)
    };

    let out_file = args
        .out_file
        .clone()
        .unwrap_or_else(|| make_default_out_file(&args.input_file, default_extension));

    let write_result = File::create(&out_file).and_then(|mut f| {
        if write_bom {
            const UTF8_BOM: &[u8] = b"\xEF\xBB\xBF";
            f.write_all(UTF8_BOM)?;
        }

        f.write_all(text.as_bytes())
    });

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

fn make_canvas_settings(args: &CliArgs) -> Result<NfoHtmlCanvasSettings, String> {
    let mut settings = NfoHtmlCanvasSettings::default();

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

fn first_not_implemented_option(args: &CliArgs) -> Option<&'static str> {
    let canvas_export = args.html_canvas || args.json;

    if args.png {
        return Some("--png/-P");
    }
    if args.png_classic {
        return Some("--png-classic/-p");
    }
    if args.cp_437 {
        return Some("--cp-437/-e");
    }
    if args.html {
        return Some("--html/-m");
    }
    if args.pdf {
        return Some("--pdf/-d");
    }
    if args.pdf_din {
        return Some("--pdf-din/-D");
    }
    if args.utf16 {
        return Some("--utf-16/-t");
    }
    if !canvas_export && args.text_color.is_some() {
        return Some("--text-color/-T");
    }
    if !canvas_export && args.back_color.is_some() {
        return Some("--back-color/-B");
    }
    if !canvas_export && args.block_color.is_some() {
        return Some("--block-color/-A");
    }
    if !canvas_export && args.no_glow {
        return Some("--no-glow/-g");
    }
    if !canvas_export && args.glow_color.is_some() {
        return Some("--glow-color/-G");
    }
    if !canvas_export && args.hilight_links {
        return Some("--hilight-links/-L");
    }
    if !canvas_export && args.link_color.is_some() {
        return Some("--link-color/-U");
    }
    if !canvas_export && args.no_link_underl {
        return Some("--no-link-underl/-u");
    }
    if !canvas_export && args.block_width.is_some() {
        return Some("--block-width/-W");
    }
    if !canvas_export && args.block_height.is_some() {
        return Some("--block-height/-H");
    }
    if !canvas_export && args.glow_radius.is_some() {
        return Some("--glow-radius/-R");
    }
    if args.compound_whitespace {
        return Some("--compound-whitespace/-c");
    }
    if args.wrap {
        return Some("--wrap/-w");
    }

    None
}

fn make_default_out_file(input_file: &Path, extension: &str) -> PathBuf {
    let mut base = input_file.to_string_lossy().to_string();

    if base.ends_with(".nfo") {
        base.truncate(base.len() - 4);
    }

    if extension == "nfo" {
        base.push_str("-utf8.nfo");
    } else {
        base.push('.');
        base.push_str(extension);
    }

    PathBuf::from(base)
}
