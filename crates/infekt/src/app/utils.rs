use iced::Task;

use super::{InfektApp, Message};

impl InfektApp {
    pub(super) fn show_error_message_popup(&mut self, message: String) -> Task<Message> {
        iced::window::run(self.main_window_id.unwrap(), move |w| {
            rfd::MessageDialog::new()
                .set_parent(&w)
                .set_level(rfd::MessageLevel::Error)
                .set_title("Error")
                .set_description(&message)
                .show();

            Message::NoOp
        })
    }
}
