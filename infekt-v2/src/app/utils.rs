use super::InfektApp;

impl InfektApp {
    pub(super) fn show_error_message_popup(&mut self, message: String) {
        rfd::MessageDialog::new()
            .set_level(rfd::MessageLevel::Error)
            .set_title("Error")
            .set_description(&message)
            .show();
    }
}
