// Prevent console window in addition to Slint window in Windows release builds when, e.g., starting the app via file manager. Ignored on other platforms.
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

mod infekt_core;
mod nfo_data;
mod nfo_renderer_grid;
mod nfo_to_html;

use crate::nfo_data::NfoData;
use native_dialog::FileDialog;
use slint::SharedString;
use std::error::Error;

slint::include_modules!();

fn main() -> Result<(), Box<dyn Error>> {
    let ui = AppWindow::new()?;
    let mut current_nfo_file = NfoData::new();

    ui.on_sidebar_clicked_open_file({
        let ui_handle = ui.as_weak();

        move || {
            // let ui = ui_handle.unwrap();

            let file_path = FileDialog::new()
                .add_filter("Supported files", &["nfo", "asc", "diz"])
                .show_open_single_file()
                .unwrap_or_default();

            if file_path.is_none() {
                return;
            }

            current_nfo_file
                .load_from_file(file_path.unwrap().as_path())
                .unwrap();

            ui_handle.unwrap().set_loaded_nfo_classic(
                SharedString::from(current_nfo_file.get_classic_text()));

            ui_handle.unwrap().set_loaded_nfo_text_only(
                SharedString::from(current_nfo_file.get_stripped_text()));
        }
    });

    ui.run()?;

    Ok(())
}
