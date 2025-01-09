mod args;
mod bundle;
mod utils;

use anyhow::{Context, Result};
use pico_args::Arguments;

use args::BundleArgs;

fn run() -> Result<()> {
    let mut args = Arguments::from_env();

    match args
        .subcommand()?
        .context("no subcommand provided")?
        .as_str()
    {
        "bundle" => {
            let cmd_args = BundleArgs::parse(&mut args)?;
            bundle::cmd_bundle(cmd_args)
        }
        _ => anyhow::bail!("unknown subcommand"),
    }
}

fn main() {
    if let Err(e) = run() {
        eprintln!("error: {}", e);
        std::process::exit(1);
    }
}
