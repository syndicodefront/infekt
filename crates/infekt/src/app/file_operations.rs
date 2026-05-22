use iced::Task;
use std::path::PathBuf;

use super::{Action, InfektApp, Message};

impl InfektApp {
    pub(super) fn task_open_nfo_file_dialog(&mut self) -> Task<Message> {
        iced::window::run(self.main_window_id.unwrap(), move |w| {
            let path = rfd::FileDialog::new()
                .set_parent(&w)
                .add_filter("Block Art Files", &["nfo", "diz", "asc", "txt"])
                .pick_file();

            Message::OpenFile(path)
        })
    }

    pub(super) fn action_load_new_nfo(&mut self, file_path: Option<PathBuf>) -> Action {
        let Some(file_path) = file_path else {
            return Action::None;
        };

        let status = self.current_nfo.load_from_file(&file_path);

        if status.is_err() {
            return Action::ShowErrorMessage(format!(
                "Failed to load file: {}",
                status.err().unwrap()
            ));
        }

        Action::None
    }
}
