use askama::Template;
use hex_color::HexColor;
use serde::Serialize;

use super::nfo_data::NfoData;
use super::nfo_renderer_grid::{NfoRendererBlockShape, NfoRendererGrid};

const RENDERER_CODE: &str = include_str!("../templates/static/nfo_canvas_renderer.js");

pub type NfoHtmlColor = HexColor;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct NfoHtmlCanvasSettings {
    pub block_width: usize,
    pub block_height: usize,
    pub color_back: NfoHtmlColor,
    pub color_text: NfoHtmlColor,
    pub color_art: NfoHtmlColor,
    pub font_bold: bool,
    pub shadow_enable: bool,
    pub shadow_radius: usize,
    pub shadow_color: NfoHtmlColor,
    pub hyperlinks_highlight: bool,
    pub hyperlinks_color: NfoHtmlColor,
    pub hyperlinks_underline: bool,
}

impl Default for NfoHtmlCanvasSettings {
    fn default() -> Self {
        Self {
            block_width: 7,
            block_height: 12,
            color_back: NfoHtmlColor::rgb(0xff, 0xff, 0xff),
            color_text: NfoHtmlColor::rgb(0, 0, 0),
            color_art: NfoHtmlColor::rgb(0, 0, 0),
            font_bold: false,
            shadow_enable: true,
            shadow_radius: 10,
            shadow_color: NfoHtmlColor::rgb(0, 0, 0),
            hyperlinks_highlight: false,
            hyperlinks_color: NfoHtmlColor::rgb(0, 0, 0xff),
            hyperlinks_underline: true,
        }
    }
}

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
struct CanvasSettingsJson {
    block_width: usize,
    block_height: usize,
    color_back: String,
    color_text: String,
    color_art: String,
    font_bold: bool,
    shadow_enable: bool,
    shadow_radius: usize,
    shadow_color: String,
    hyperlinks_highlight: bool,
    hyperlinks_color: String,
    hyperlinks_underline: bool,
}

impl From<&NfoHtmlCanvasSettings> for CanvasSettingsJson {
    fn from(settings: &NfoHtmlCanvasSettings) -> Self {
        Self {
            block_width: settings.block_width,
            block_height: settings.block_height,
            color_back: color_hex_rgb(settings.color_back),
            color_text: color_hex_rgb(settings.color_text),
            color_art: color_hex_rgb(settings.color_art),
            font_bold: settings.font_bold,
            shadow_enable: settings.shadow_enable,
            shadow_radius: settings.shadow_radius,
            shadow_color: color_hex_rgb(settings.shadow_color),
            hyperlinks_highlight: settings.hyperlinks_highlight,
            hyperlinks_color: color_hex_rgb(settings.hyperlinks_color),
            hyperlinks_underline: settings.hyperlinks_underline,
        }
    }
}

#[derive(Serialize)]
struct CanvasRenderJson {
    width: usize,
    height: usize,
    blocks: Vec<CanvasBlockRow>,
    links: Vec<CanvasLinkFlight>,
    text: Vec<CanvasTextFlight>,
}

#[derive(Serialize)]
struct CanvasBlockRow {
    row: usize,
    b: Vec<Vec<usize>>,
}

#[derive(Serialize)]
struct CanvasTextFlight {
    row: usize,
    col: usize,
    t: String,
}

#[derive(Serialize)]
struct CanvasLinkFlight {
    row: usize,
    col: usize,
    t: String,
    href: String,
}

#[derive(Template)]
#[template(path = "nfo_canvas.html")]
struct CanvasHtmlTemplate<'a> {
    render_settings: CanvasSettingsJson,
    nfo_data: CanvasRenderJson,
    renderer_code: &'a str,
}

pub(super) fn nfo_to_html_canvas_render_json(nfo: &NfoData) -> String {
    let Some(grid) = nfo.get_renderer_grid() else {
        return String::new();
    };

    serde_json::to_string(&make_canvas_render_json(grid)).unwrap_or_default()
}

pub(super) fn nfo_to_html_canvas_full_html(
    nfo: &NfoData,
    settings: &NfoHtmlCanvasSettings,
) -> String {
    let Some(grid) = nfo.get_renderer_grid() else {
        return String::new();
    };

    CanvasHtmlTemplate {
        render_settings: CanvasSettingsJson::from(settings),
        nfo_data: make_canvas_render_json(grid),
        renderer_code: RENDERER_CODE,
    }
    .render()
    .unwrap_or_default()
}

fn color_hex_rgb(color: NfoHtmlColor) -> String {
    let (red, green, blue) = color.split_rgb();
    format!("{red:02x}{green:02x}{blue:02x}")
}

