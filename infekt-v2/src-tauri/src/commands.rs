use std::path::Path;

use crate::nfo_data::NfoData;

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
pub fn load_nfo(req: LoadNfoRequest) -> LoadNfoResponse {
    let mut nfo_data = NfoData::new();
    let load_result = nfo_data.load_from_file(Path::new(&req.file_path));

    if load_result.is_ok() {
        println!("Charset name = {}", nfo_data.get_charset_name())
    }

    LoadNfoResponse {
        success: load_result.is_ok(),
        message: match load_result {
            Ok(()) => None,
            Err(msg) => Some(msg),
        },
    }
}
