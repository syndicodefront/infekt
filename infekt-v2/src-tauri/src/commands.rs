use std::path::Path;

use crate::{nfo_data::NfoData, LoadedNfoState};

#[derive(serde::Deserialize)]
#[serde(deny_unknown_fields)]
#[serde(rename_all = "camelCase")]
pub struct LoadNfoRequest {
    file_path: String,
}

#[derive(serde::Serialize)]
#[serde(rename_all = "camelCase")]
pub struct LoadNfoResponse {
    success: bool,
    message: Option<String>,
}

#[derive(Debug, serde::Serialize)]
pub enum CommandError {
    NfoNotLoadedError,
}

#[tauri::command]
pub fn load_nfo(
    req: LoadNfoRequest,
    loaded_nfo_state: tauri::State<LoadedNfoState>,
) -> LoadNfoResponse {
    let mut loaded_nfo_state_guarded = loaded_nfo_state.0.lock().unwrap();

    let mut nfo_data = NfoData::new();
    let load_result = nfo_data.load_from_file(Path::new(&req.file_path));

    if load_result.is_ok() {
        println!("Charset name = {}", nfo_data.get_charset_name());
        *loaded_nfo_state_guarded = nfo_data;
    }

    LoadNfoResponse {
        success: load_result.is_ok(),
        message: match load_result {
            Ok(()) => None,
            Err(msg) => Some(msg),
        },
    }
}

#[tauri::command]
pub fn get_nfo_renderer_grid(
    loaded_nfo_state: tauri::State<LoadedNfoState>,
) -> Result<crate::nfo_renderer_grid::NfoRendererGrid, CommandError> {
    let mut loaded_nfo_state_guarded = loaded_nfo_state.0.lock().unwrap();

    let nfo_data = &mut *loaded_nfo_state_guarded;
    let grid = nfo_data.get_renderer_grid();

    if grid.is_none() {
        return Err(CommandError::NfoNotLoadedError);
    }

    Ok(grid.unwrap().clone())
}

#[tauri::command]
pub fn get_nfo_html_classic(
    loaded_nfo_state: tauri::State<LoadedNfoState>,
) -> Result<String, CommandError> {
    let loaded_nfo_state_guarded = loaded_nfo_state.0.lock().unwrap();
    let nfo_data = &*loaded_nfo_state_guarded;

    if !nfo_data.is_loaded() {
        return Err(CommandError::NfoNotLoadedError);
    }

    Ok(nfo_data.get_classic_html())
}

#[tauri::command]
pub fn get_nfo_html_stripped(
    loaded_nfo_state: tauri::State<LoadedNfoState>,
) -> Result<String, CommandError> {
    let loaded_nfo_state_guarded = loaded_nfo_state.0.lock().unwrap();
    let nfo_data = &*loaded_nfo_state_guarded;

    if !nfo_data.is_loaded() {
        return Err(CommandError::NfoNotLoadedError);
    }

    Ok(nfo_data.get_stripped_html())
}
