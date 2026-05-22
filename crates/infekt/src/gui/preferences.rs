use std::sync::Arc;

use iced::widget::{column, container, pick_list, row, text, PickList};
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

    font_name: Option<String>,
    //all_fonts: fontique::Collection,
    all_font_names: Vec<String>,
}

#[derive(Debug, Clone)]
pub enum Message {
    BackgroundColorSelected(colornames::Color),
    TextColorSelected(colornames::Color),
    ArtColorSelected(colornames::Color),
    HyperlinkColorSelected(colornames::Color),
    FontNamesLoaded(Box<Vec<String>>),
    FontNameSelected(String),
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
            Message::FontNameSelected(name) => {
                self.font_name = Some(name);
            }
            Message::FontNamesLoaded(names) => {
                self.all_font_names = *names;
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

        if self.all_font_names.is_empty() {
            return Some(Task::perform(
                async {
                    let mut db = fontdb::Database::new();

                    db.load_system_fonts();

                    let mut all_font_names = db
                        .faces()
                        .filter(|face| face.monospaced)
                        .filter(|face| !face.families.is_empty())
                        .map(|face| face.families[0].0.to_string())
                        .filter(|name| !name.is_empty() && !name.starts_with('.'))
                        .collect::<Vec<String>>();

                    all_font_names.sort();
                    all_font_names.dedup();

                    Box::new(all_font_names)
                },
                Message::FontNamesLoaded,
            ));
        }

        None
    }

    pub fn view(&self) -> Element<'_, Message> {
        column![self.view_settings_row(), self.view_preview_row(),]
            .width(Fill)
            .into()
    }

    fn view_settings_row(&self) -> Element<'_, Message> {
        let background_color_pick_list = pick_list(
            named_colors::ALL,
            self.background_color,
            Message::BackgroundColorSelected,
        );

        let text_color_pick_list = pick_list(
            named_colors::ALL,
            self.text_color,
            Message::TextColorSelected,
        );

        let art_color_pick_list = pick_list(
            named_colors::ALL,
            self.art_color,
            Message::ArtColorSelected,
        );

        let hyperlink_color_pick_list = pick_list(
            named_colors::ALL,
            self.hyperlink_color,
            Message::HyperlinkColorSelected,
        );

        let font_family_pick_list: PickList<'_, String, Vec<String>, String, Message> = pick_list(
            self.all_font_names.clone(),
            self.font_name.clone(),
            Message::FontNameSelected,
        )
        .text_shaping(text::Shaping::Advanced);

        container(column![
            row![
                text("Background Color").width(Fill),
                background_color_pick_list,
            ],
            row![text("Text Color").width(Fill), text_color_pick_list,],
            row![text("Block Art Color").width(Fill), art_color_pick_list,],
            row![text("Links Color").width(Fill), hyperlink_color_pick_list,],
            row![text("Font Family").width(Fill), font_family_pick_list,]
        ])
        .style(container::rounded_box)
        .into()
    }

    fn view_preview_row(&self) -> Element<'_, Message> {
        row![
            container(text("Enhanced").size(32)).width(Fill),
            container(text("Classic").size(32)).width(Fill)
        ]
        .into()
    }
}
