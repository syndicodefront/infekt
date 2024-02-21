// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

mod commands;
mod infekt_core;
mod nfo_data;
mod nfo_renderer_grid;

use crate::nfo_data::NfoData;
use std::sync::Mutex;

pub struct LoadedNfoState(pub Mutex<NfoData>);

// we only ever access NfoData through the locked Mutex, so...:
unsafe impl Send for LoadedNfoState {}
unsafe impl Sync for LoadedNfoState {}

fn main() {
    tauri::Builder::default()
        .plugin(tauri_plugin_dialog::init())
        .manage(LoadedNfoState(Mutex::new(NfoData::new())))
        .invoke_handler(tauri::generate_handler![
            commands::load_nfo,
            commands::get_nfo_renderer_grid,
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
