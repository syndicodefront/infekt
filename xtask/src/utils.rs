use std::env;
use std::env::consts::OS;
use std::path::{Path, PathBuf};
use std::process::Command;

use anyhow::{Context, Result};
use cargo_toml::{Manifest, Package};

const CARGO_MANIFEST_DIR: &str = env!("CARGO_MANIFEST_DIR");

pub fn get_workspace_dir() -> Result<PathBuf> {
    Path::new(CARGO_MANIFEST_DIR)
        .ancestors()
        .nth(1)
        .context("workspace directory not found")
        .map(|p| p.to_path_buf())
}

pub fn get_package_manifest(toml_path: &Path) -> Result<Package> {
    Manifest::from_path(toml_path)
        .map_err(|e| anyhow::anyhow!("failed to parse the package manifest: {}", e))?
        .package
        .context("the target file is not a package manifest")
}

pub fn run_cargo(args: &[String]) -> Result<()> {
    let cargo = std::env::var_os("CARGO").unwrap_or_else(|| "cargo".into());
    if Command::new(cargo).args(args).status()?.success() {
        Ok(())
    } else {
        anyhow::bail!("cargo command failed")
    }
}

pub fn get_target_dir(project_root: &Path, target: &Option<String>, release: bool) -> PathBuf {
    let mut path = std::env::var("CARGO_TARGET_DIR")
        .map(PathBuf::from)
        .unwrap_or_else(|_| project_root.join("target"));

    if let Some(target) = target {
        path.push(target);
    }

    let profile = if release { "release" } else { "debug" };
    path.push(profile);
    path
}

pub fn get_target_os(target: &Option<String>) -> String {
    target
        .as_ref()
        .map(|t| t.split('-').nth(2))
        .flatten()
        .unwrap_or(OS)
        .replace("darwin", "macos")
}

pub fn get_binary_suffix(target: &Option<String>) -> String {
    if get_target_os(target) == "windows" {
        ".exe".to_string()
    } else {
        "".to_string()
    }
}