fn make_canvas_render_json(grid: &NfoRendererGrid) -> CanvasRenderJson {
    let mut render = CanvasRenderJson {
        width: grid.width,
        height: grid.height,
        blocks: Vec::new(),
        links: Vec::new(),
        text: Vec::new(),
    };

    for line in &grid.lines {
        let block_spans: Vec<Vec<usize>> = line
            .block_groups
            .iter()
            .filter_map(|group| {
                let mut span = Vec::with_capacity(group.blocks.len() + 1);
                span.push(group.col);

                for block in &group.blocks {
                    span.push(canvas_block_index(block)?);
                }

                Some(span)
            })
            .collect();

        if !block_spans.is_empty() {
            render.blocks.push(CanvasBlockRow {
                row: line.row,
                b: block_spans,
            });
        }

        render
            .text
            .extend(line.text_flights.iter().map(|flight| CanvasTextFlight {
                row: line.row,
                col: flight.col,
                t: flight.text.clone(),
            }));

        render
            .links
            .extend(line.links.iter().map(|link| CanvasLinkFlight {
                row: line.row,
                col: link.col,
                t: link.text.clone(),
                href: link.url.clone(),
            }));
    }

    render
}

fn canvas_block_index(block: &NfoRendererBlockShape) -> Option<usize> {
    match block {
        NfoRendererBlockShape::FullBlock => Some(0),
        NfoRendererBlockShape::FullBlockLightShade => Some(1),
        NfoRendererBlockShape::FullBlockMediumShade => Some(2),
        NfoRendererBlockShape::FullBlockDarkShade => Some(3),
        NfoRendererBlockShape::LowerHalf => Some(4),
        NfoRendererBlockShape::UpperHalf => Some(5),
        NfoRendererBlockShape::RightHalf => Some(6),
        NfoRendererBlockShape::LeftHalf => Some(7),
        NfoRendererBlockShape::BlackSquare => Some(8),
        NfoRendererBlockShape::BlackSquareSmall => Some(9),
        _ => None,
    }
}

#[cfg(test)]
mod tests {
    use serde_json::Value;
    use std::sync::atomic::{AtomicUsize, Ordering};

    use super::*;

    fn load_bytes(bytes: &[u8]) -> NfoData {
        static COUNTER: AtomicUsize = AtomicUsize::new(0);

        let path = std::env::temp_dir().join(format!(
            "infekt-core-canvas-test-{}-{}.nfo",
            std::process::id(),
            COUNTER.fetch_add(1, Ordering::Relaxed)
        ));
        std::fs::write(&path, bytes).unwrap();

        let mut data = NfoData::new();
        data.load_from_file(&path).unwrap();
        let _ = std::fs::remove_file(path);

        data
    }

    #[test]
    fn exports_legacy_canvas_json_shape() {
        let data = load_bytes("A █░▒▓▄▀▐▌■▪ hxxp://example.com\n".as_bytes());
        let value: Value = serde_json::from_str(&data.get_canvas_render_json()).unwrap();

        assert_eq!(value["width"], 31);
        assert_eq!(value["height"], 1);
        assert_eq!(value["blocks"][0]["row"], 0);
        assert_eq!(
            value["blocks"][0]["b"][0],
            serde_json::json!([2, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9])
        );
        assert_eq!(
            value["text"][0],
            serde_json::json!({"row": 0, "col": 0, "t": "A"})
        );
        assert_eq!(value["links"][0]["col"], 13);
        assert_eq!(value["links"][0]["href"], "http://example.com");
    }

    #[test]
    fn embeds_script_safe_json_in_html() {
        let data = load_bytes(b"</script> hxxp://example.com\n");
        let html = data.get_canvas_html(&NfoHtmlCanvasSettings::default());

        assert!(html.contains("const renderSettings = "));
        assert!(html.contains("const nfoData = "));
        assert!(html.contains("const NfoRenderer = class"));
        assert!(html.contains("\\u003c/script\\u003e"));
    }

    #[test]
    fn serializes_canvas_settings_with_legacy_names() {
        let settings = NfoHtmlCanvasSettings {
            block_width: 8,
            block_height: 16,
            hyperlinks_highlight: true,
            ..Default::default()
        };
        let json = serde_json::to_value(CanvasSettingsJson::from(&settings)).unwrap();

        assert_eq!(json["blockWidth"], 8);
        assert_eq!(json["blockHeight"], 16);
        assert_eq!(json["colorBack"], "ffffff");
        assert_eq!(json["hyperlinksHighlight"], true);
    }
}
