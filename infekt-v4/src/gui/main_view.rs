use iced::alignment::Horizontal;
use iced::widget::scrollable::{Direction, Id, Scrollbar};
use iced::widget::{column, row, scrollable, text};
use iced::Element;
use iced::Length::{Fill, FillPortion};
use iced_aw::{TabLabel, Tabs};

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
pub enum InfektMainViewMessage {
    TabSelected(TabId),
}

impl InfektMainView {
    pub fn update(&mut self, message: InfektMainViewMessage) -> Action {
        match message {
            InfektMainViewMessage::TabSelected(selected) => self.active_tab = selected,
        }

        Action::None
    }

    pub fn view<'a>(&self, current_nfo: &'a NfoData) -> Element<'a, InfektMainViewMessage> {
        // XXX: could use some optimization, keep in mind that ideally we preserve
        // scroll positions etc. when switching tabs.

        Tabs::new(InfektMainViewMessage::TabSelected)
            .push(
                TabId::Rendered,
                TabLabel::Text("Rendered".to_owned()),
                self.rendered_tab(current_nfo),
            )
            .push(
                TabId::Classic,
                TabLabel::Text("Classic".to_owned()),
                self.classic_tab(current_nfo),
            )
            .push(
                TabId::TextOnly,
                TabLabel::Text("Text-Only".to_owned()),
                self.text_only_tab(current_nfo),
            )
            .push(
                TabId::FileInfo,
                TabLabel::Text("Properties".to_owned()),
                self.file_info_tab(current_nfo),
            )
            .set_active_tab(&self.active_tab)
            .tab_bar_position(iced_aw::TabBarPosition::Top)
            .into()
    }

    fn rendered_tab<'a>(&self, current_nfo: &'a NfoData) -> Element<'a, InfektMainViewMessage> {
        scrollable(NfoViewRendered::new(7, 12, current_nfo))
            .id(Id::new("main view rendered"))
            .direction(Direction::Both {
                vertical: Scrollbar::default(),
                horizontal: Scrollbar::default(),
            })
            .width(Fill)
            .height(Fill)
            .into()
    }

    // XXX: combine class and text_only view implementations
    fn classic_tab<'a>(&self, current_nfo: &'a NfoData) -> Element<'a, InfektMainViewMessage> {
        scrollable(
            text(current_nfo.get_classic_text())
                .font(iced::Font::with_name("Server Mono"))
                .shaping(text::Shaping::Advanced)
                .wrapping(text::Wrapping::None),
        )
        .id(Id::new("main view classic"))
        .direction(Direction::Both {
            vertical: Scrollbar::default(),
            horizontal: Scrollbar::default(),
        })
        .width(Fill)
        .height(Fill)
        .into()
    }

    fn text_only_tab<'a>(&self, current_nfo: &'a NfoData) -> Element<'a, InfektMainViewMessage> {
        scrollable(
            text(current_nfo.get_stripped_text())
                .font(iced::Font::with_name("Server Mono"))
                .wrapping(text::Wrapping::None),
        )
        .id(Id::new("main view text only"))
        .direction(Direction::Both {
            vertical: Scrollbar::default(),
            horizontal: Scrollbar::default(),
        })
        .width(Fill)
        .height(Fill)
        .into()
    }

    fn file_info_tab<'a>(&self, current_nfo: &'a NfoData) -> Element<'a, InfektMainViewMessage> {
        if !current_nfo.is_loaded() {
            return column![].into();
        }

        fn label<'a, T>(s: T) -> Element<'a, InfektMainViewMessage>
        where
            T: text::IntoFragment<'a>,
        {
            text(s)
                .width(FillPortion(1))
                .align_x(Horizontal::Right)
                .into()
        }

        fn value<'a, T>(s: T) -> Element<'a, InfektMainViewMessage>
        where
            T: text::IntoFragment<'a>,
        {
            text(s).width(FillPortion(4)).into()
        }

        const ROW_SPACING: u16 = 10;

        column![
            row![
                label("File path:"),
                value(
                    current_nfo
                        .get_file_path()
                        .map(|p| p.to_string_lossy())
                        .unwrap_or_default()
                )
            ]
            .spacing(ROW_SPACING),
            row![label("Charset:"), value(current_nfo.get_charset_name())].spacing(ROW_SPACING),
            row![
                label("Dimensions:"),
                value(format!(
                    "{} columns x {} lines",
                    current_nfo.get_renderer_grid().unwrap().width,
                    current_nfo.get_renderer_grid().unwrap().height
                ))
            ]
            .spacing(ROW_SPACING),
        ]
        .spacing(2)
        .padding(10)
        .width(Fill)
        .height(Fill)
        .into()
    }
}
