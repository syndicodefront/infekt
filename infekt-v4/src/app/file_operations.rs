use iced::Task;
use std::path::PathBuf;

use super::{InfektApp, InfektAppAction, Message};

impl InfektApp {
    pub(super) fn task_open_nfo_file_dialog(&mut self) -> Task<Message> {
        Task::perform(
            async {
                let file = rfd::AsyncFileDialog::new()
                    .add_filter("Block Art Files", &["nfo", "diz", "asc", "txt"])
                    .pick_file()
                    .await;

                if let Some(file) = file {
                    Some(file.path().to_path_buf())
                } else {
                    None
                }
            },
            |f| Message::OpenFile(f),
        )
    }

    pub(super) fn action_load_new_nfo(&mut self, file_path: Option<PathBuf>) -> InfektAppAction {
        let Some(file_path) = file_path else {
            return InfektAppAction::None;
        };

        let status = self.current_nfo.load_from_file(&file_path);

        if !status.is_ok() {
            return InfektAppAction::ShowErrorMessage(format!(
                "Failed to load file: {}",
                status.err().unwrap()
            ));
        }

        InfektAppAction::None
    }
}
