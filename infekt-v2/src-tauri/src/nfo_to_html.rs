use crate::infekt_core;
use crate::nfo_renderer_grid::{get_block_shape, NfoRendererBlockShape};
use cxx::UniquePtr;
use htmlentity::entity::*;

#[derive(PartialEq, Clone, Copy)]
enum CharColorType {
    Unknown,
    Block,
    Text,
    Link,
}

pub fn nfo_to_html_classic(nfo: &UniquePtr<infekt_core::ffi::CNFOData>) -> String {
    let mut html = String::with_capacity(nfo.GetGridHeight() * 80);

    let width = nfo.GetGridWidth();
    let height = nfo.GetGridHeight();

    for row in 0..height {
        let mut previous_type = CharColorType::Unknown;
        //let mut link_url: Option<String> = None;

        for col in 0..width {
            let grid_char = nfo.GetGridCharUtf32(row, col);

            if grid_char == 0 {
                // EOL
                break;
            }

            let mut new_type = CharColorType::Unknown;
            let block_shape = get_block_shape(grid_char, false);
            let mut link_url = String::new();

            if block_shape == NfoRendererBlockShape::Whitespace {
                new_type = previous_type;
                // TODO: deal with links
            } else if block_shape != NfoRendererBlockShape::NoBlock {
                new_type = CharColorType::Block;
            } else {
                let raw_link_url = nfo.GetLinkUrlUtf8(row, col);

                if !raw_link_url.is_empty() {
                    link_url = raw_link_url.to_string();
                }
            }

            if new_type != previous_type && new_type != CharColorType::Unknown {
                match new_type {
                    CharColorType::Block => {
                        html.push_str("<span class=\"nfo-block\">");
                    }
                    CharColorType::Text => {
                        html.push_str("<span class=\"nfo-text\">");
                    }
                    CharColorType::Link => {
                        let _ = encode(
                            link_url.as_bytes(),
                            &EncodeType::Named,
                            &CharacterSet::SpecialChars,
                        )
                        .to_string()
                        .and_then(|encoded_link_url| {
                            html.push_str("<a href=\"#");
                            html.push_str(&encoded_link_url);
                            html.push_str("\">");
                            Ok({})
                        });
                    }
                    _ => {}
                }

                previous_type = new_type;
            }

            char::from_u32(grid_char).and_then(|real_char| {
                match real_char {
                    '<' => html.push_str("&lt;"),
                    '>' => html.push_str("&gt;"),
                    '"' => html.push_str("&quot;"),
                    '&' => html.push_str("&amp;"),
                    '\x20'..='\x7f' => {
                        html.push(real_char);
                    }
                    _ => {
                        encode_char(&real_char, &EncodeType::Decimal).and_then(|char_entity| {
                            html.push_str(&char_entity.to_string());

                            Some(())
                        });
                    }
                }

                Some(())
            });
        }

        if previous_type == CharColorType::Link {
            html.push_str("</a>");
        } else if previous_type != CharColorType::Unknown {
            html.push_str("</span>");
        }

        html.push('\n');
    }

    html
}
