use iced::widget::{column, row, text};
use iced::Length::Fill;
use iced::{Alignment, Element};

use super::{InfektApp, Message, InfektActiveScreen};

impl InfektApp {
    pub fn view(&self) -> Element<Message> {
        let sidebar = self.sidebar.view().map(Message::SidebarMessage);
        let content = column![]
            .push(match self.active_screen {
                InfektActiveScreen::MainView => self.main_view.view().map(Message::MainViewMessage),
                InfektActiveScreen::Preferences => text("Preferences").into(),
                InfektActiveScreen::About => {
                    self.about_screen.view().map(Message::AboutScreenMessage)
                }
            })
            .height(Fill)
            .width(Fill);

        row![sidebar, content]
            .padding(0)
            .align_y(Alignment::Start)
            .into()
    }
}
