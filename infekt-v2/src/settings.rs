use palette::rgb::{Rgb, Rgba};
use std::hash::Hasher;

#[derive(Debug, Clone)]
pub struct NfoRenderSettings {
    pub enhanced_view_block_width: u16,
    pub enhanced_view_block_height: u16,
    pub classic_font_size: f32,
    pub background_color: Rgb,
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
            enhanced_view_block_width: 7,
            enhanced_view_block_height: 12,
            classic_font_size: 14.0,
            background_color: Rgb::new(1.0, 1.0, 1.0),
            text_color: Rgba::new(0.0, 0.0, 0.0, 1.0),
            art_color: Rgba::new(0.529, 0.808, 0.922, 1.0),
            font_name: String::from("Cascadia Mono"),
            blur_enabled: true,
            blur_color: Rgba::new(0.5, 0.5, 0.5, 1.0),
            blur_radius: 15,
            blur_enabled_for_ansi_art: true,
            hyperlink_color: Rgba::new(0.0, 0.0, 1.0, 1.0),
            hyperlink_underline: true,
        }
    }
}

impl NfoRenderSettings {
    pub fn hash(&self) -> u64 {
        let mut hasher = rustc_hash::FxHasher::default();

        hasher.write_u16(self.enhanced_view_block_width);
        hasher.write_u16(self.enhanced_view_block_height);
        hasher.write_i64((self.classic_font_size * 10000.0) as i64);

        hash_rgb(&mut hasher, self.background_color);
        hash_rgba(&mut hasher, self.text_color);
        hash_rgba(&mut hasher, self.art_color);

        hasher.write(self.font_name.as_bytes());
        hasher.write_u8(self.blur_enabled as u8);
        hash_rgba(&mut hasher, self.blur_color);
        hasher.write_u16(self.blur_radius);
        hasher.write_u8(self.blur_enabled_for_ansi_art as u8);
        hash_rgba(&mut hasher, self.hyperlink_color);
        hasher.write_u8(self.hyperlink_underline as u8);

        hasher.finish()
    }
}

fn hash_rgb(hasher: &mut rustc_hash::FxHasher, rgb: Rgb) {
    let red = (rgb.red * 1_000_000.0) as u32;
    let green = (rgb.green * 1_000_000.0) as u32;
    let blue = (rgb.blue * 1_000_000.0) as u32;

    hasher.write_u32(red);
    hasher.write_u32(green);
    hasher.write_u32(blue);
}

fn hash_rgba(hasher: &mut rustc_hash::FxHasher, rgba: Rgba) {
    let red = (rgba.red * 1_000_000.0) as u32;
    let green = (rgba.green * 1_000_000.0) as u32;
    let blue = (rgba.blue * 1_000_000.0) as u32;
    let alpha = (rgba.alpha * 1_000_000.0) as u32;

    hasher.write_u32(red);
    hasher.write_u32(green);
    hasher.write_u32(blue);
    hasher.write_u32(alpha);
}
