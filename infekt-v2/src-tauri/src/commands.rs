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
