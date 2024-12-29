use iced::Task;

use super::{InfektApp, Message};

impl InfektApp {
    pub(super) fn task_show_error_message(&mut self, message: String) -> Task<Message> {
        Task::perform(
            async move {
                rfd::AsyncMessageDialog::new()
                    .set_level(rfd::MessageLevel::Error)
                    .set_title("Error")
                    .set_description(&message)
                    .show()
                    .await
            },
            |_| Message::NoOp,
        )
    }
}
