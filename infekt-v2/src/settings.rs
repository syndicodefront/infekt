use palette::rgb::{Rgb, Rgba};

#[derive(Debug, Clone)]
pub struct NfoRenderSettings {
    pub enhanced_view_block_height: u16,
    pub enhanced_view_block_width: u16,
    pub classic_font_size: f32,
    pub back_color: Rgb,
    pub text_color: Rgba,
    pub art_color: Rgba,
    pub font_name: String,
    pub blur_enabled: bool,
    pub blur_color: Rgba,
    pub blur_radius: u16,
    pub blur_enabled_for_ansi_art: bool,
    pub hyperlink_color: Rgba,
    pub hyperlink_underline: bool,
}

impl Default for NfoRenderSettings {
    fn default() -> Self {
        Self {
            enhanced_view_block_height: 12,
            enhanced_view_block_width: 7,
            classic_font_size: 14.0,
            back_color: Rgb::new(1.0, 1.0, 1.0),
            text_color: Rgba::new(0.0, 0.0, 0.0, 1.0),
            art_color: Rgba::new(0.0, 0.0, 0.0, 1.0),
            font_name: String::from("monospace"),
            blur_enabled: true,
            blur_color: Rgba::new(0.5, 0.5, 0.5, 1.0),
            blur_radius: 15,
            blur_enabled_for_ansi_art: true,
            hyperlink_color: Rgba::new(0.0, 0.0, 1.0, 1.0),
            hyperlink_underline: true,
        }
    }
}
