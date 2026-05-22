use askama::Template;
use hex_color::HexColor;
use serde::Serialize;

use super::nfo_data::NfoData;
use super::nfo_renderer_grid::{NfoRendererBlockShape, NfoRendererGrid, get_block_shape};

const RENDERER_CODE: &str = include_str!("../templates/static/nfo_canvas_renderer.js");

pub type NfoHtmlColor = HexColor;
pub type NfoHtmlCanvasSettings = NfoHtmlSettings;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct NfoHtmlSettings {
    pub block_width: usize,
    pub block_height: usize,
    pub font_size: usize,
    pub font_face: String,
    pub title: String,
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

impl Default for NfoHtmlSettings {
    fn default() -> Self {
        Self {
            block_width: 7,
            block_height: 12,
            font_size: 12,
            font_face: String::from("SF Mono"),
            title: String::new(),
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

pub struct NfoHtmlExporter<'a> {
    nfo: &'a NfoData,
    settings: NfoHtmlSettings,
}

impl<'a> NfoHtmlExporter<'a> {
    pub fn new(nfo: &'a NfoData, settings: NfoHtmlSettings) -> Self {
        Self { nfo, settings }
    }

    pub fn export_html(&self) -> String {
        if !self.nfo.is_loaded() {
            return String::new();
        }

        ClassicHtmlTemplate {
            settings: ClassicSettingsTemplate::from(&self.settings),
            content_html: make_classic_body_html(self.nfo),
        }
        .render()
        .unwrap_or_default()
    }

    pub fn export_canvas_html(&self) -> String {
        let Some(grid) = self.nfo.get_renderer_grid() else {
            return String::new();
        };

        CanvasHtmlTemplate {
            render_settings: CanvasSettingsJson::from(&self.settings),
            nfo_data: make_canvas_render_json(grid),
            renderer_code: RENDERER_CODE,
        }
        .render()
        .unwrap_or_default()
    }

    pub fn export_canvas_json(&self) -> String {
        let Some(grid) = self.nfo.get_renderer_grid() else {
            return String::new();
        };

        serde_json::to_string(&make_canvas_render_json(grid)).unwrap_or_default()
    }
}

#[derive(Template)]
#[template(path = "nfo_html.html")]
struct ClassicHtmlTemplate {
    settings: ClassicSettingsTemplate,
    content_html: String,
}

struct ClassicSettingsTemplate {
    title: String,
    font_face_css: String,
    font_size: usize,
    font_weight: &'static str,
    color_back: String,
    color_text: String,
    color_art: String,
    shadow_enable: bool,
    shadow_radius: usize,
    shadow_color: String,
    hyperlinks_highlight: bool,
    hyperlinks_color: String,
    hyperlinks_text_decoration: &'static str,
}

impl From<&NfoHtmlSettings> for ClassicSettingsTemplate {
    fn from(settings: &NfoHtmlSettings) -> Self {
        Self {
            title: settings.title.clone(),
            font_face_css: escape_css_string(&settings.font_face),
            font_size: settings.font_size,
            font_weight: if settings.font_bold { "bold" } else { "normal" },
            color_back: color_hex_rgb(settings.color_back),
            color_text: color_hex_rgb(settings.color_text),
            color_art: color_hex_rgb(settings.color_art),
            shadow_enable: settings.shadow_enable,
            shadow_radius: settings.shadow_radius,
            shadow_color: color_hex_rgb(settings.shadow_color),
            hyperlinks_highlight: settings.hyperlinks_highlight,
            hyperlinks_color: color_hex_rgb(settings.hyperlinks_color),
            hyperlinks_text_decoration: if settings.hyperlinks_underline {
                "underline"
            } else {
                "none"
            },
        }
    }
}

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
struct CanvasSettingsJson {
    block_width: usize,
    block_height: usize,
    font_size: usize,
    font_face: String,
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

impl From<&NfoHtmlSettings> for CanvasSettingsJson {
    fn from(settings: &NfoHtmlSettings) -> Self {
        Self {
            block_width: settings.block_width,
            block_height: settings.block_height,
            font_size: settings.font_size,
            font_face: settings.font_face.clone(),
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

#[derive(PartialEq, Clone, Copy)]
enum HtmlRunType {
    Unknown,
    Block,
    Text,
    Link,
}

fn make_classic_body_html(nfo: &NfoData) -> String {
    let mut html = String::with_capacity(nfo.grid_height() * 96);

    for row in 0..nfo.grid_height() {
        let mut current_type = HtmlRunType::Unknown;

        for col in 0..nfo.grid_width() {
            let Some(real_char) = nfo.grid_char(row, col) else {
                break;
            };
            let grid_char = real_char as u32;

            if grid_char == 0 {
                break;
            }

            let block_shape = get_block_shape(grid_char, false);
            let new_type;
            let mut link_url = None;

            if block_shape == NfoRendererBlockShape::NoBlock
                || (block_shape == NfoRendererBlockShape::Whitespace
                    && current_type == HtmlRunType::Link)
            {
                link_url = nfo.link_url(row, col);
                new_type = if link_url.is_some() {
                    HtmlRunType::Link
                } else {
                    HtmlRunType::Text
                };
            } else if block_shape == NfoRendererBlockShape::Whitespace {
                new_type = current_type;
            } else {
                new_type = HtmlRunType::Block;
            }

            if new_type != HtmlRunType::Unknown && new_type != current_type {
                close_run(&mut html, current_type);
                open_run(&mut html, new_type, link_url);
                current_type = new_type;
            }

            push_escaped_nfo_char(&mut html, real_char);
        }

        close_run(&mut html, current_type);
        html.push('\n');
    }

    html
}

fn open_run(html: &mut String, run_type: HtmlRunType, link_url: Option<&str>) {
    match run_type {
        HtmlRunType::Block => html.push_str("<span class=\"nfo_block\">"),
        HtmlRunType::Text => html.push_str("<span class=\"nfo_text\">"),
        HtmlRunType::Link => {
            html.push_str("<a href=\"");
            push_escaped_html_attr(html, link_url.unwrap_or("#"));
            html.push_str("\" class=\"nfo_link\">");
        }
        HtmlRunType::Unknown => {}
    }
}

fn close_run(html: &mut String, run_type: HtmlRunType) {
    match run_type {
        HtmlRunType::Link => html.push_str("</a>"),
        HtmlRunType::Block | HtmlRunType::Text => html.push_str("</span>"),
        HtmlRunType::Unknown => {}
    }
}

fn push_escaped_nfo_char(html: &mut String, ch: char) {
    match ch {
        '<' => html.push_str("&lt;"),
        '>' => html.push_str("&gt;"),
        '"' => html.push_str("&quot;"),
        '&' => html.push_str("&amp;"),
        '\u{20}'..='\u{7e}' => html.push(ch),
        _ => {
            html.push_str("&#");
            html.push_str(&(ch as u32).to_string());
            html.push(';');
        }
    }
}

fn push_escaped_html_attr(html: &mut String, input: &str) {
    for ch in input.chars() {
        match ch {
            '<' => html.push_str("&lt;"),
            '>' => html.push_str("&gt;"),
            '"' => html.push_str("&quot;"),
            '&' => html.push_str("&amp;"),
            '\'' => html.push_str("&#39;"),
            _ => html.push(ch),
        }
    }
}

fn escape_css_string(input: &str) -> String {
    let mut escaped = String::with_capacity(input.len());

    for ch in input.chars() {
        match ch {
            '\\' => escaped.push_str("\\\\"),
            '\'' => escaped.push_str("\\27 "),
            '\n' | '\r' | '\u{0c}' => escaped.push(' '),
            _ => escaped.push(ch),
        }
    }

    escaped
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
            "infekt-core-html-export-test-{}-{}.nfo",
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
    fn unloaded_data_exports_empty_strings() {
        let data = NfoData::new();
        let exporter = NfoHtmlExporter::new(&data, NfoHtmlSettings::default());

        assert!(exporter.export_html().is_empty());
        assert!(exporter.export_canvas_html().is_empty());
        assert!(exporter.export_canvas_json().is_empty());
    }

    #[test]
    fn exports_classic_html_document() {
        let data = load_bytes("A <&\"> █ hxxp://example.com/ä\n".as_bytes());
        let html = NfoHtmlExporter::new(&data, NfoHtmlSettings::default()).export_html();

        assert!(html.contains("<!DOCTYPE html>"));
        assert!(html.contains("<div class=\"infekt_nfo\"><pre>"));
        assert!(html.contains("<span class=\"nfo_text\">A &lt;&amp;&quot;&gt; </span>"));
        assert!(html.contains("<span class=\"nfo_block\">&#9608; </span>"));
        assert!(html.contains("<a href=\"http://example.com/\" class=\"nfo_link\">"));
        assert!(html.contains("hxxp://example.com/</a><span class=\"nfo_text\">&#228;</span>"));
        assert!(html.contains("font-family: 'SF Mono', monospace;"));
    }

    #[test]
    fn shared_settings_affect_classic_html() {
        let data = load_bytes(b"hello\n");
        let settings = NfoHtmlSettings {
            font_face: String::from("Test Font"),
            font_size: 15,
            font_bold: true,
            color_text: NfoHtmlColor::rgb(0x11, 0x22, 0x33),
            hyperlinks_highlight: true,
            ..Default::default()
        };
        let html = NfoHtmlExporter::new(&data, settings).export_html();

        assert!(html.contains("font-family: 'Test Font', monospace;"));
        assert!(html.contains("font-size: 15px;"));
        assert!(html.contains("font-weight: bold;"));
        assert!(html.contains("color: #112233;"));
        assert!(html.contains(".infekt_nfo a.nfo_link"));
    }

    #[test]
    fn exports_legacy_canvas_json_shape() {
        let data = load_bytes("A █░▒▓▄▀▐▌■▪ hxxp://example.com\n".as_bytes());
        let exporter = NfoHtmlExporter::new(&data, NfoHtmlSettings::default());
        let value: Value = serde_json::from_str(&exporter.export_canvas_json()).unwrap();

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
    fn embeds_script_safe_json_in_canvas_html() {
        let data = load_bytes(b"</script> hxxp://example.com\n");
        let html = NfoHtmlExporter::new(&data, NfoHtmlSettings::default()).export_canvas_html();

        assert!(html.contains("const renderSettings = "));
        assert!(html.contains("const nfoData = "));
        assert!(html.contains("const NfoRenderer = class"));
        assert!(html.contains("\\u003c/script\\u003e"));
    }

    #[test]
    fn serializes_canvas_settings_with_shared_defaults() {
        let settings = NfoHtmlSettings {
            block_width: 8,
            block_height: 16,
            hyperlinks_highlight: true,
            ..Default::default()
        };
        let json = serde_json::to_value(CanvasSettingsJson::from(&settings)).unwrap();

        assert_eq!(json["blockWidth"], 8);
        assert_eq!(json["blockHeight"], 16);
        assert_eq!(json["fontFace"], "SF Mono");
        assert_eq!(json["colorBack"], "ffffff");
        assert_eq!(json["hyperlinksHighlight"], true);
    }
}
