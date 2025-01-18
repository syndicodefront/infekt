mod classic_view;
mod file_info;

use iced::widget::scrollable::{Direction, Id as ScrollableId, Scrollbar};
use iced::widget::{button, column, row, scrollable, text};
use iced::Element;
use iced::Length::Fill;

use crate::app::Action;
use crate::core::nfo_data::NfoData;

use super::widget::nfo_view_rendered::NfoViewRendered;

#[derive(Default)]
pub struct InfektMainView {
    active_tab: TabId,
}

#[derive(Clone, PartialEq, Eq, Debug, Default, Copy)]
pub enum TabId {
    #[default]
    Rendered,
    Classic,
    TextOnly,
    FileInfo,
}

#[derive(Debug, Clone)]
pub enum Message {
    TabSelected(TabId),
}

impl InfektMainView {
    pub fn update(&mut self, message: Message) -> Action {
        match message {
            Message::TabSelected(selected) => self.active_tab = selected,
        }

        Action::None
    }

    pub fn view<'a>(&self, current_nfo: &'a NfoData) -> Element<'a, Message> {
        let tab_button = |label: &'a str, tab_id: TabId| -> Element<'a, Message> {
            let mut button = button(text(label).center()).width(Fill);

            if current_nfo.is_loaded() {
                button = button.on_press(Message::TabSelected(tab_id));
            }

            button.into()
        };

        column![
            row![
                tab_button("Rendered", TabId::Rendered),
                tab_button("Classic", TabId::Classic),
                tab_button("Text-Only", TabId::TextOnly),
                tab_button("Properties", TabId::FileInfo),
            ]
            .spacing(1),
            match self.active_tab {
                TabId::Rendered => self.rendered_tab(current_nfo),
                TabId::Classic => self.classic_tab(current_nfo, false),
                TabId::TextOnly => self.classic_tab(current_nfo, true),
                TabId::FileInfo => self.file_info_tab(current_nfo),
            }
        ]
        .into()
    }

    fn rendered_tab<'a>(&self, current_nfo: &'a NfoData) -> Element<'a, Message> {
        scrollable(NfoViewRendered::new(7, 12, current_nfo))
            .id(ScrollableId::new("main view rendered"))
            .direction(Direction::Both {
                vertical: Scrollbar::default(),
                horizontal: Scrollbar::default(),
            })
            .width(Fill)
            .height(Fill)
            .into()
    }
}
