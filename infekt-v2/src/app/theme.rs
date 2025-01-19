use crate::settings::NfoRenderSettings;
use iced::theme::Palette;
use iced::{Color, Theme};
use palette::rgb::Rgb;
use palette::{FromColor, Hsl};
use std::sync::Arc;

pub(super) fn create_theme(settings: Arc<NfoRenderSettings>) -> Theme {
    let inspiration = if is_dark(settings.back_color) {
        Theme::Dark
    } else {
        Theme::Light
    }
    .palette();

    // Most probably some more fancy logic will be needed here.
    let theme = Theme::custom(
        "iNfekt".to_owned(),
        // format!("iNFekt-{}", settings.hash()),
        Palette {
            background: Color::from(settings.back_color),
            text: Color::from(settings.text_color), // XXX: should actually depend on the background color to ensure constrast
            primary: Color::from(settings.art_color),
            success: inspiration.success,
            warning: inspiration.warning,
            danger: inspiration.danger,
        },
    );

    theme
}

fn is_dark(color: Rgb) -> bool {
    // copied from iced codebase
    Hsl::from_color(color).lightness < 0.6
}
