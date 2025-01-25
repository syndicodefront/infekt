use iced::widget::{
    column, container, image, rich_text, row, span, text, Space,
};
use iced::{Element, Task};

use crate::app::Action;

#[derive(Default)]
pub struct InfektPreferencesScreen {}

#[derive(Debug, Clone)]
pub enum Message {
    Dummy,
}

impl InfektPreferencesScreen {
    pub fn update(&mut self, message: Message) -> Action {
        match message {
            Message::Dummy => {}
        }

        Action::None
    }

    pub fn on_before_shown(&mut self) -> Option<Task<Message>> {
        None
    }

    pub fn view(&self) -> Element<Message> {
        text("Preferences").size(32).into()
    }
}
