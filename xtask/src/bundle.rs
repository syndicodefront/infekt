use std::path::Path;
use std::vec;

// Based on https://github.com/sonodima/blurthing
// THANK YOU!

use anyhow::{Context, Result};
use cargo_toml::Package;
use tauri_bundler::{
    AppCategory, BundleBinary, BundleSettings, DmgSettings, MacOsSettings, PackageSettings,
    PackageType, Position, SettingsBuilder, Size, WindowsSettings, WixSettings,
};

use crate::args::BundleArgs;
use crate::utils;

const TARGET_PACKAGE: &str = "infekt-v2";

const PRODUCT_NAME: &str = "iNFekt";
const BUNDLE_IDENTIFIER: &str = "org.syndicode.infekt2";
const COPYRIGHT: &str = "Copyright © 2010-2022 syndicode";
const CATEGORY: AppCategory = AppCategory::Utility;

// const LICENSE_FILE: &str = "LICENSE";
// const DMG_BACKGROUND: &str = "dmg-background.jpg";

pub fn cmd_bundle(args: BundleArgs) -> Result<()> {
    let workspace_dir = utils::get_workspace_dir()?;
    let toml_path = workspace_dir.join(TARGET_PACKAGE).join("Cargo.toml");

    let manifest = utils::get_package_manifest(&toml_path)?;
    compile_package(manifest.name().to_string(), args.release, &args.target)?;

    let binary_suffix = utils::get_binary_suffix(&args.target);
    let binary_name = format!("{}{}", manifest.name(), binary_suffix);
    let main_binary = BundleBinary::new(binary_name, true);

    let target_dir = utils::get_target_dir(&workspace_dir, &args.target, args.release);
    let mut settings_builder = SettingsBuilder::new()
        .package_settings(package_settings(&manifest)?)
        .bundle_settings(bundle_settings(&workspace_dir))
        .package_types(package_types(&args.target))
        .binaries(vec![main_binary])
        .project_out_directory(target_dir);

    if let Some(target) = args.target {
        settings_builder = settings_builder.target(target);
    }

    let settings = settings_builder
        .build()
        .context("failed to create the bundler settings")?;

    tauri_bundler::bundle_project(&settings)
        .map_err(|e| anyhow::anyhow!("failed to bundle the project: {}", e))
        .map(|_| ())
}

fn compile_package(package: String, release: bool, target: &Option<String>) -> Result<()> {
    let mut build_args = vec!["build".to_string(), "--bin".to_string(), package];

    if release {
        build_args.push("--release".to_string());
    }

    if let Some(target) = target {
        build_args.push("--target".to_string());
        build_args.push(target.to_string());
    }

    utils::run_cargo(&build_args)
}

fn package_settings(manifest: &Package) -> Result<PackageSettings> {
    let authors = manifest.authors().iter().map(|s| s.to_string()).collect();

    Ok(PackageSettings {
        product_name: PRODUCT_NAME.to_string(),
        version: manifest.version().to_string(),
        description: manifest
            .description()
            .context("description is not set in the package manifest")?
            .to_string(),
        homepage: manifest.homepage().map(|s| s.to_string()),
        authors: Some(authors),
        // license: manifest.license().map(|s| s.to_string()),
        default_run: manifest.default_run.as_ref().map(|s| s.to_string()),
    })
}

fn bundle_settings(workspace_dir: &Path) -> BundleSettings {
    let icon = workspace_dir
        .join("infekt-v2")
        .join("assets")
        .join("infekt-icons")
        .join("*")
        .to_string_lossy()
        .to_string();

    BundleSettings {
        identifier: Some(BUNDLE_IDENTIFIER.to_string()),
        icon: Some(vec![icon]),
        copyright: Some(COPYRIGHT.to_string()),
        category: Some(CATEGORY),
        macos: macos_settings(workspace_dir),
        dmg: dmg_settings(workspace_dir),
        windows: windows_settings(workspace_dir),
        ..Default::default()
    }
}

fn macos_settings(_workspace_dir: &Path) -> MacOsSettings {
    // let license_path = workspace_dir.join("assets").join(LICENSE_FILE);

    MacOsSettings {
        minimum_system_version: Some("10.12".into()), // MACOSX_DEPLOYMENT_TARGET - blurthing/build.rs
        signing_identity: Some("-".into()), // ad-hoc signing
        ..Default::default()
    }
}

fn dmg_settings(_workspace_dir: &Path) -> DmgSettings {
    // let background = workspace_dir.join("assets").join(DMG_BACKGROUND);

    DmgSettings {
        // background: Some(background),
        window_size: Size {
            width: 700,
            height: 500,
        },
        app_position: Position { x: 170, y: 230 },
        application_folder_position: Position { x: 530, y: 230 },
        ..Default::default()
    }
}

fn windows_settings(workspace_dir: &Path) -> WindowsSettings {
    WindowsSettings {
        wix: Some(wix_settings(workspace_dir)),
        ..Default::default()
    }
}

fn wix_settings(_workspace_dir: &Path) -> WixSettings {
    // let license_path = workspace_dir.join("assets").join(LICENSE_FILE);

    WixSettings {
        ..Default::default()
    }
}

fn package_types(target: &Option<String>) -> Vec<PackageType> {
    match utils::get_target_os(target).as_str() {
        "macos" => vec![PackageType::Dmg],
        "windows" => vec![PackageType::WindowsMsi],
        "linux" => vec![PackageType::Deb, PackageType::Rpm],
        _ => vec![],
    }
}
