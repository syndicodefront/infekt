use crate::settings::NfoRenderSettings;
use iced::theme::Palette;
use iced::{Color, Theme};
use palette::rgb::Rgb;
use palette::{FromColor, Hsl};
use std::sync::Arc;

pub(super) fn create_theme(settings: Arc<NfoRenderSettings>) -> Theme {
    let inspiration = if is_dark(settings.background_color) {
        Theme::Dark
    } else {
        Theme::Light
    }
    .palette();

    // Most probably some more fancy logic will be needed here.

    Theme::custom(
        "iNfekt".to_owned(),
        // format!("iNFekt-{}", settings.hash()),
        Palette {
            background: Color::from_rgb(
                settings.background_color.red,
                settings.background_color.green,
                settings.background_color.blue,
            ),
            text: Color::from_rgb(
                settings.text_color.red,
                settings.text_color.green,
                settings.text_color.blue,
            ), // XXX: should actually depend on the background color to ensure constrast
            primary: Color::from_rgb(
                settings.art_color.red,
                settings.art_color.green,
                settings.art_color.blue,
            ),
            success: inspiration.success,
            warning: inspiration.warning,
            danger: inspiration.danger,
        },
    )
}

fn is_dark(color: Rgb) -> bool {
    // copied from iced codebase
    Hsl::from_color(color).lightness < 0.6
}
