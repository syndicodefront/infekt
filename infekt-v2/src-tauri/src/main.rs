// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

mod commands;
mod infekt_core;
mod nfo_data;


pub struct InnerGameState {
    pub foo: String,
}

impl InnerGameState {
    pub fn reset(&mut self) {
        // do stuff
    }
}


fn main() {
    tauri::Builder::default()
        .plugin(tauri_plugin_dialog::init())
        .invoke_handler(tauri::generate_handler![
            commands::load_nfo,
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
