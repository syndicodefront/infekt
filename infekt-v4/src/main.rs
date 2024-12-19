use gui::sidebar::InfektSidebarMessage;
use iced::widget::{column, row, text};
use iced::Length::Fill;
use iced::{Alignment, Element, Size, Task};

mod gui;

use crate::gui::sidebar::InfektSidebar;

pub fn main() -> iced::Result {
    iced::application("iNFekt NFO Viewer", Counter::update, Counter::view)
        .antialiasing(true)
        .centered()
        .window_size(Size::new(850.0, 700.0))
        .run()
}

#[derive(Default)]
struct Counter {
    value: i64,
    sidebar: InfektSidebar,
}

#[derive(Debug, Clone, Copy)]
enum Message {
    SidebarMessage(InfektSidebarMessage),
}

#[derive(Debug, Clone, Copy)]
enum InfektUserAction {
    None,
    Increment,
    Decrement,
}

impl Counter {
    fn update(&mut self, message: Message) -> Task<Message> {
        let action = match message {
            Message::SidebarMessage(message) => self.sidebar.update(message),
            _ => InfektUserAction::None,
        };

        match action {
            InfektUserAction::None => {}
            InfektUserAction::Increment => self.value += 1,
            InfektUserAction::Decrement => self.value -= 1,
        }

        Task::none()
    }

    fn view(&self) -> Element<Message> {
        let sidebar = self.sidebar.view().map(Message::SidebarMessage);
        let content = column![text(self.value).size(50),].height(Fill).width(Fill);

        row![sidebar, content]
            .padding(0)
            .align_y(Alignment::Start)
            .into()
    }
}
