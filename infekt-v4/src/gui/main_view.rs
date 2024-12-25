use iced::widget::{column, scrollable, text};
use iced::Element;
use iced::Length::Fill;
use iced_aw::{TabLabel, Tabs};

use crate::InfektUserAction;

use super::nfo_view_rendered::nfo_view_rendered;

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
pub enum InfektMainViewMessage {
    TabSelected(TabId),
}

impl InfektMainView {
    pub fn update(&mut self, message: InfektMainViewMessage) -> InfektUserAction {
        match message {
            InfektMainViewMessage::TabSelected(selected) => self.active_tab = selected,
        }

        InfektUserAction::None
    }

    pub fn view(&self) -> Element<InfektMainViewMessage> {
        // XXX: why do we have to push the contents of all tabs,
        // when the active tab is the only one that will be displayed?

        Tabs::new(InfektMainViewMessage::TabSelected)
            .push(
                TabId::Rendered,
                TabLabel::Text("Rendered".to_owned()),
                scrollable(nfo_view_rendered(100.0))
                    .width(Fill)
                    .height(Fill),
            )
            .push(
                TabId::Classic,
                TabLabel::Text("Classic".to_owned()),
                column![text("Classic")],
            )
            .push(
                TabId::TextOnly,
                TabLabel::Text("Text-Only".to_owned()),
                column![text("Text-Only")],
            )
            .push(
                TabId::FileInfo,
                TabLabel::Text("File Information".to_owned()),
                column![text("File Information")],
            )
            .set_active_tab(&self.active_tab)
            .tab_bar_position(iced_aw::TabBarPosition::Top)
            .into()
    }
}
