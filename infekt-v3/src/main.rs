// Prevent console window in addition to Slint window in Windows release builds when, e.g., starting the app via file manager. Ignored on other platforms.
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

mod infekt_core;
mod nfo_data;
mod nfo_renderer_grid;
mod nfo_renderer_modern;
mod nfo_to_html;

use crate::nfo_data::NfoData;
use native_dialog::FileDialog;
use slint::{Image, Rgba8Pixel, SharedPixelBuffer};
use std::{
    error::Error,
    sync::{Arc, Mutex},
};

slint::include_modules!();

fn main() -> Result<(), Box<dyn Error>> {
    let ui = AppWindow::new()?;
    let current_nfo_file_global: Arc<Mutex<NfoData>> = Arc::new(Mutex::new(NfoData::new()));

    ui.on_sidebar_clicked_open_file({
        let ui_handle = ui.as_weak();
        let current_nfo_file_handle = current_nfo_file_global.clone();

        move || {
            let ui = ui_handle.unwrap();
            let mut current_nfo_file = current_nfo_file_handle.lock().unwrap();

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

            let renderer_grid = current_nfo_file.get_renderer_grid();

            ui.set_loaded_nfo_width_pixels(renderer_grid.unwrap().width as f32 * 7.0);
            ui.set_loaded_nfo_height_pixels(renderer_grid.unwrap().height as f32 * 12.0);

            //ui_handle.unwrap().set_loaded_nfo_classic(
            //    SharedString::from(current_nfo_file.get_classic_text()));

            //ui_handle.unwrap().set_loaded_nfo_text_only(
            //    SharedString::from(current_nfo_file.get_stripped_text()));
        }
    });

    let current_nfo_file_handle = current_nfo_file_global.clone();

    ui.on_render_loaded_nfo_image(move |_, _| -> Image {
        let mut current_nfo_file = current_nfo_file_handle.lock().unwrap();

        if !current_nfo_file.is_loaded() {
            return Image::default();
        }

        let grid_opt = current_nfo_file.get_renderer_grid();

        if grid_opt.is_none() {
            return Image::default();
        }

        let pixmap = nfo_renderer_modern::render_nfo_modern(&grid_opt.unwrap());

        let pixel_buffer = SharedPixelBuffer::<Rgba8Pixel>::clone_from_slice(
            pixmap.unwrap().data(),
            grid_opt.unwrap().width as u32 * 7,
            grid_opt.unwrap().height as u32 * 12,
        );

        Image::from_rgba8_premultiplied(pixel_buffer)
    });

    ui.run()?;

    Ok(())
}
