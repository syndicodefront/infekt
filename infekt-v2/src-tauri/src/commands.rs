use cxx::let_cxx_string;
use crate::infekt_core;

#[derive(serde::Deserialize)]
#[serde(deny_unknown_fields)]
#[serde(rename_all = "camelCase")]
pub struct LoadNfoRequest {
    file_path: String,
    return_browseable_files: bool,
}

#[derive(serde::Serialize)]
#[serde(rename_all = "camelCase")]
pub struct LoadNfoResponse {
    success: bool,
    message: Option<String>,
    browseable_file_paths: Option<Vec<String>>,
}

#[tauri::command]
pub fn load_nfo(req: LoadNfoRequest) -> LoadNfoResponse {
    println!("I have no idea what I'm doing! Path =  {}", req.file_path);

    let mut nfo_data = infekt_core::ffi::new_nfo_data();
    let_cxx_string!(path=req.file_path);

    LoadNfoResponse {
        success: nfo_data.pin_mut().LoadFromFile(&path),
        message: None,
        browseable_file_paths: if req.return_browseable_files {
            Some(Vec::new())
        } else {
            None
        },
    }
}
