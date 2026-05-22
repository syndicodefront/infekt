#[inline]
pub fn to_iced_color_rgb(color: palette::rgb::Rgb) -> iced::Color {
    iced::Color::from_rgb(
        color.red,
        color.green,
        color.blue,
    )
}

#[inline]
pub fn to_iced_color(color: palette::rgb::Rgba) -> iced::Color {
    iced::Color::from_rgba(
        color.red,
        color.green,
        color.blue,
        color.alpha,
    )
}
