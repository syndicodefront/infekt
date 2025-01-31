use std::sync::Arc;

use iced::widget::{column, container, pick_list, rich_text, row, span, text, Space};
use iced::Length::Fill;
use iced::{Element, Task};

use crate::app::Action;
use crate::settings::NfoRenderSettings;

mod named_colors;

#[derive(Default)]
pub struct InfektPreferencesScreen {
    temporary_settings: Arc<NfoRenderSettings>,

    background_color: Option<colornames::Color>,
    text_color: Option<colornames::Color>,
    art_color: Option<colornames::Color>,
    hyperlink_color: Option<colornames::Color>,
}

#[derive(Debug, Clone)]
pub enum Message {
    BackgroundColorSelected(colornames::Color),
    TextColorSelected(colornames::Color),
    ArtColorSelected(colornames::Color),
    HyperlinkColorSelected(colornames::Color),
}

impl InfektPreferencesScreen {
    pub fn update(&mut self, message: Message) -> Action {
        match message {
            Message::BackgroundColorSelected(color) => {
                self.background_color = Some(color);
            }
            Message::TextColorSelected(color) => {
                self.text_color = Some(color);
            }
            Message::ArtColorSelected(color) => {
                self.art_color = Some(color);
            }
            Message::HyperlinkColorSelected(color) => {
                self.hyperlink_color = Some(color);
            }
        }

        let mut updated_settings = (*self.temporary_settings).clone();

        updated_settings.background_color =
            named_colors::to_palette_rgb(self.background_color.unwrap());
        updated_settings.text_color = named_colors::to_palette_rgba(self.text_color.unwrap());
        updated_settings.art_color = named_colors::to_palette_rgba(self.art_color.unwrap());
        updated_settings.hyperlink_color =
            named_colors::to_palette_rgba(self.hyperlink_color.unwrap());

        self.temporary_settings = Arc::new(updated_settings);

        Action::None
    }

    pub fn on_before_shown(
        &mut self,
        active_render_settings: Arc<NfoRenderSettings>,
    ) -> Option<Task<Message>> {
        self.temporary_settings = active_render_settings;
        self.background_color =
            named_colors::from_palette_rgb(self.temporary_settings.background_color);
        self.text_color = named_colors::from_palette_rgba(self.temporary_settings.text_color);
        self.art_color = named_colors::from_palette_rgba(self.temporary_settings.art_color);
        self.hyperlink_color =
            named_colors::from_palette_rgba(self.temporary_settings.hyperlink_color);

        None
    }

    pub fn view(&self) -> Element<Message> {
        column![self.view_settings_row(), self.view_preview_row(),]
            .width(Fill)
            .into()
    }

    fn view_settings_row(&self) -> Element<Message> {
        let background_color_pick_list = pick_list(
            &named_colors::ALL[..],
            self.background_color,
            Message::BackgroundColorSelected,
        );

        let text_color_pick_list = pick_list(
            &named_colors::ALL[..],
            self.text_color,
            Message::TextColorSelected,
        );

        let art_color_pick_list = pick_list(
            &named_colors::ALL[..],
            self.art_color,
            Message::ArtColorSelected,
        );

        let hyperlink_color_pick_list = pick_list(
            &named_colors::ALL[..],
            self.hyperlink_color,
            Message::HyperlinkColorSelected,
        );

        container(column![
            row![
                text("Background Color").width(Fill),
                background_color_pick_list,
            ],
            row![text("Text Color").width(Fill), text_color_pick_list,],
            row![text("Block Art Color").width(Fill), art_color_pick_list,],
            row![text("Links Color").width(Fill), hyperlink_color_pick_list,]
        ])
        .style(container::rounded_box)
        .into()
    }

    fn view_preview_row(&self) -> Element<Message> {
        row![
            container(text("Enhanced").size(32)).width(Fill),
            container(text("Classic").size(32)).width(Fill)
        ]
        .into()
    }
}
