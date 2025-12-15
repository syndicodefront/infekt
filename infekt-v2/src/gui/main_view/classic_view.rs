use super::{InfektMainView, Message};

use iced::widget::scrollable::{Direction, Scrollbar};
use iced::widget::{self, Text, container, scrollable, text};
use iced::Element;
use iced::Length::{Fill, Shrink};

use crate::core::nfo_data::NfoData;

impl InfektMainView {
    pub(super) fn classic_tab<'a>(
        &self,
        current_nfo: &'a NfoData,
        stripped: bool,
    ) -> Element<'a, Message> {
        let scrollable_id = widget::Id::new(if stripped {
            "main view stripped"
        } else {
            "main view classic"
        });
        let _has_blocks = !stripped && current_nfo.has_blocks();

        scrollable(
            container(
                (if stripped {
                    self.stripped_content(current_nfo)
                } else {
                    self.classic_content(current_nfo)
                })
                .font(iced::Font::with_name("Cascadia Mono"))
                .size(14.0)
                .line_height(text::LineHeight::Relative(1.0))
                .shaping(text::Shaping::Advanced)
                .wrapping(text::Wrapping::None),
            )
            .center_x(Shrink)
            .padding(25),
        )
        .id(scrollable_id)
        .direction(Direction::Both {
            vertical: Scrollbar::default(),
            horizontal: Scrollbar::default(),
        })
        .width(Fill)
        .height(Fill)
        .into()
    }

    fn classic_content<'a>(&self, current_nfo: &'a NfoData) -> Text<'a> {
        if !current_nfo.has_blocks() {
            return self.stripped_content(current_nfo);
        }

        text(current_nfo.get_classic_text())
    }

    fn stripped_content<'a>(&self, current_nfo: &'a NfoData) -> Text<'a> {
        text(current_nfo.get_stripped_text())
    }
}
