use clap::{ArgAction, Parser};
use infekt_nfo_viewer::core;
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
    #[arg(short = 'M', long = "html-canvas", action = ArgAction::SetTrue, hide = true)]
    html_canvas: bool,
    #[arg(short = 'J', long = "json", action = ArgAction::SetTrue, hide = true)]
    json: bool,
    #[arg(short = 'd', long = "pdf", action = ArgAction::SetTrue, hide = true)]
    pdf: bool,
    #[arg(short = 'D', long = "pdf-din", action = ArgAction::SetTrue, hide = true)]
    pdf_din: bool,
    #[arg(short = 't', long = "utf-16", action = ArgAction::SetTrue, hide = true)]
    utf16: bool,

    #[arg(short = 'T', long = "text-color", hide = true)]
    text_color: Option<String>,
    #[arg(short = 'B', long = "back-color", hide = true)]
    back_color: Option<String>,
    #[arg(short = 'A', long = "block-color", hide = true)]
    block_color: Option<String>,
    #[arg(short = 'g', long = "no-glow", action = ArgAction::SetTrue, hide = true)]
    no_glow: bool,
    #[arg(short = 'G', long = "glow-color", hide = true)]
    glow_color: Option<String>,
    #[arg(short = 'L', long = "hilight-links", action = ArgAction::SetTrue, hide = true)]
    hilight_links: bool,
    #[arg(short = 'U', long = "link-color", hide = true)]
    link_color: Option<String>,
    #[arg(short = 'u', long = "no-link-underl", action = ArgAction::SetTrue, hide = true)]
    no_link_underl: bool,
    #[arg(short = 'W', long = "block-width", hide = true)]
    block_width: Option<String>,
    #[arg(short = 'H', long = "block-height", hide = true)]
    block_height: Option<String>,
    #[arg(short = 'R', long = "glow-radius", hide = true)]
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

    if !args.utf8 {
        eprintln!("ERROR: Only --utf-8 output mode is implemented right now.");
        process::exit(1);
    }

    let mut nfo_data = core::nfo_data::NfoData::new();
    if let Err(err) = nfo_data.load_from_file(&args.input_file) {
        eprintln!("ERROR: Unable to load NFO file: {err}");
        process::exit(1);
    }

    let text = if args.text_only {
        nfo_data.get_stripped_text()
    } else {
        nfo_data.get_classic_text()
    };

    let out_file = args
        .out_file
        .clone()
        .unwrap_or_else(|| make_default_out_file(&args.input_file));

    const UTF8_BOM: &[u8] = b"\xEF\xBB\xBF";
    let write_result = File::create(&out_file).and_then(|mut f| {
        f.write_all(UTF8_BOM)
            .and_then(|_| f.write_all(text.as_bytes()))
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

fn first_not_implemented_option(args: &CliArgs) -> Option<&'static str> {
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
    if args.html_canvas {
        return Some("--html-canvas/-M");
    }
    if args.json {
        return Some("--json/-J");
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
    if args.text_color.is_some() {
        return Some("--text-color/-T");
    }
    if args.back_color.is_some() {
        return Some("--back-color/-B");
    }
    if args.block_color.is_some() {
        return Some("--block-color/-A");
    }
    if args.no_glow {
        return Some("--no-glow/-g");
    }
    if args.glow_color.is_some() {
        return Some("--glow-color/-G");
    }
    if args.hilight_links {
        return Some("--hilight-links/-L");
    }
    if args.link_color.is_some() {
        return Some("--link-color/-U");
    }
    if args.no_link_underl {
        return Some("--no-link-underl/-u");
    }
    if args.block_width.is_some() {
        return Some("--block-width/-W");
    }
    if args.block_height.is_some() {
        return Some("--block-height/-H");
    }
    if args.glow_radius.is_some() {
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

fn make_default_out_file(input_file: &Path) -> PathBuf {
    let mut base = input_file.to_string_lossy().to_string();

    if base.ends_with(".nfo") {
        base.truncate(base.len() - 4);
    }

    base.push_str("-utf8.nfo");
    PathBuf::from(base)
}
