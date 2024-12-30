use iced::Task;
use std::path::PathBuf;

use super::{InfektApp, InfektUserAction, Message};

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

    pub(super) fn action_load_new_nfo(&mut self, file: Option<PathBuf>) -> InfektUserAction {
        let Some(file) = file else {
            return InfektUserAction::None;
        };

        let status = self.current_nfo_data.load_from_file(&file);

        if !status.is_ok() {
            return InfektUserAction::ShowErrorMessage(format!(
                "Failed to load file: {}",
                status.err().unwrap()
            ));
        }

        self.current_nfo_path = Some(file);

        let _renderer_grid = self.current_nfo_data.get_renderer_grid();

        InfektUserAction::None
    }
}
