use super::{InfektMainView, Message};

use iced::alignment::Horizontal;
use iced::widget::{column, row, text};
use iced::Element;
use iced::Length::{Fill, FillPortion};

use crate::core::nfo_data::NfoData;

impl InfektMainView {
    pub(super) fn file_info_tab<'a>(&self, current_nfo: &'a NfoData) -> Element<'a, Message> {
        if !current_nfo.is_loaded() {
            return column![].into();
        }

        fn label<'a, T>(s: T) -> Element<'a, Message>
        where
            T: text::IntoFragment<'a>,
        {
            text(s)
                .width(FillPortion(1))
                .align_x(Horizontal::Right)
                .into()
        }

        fn value<'a, T>(s: T) -> Element<'a, Message>
        where
            T: text::IntoFragment<'a>,
        {
            text(s).width(FillPortion(4)).into()
        }

        const ROW_SPACING: f32 = 10.0;

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
