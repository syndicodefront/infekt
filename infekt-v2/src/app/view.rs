use iced::widget::{column, row};
use iced::Length::Fill;
use iced::{Alignment, Element};

use super::{ActiveScreen, InfektApp, Message};

impl InfektApp {
    pub fn view(&self) -> Element<Message> {
        let sidebar = self.sidebar.view().map(Message::SidebarMessage);
        let content = column![]
            .push(match self.active_screen {
                ActiveScreen::MainView => self
                    .main_view
                    .view(&self.current_nfo)
                    .map(Message::MainViewMessage),
                ActiveScreen::Preferences => self
                    .preferences_screen
                    .view()
                    .map(Message::PreferencesScreenMessage),
                ActiveScreen::About => self.about_screen.view().map(Message::AboutScreenMessage),
            })
            .height(Fill)
            .width(Fill);

        row![sidebar, content]
            .padding(0)
            .align_y(Alignment::Start)
            .into()
    }
}
