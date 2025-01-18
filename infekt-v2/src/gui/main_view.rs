use iced::alignment::Horizontal;
use iced::widget::scrollable::{Direction, Id, Scrollbar};
use iced::widget::{button, column, container, row, scrollable, text};
use iced::Element;
use iced::Length::{Fill, FillPortion, Shrink};

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
        fn tab_button<'a, T>(label: T, tab_id: TabId) -> Element<'a, InfektMainViewMessage>
        where
            T: text::IntoFragment<'a>,
        {
            button(container(text(label)).center_x(Fill))
                .width(Fill)
                .on_press(InfektMainViewMessage::TabSelected(tab_id))
                .into()
        }

        column![
            row![
                tab_button("Rendered", TabId::Rendered),
                tab_button("Classic", TabId::Classic),
                tab_button("Text-Only", TabId::TextOnly),
                tab_button("Properties", TabId::FileInfo),
            ],
            match self.active_tab {
                TabId::Rendered => self.rendered_tab(current_nfo),
                TabId::Classic => self.classic_tab(current_nfo, false),
                TabId::TextOnly => self.classic_tab(current_nfo, true),
                TabId::FileInfo => self.file_info_tab(current_nfo),
            }
        ]
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

    fn classic_tab<'a>(
        &self,
        current_nfo: &'a NfoData,
        stripped: bool,
    ) -> Element<'a, InfektMainViewMessage> {
        let has_blocks = !stripped && current_nfo.has_blocks();

        scrollable(
            container(
                text(if stripped {
                    current_nfo.get_stripped_text()
                } else {
                    current_nfo.get_classic_text()
                })
                .font(iced::Font::with_name("Monaco"))
                .size(14.0)
                .line_height(text::LineHeight::Relative(1.0))
                .shaping(text::Shaping::Advanced)
                .wrapping(text::Wrapping::None),
            )
            .center_x(Shrink)
            .padding(25),
        )
        //.id(Id::new("main view classic"))
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
