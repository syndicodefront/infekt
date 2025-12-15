mod classic_view;
mod file_info;

use std::sync::Arc;

use iced::widget::scrollable::{Direction, Scrollbar};
use iced::widget::{self, button, column, row, scrollable, text};
use iced::Element;
use iced::Length::Fill;

use crate::app::Action;
use crate::core::nfo_data::NfoData;
use crate::settings::NfoRenderSettings;

use super::widget::enhanced_nfo_view::EnhancedNfoView;

#[derive(Default)]
pub struct InfektMainView {
    active_tab: TabId,
    active_render_settings: Arc<NfoRenderSettings>,
}

#[derive(Clone, PartialEq, Eq, Debug, Default, Copy)]
pub enum TabId {
    #[default]
    Enhanced,
    Classic,
    TextOnly,
    FileInfo,
}

#[derive(Debug, Clone)]
pub enum Message {
    TabSelected(TabId),
    RenderSettingsChanged(Arc<NfoRenderSettings>),
}

impl InfektMainView {
    pub fn update(&mut self, message: Message) -> Action {
        match message {
            Message::TabSelected(selected) => self.active_tab = selected,
            Message::RenderSettingsChanged(settings) => self.active_render_settings = settings,
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
                tab_button("Enhanced", TabId::Enhanced),
                tab_button("Classic", TabId::Classic),
                tab_button("Text-Only", TabId::TextOnly),
                tab_button("Properties", TabId::FileInfo),
            ]
            .spacing(1),
            match self.active_tab {
                TabId::Enhanced => self.enhanced_tab(current_nfo),
                TabId::Classic => self.classic_tab(current_nfo, false),
                TabId::TextOnly => self.classic_tab(current_nfo, true),
                TabId::FileInfo => self.file_info_tab(current_nfo),
            }
        ]
        .into()
    }

    fn enhanced_tab<'a>(&self, current_nfo: &'a NfoData) -> Element<'a, Message> {
        scrollable(EnhancedNfoView::new(self.active_render_settings.clone(), current_nfo))
            .id(widget::Id::new("enhanced view"))
            .direction(Direction::Both {
                vertical: Scrollbar::default(),
                horizontal: Scrollbar::default(),
            })
            .width(Fill)
            .height(Fill)
            .into()
    }
}
