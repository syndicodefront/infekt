use crate::infekt_core;
use crate::nfo_renderer_grid::{get_block_shape, NfoRendererBlockShape};
use cxx::UniquePtr;
use htmlentity::entity::*;

#[derive(PartialEq, Clone, Copy)]
enum CharFlightType {
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
        let mut previous_type = CharFlightType::Unknown;

        for col in 0..width {
            let grid_char = nfo.GetGridCharUtf32(row, col);

            if grid_char == 0 {
                // EOL
                break;
            }

            let mut link_url = String::new();
            let block_shape = get_block_shape(grid_char, false);
            let mut new_type = CharFlightType::Unknown;
            let mut could_be_text = false;

            if block_shape == NfoRendererBlockShape::Whitespace {
                if previous_type == CharFlightType::Link {
                    could_be_text = true;
                } else {
                    // whitespace between blocks and within text doesn't change the type
                    new_type = previous_type;
                }
            } else if block_shape == NfoRendererBlockShape::NoBlock {
                could_be_text = true;
            } else {
                new_type = CharFlightType::Block;
            }

            if could_be_text {
                let raw_link_url = nfo.GetLinkUrlUtf8(row, col);

                if !raw_link_url.is_empty() {
                    link_url = raw_link_url.to_string();
                    new_type = CharFlightType::Link;
                } else {
                    new_type = CharFlightType::Text;
                }
            }

            if new_type != CharFlightType::Unknown && new_type != previous_type {
                if previous_type == CharFlightType::Link {
                    html.push_str("</a>");
                } else if previous_type != CharFlightType::Unknown {
                    html.push_str("</span>");
                }

                match new_type {
                    CharFlightType::Block => {
                        html.push_str("<span class=\"nfo-block\" style=\"color: crimson\">");
                    }
                    CharFlightType::Text => {
                        html.push_str("<span class=\"nfo-text\">");
                    }
                    CharFlightType::Link => {
                        let encoded_link_url = encode(
                            link_url.as_bytes(),
                            &EncodeType::Named,
                            &CharacterSet::SpecialChars,
                        )
                        .to_string();

                        html.push_str("<a href=\"");
                        html.push_str(&encoded_link_url.unwrap_or_else(|_| String::from("#")));
                        html.push_str("\" target=\"_blank\">");
                    }
                    _ => {}
                }

                previous_type = new_type;
            }

            char::from_u32(grid_char).and_then(|real_char| {
                encode_char(&real_char, &EncodeType::NamedOrDecimal).and_then(|unwrapped| {
                    html.push_str(&unwrapped.to_string());
                    None::<()>
                })
            });
        }

        if previous_type == CharFlightType::Link {
            html.push_str("</a>");
        } else if previous_type != CharFlightType::Unknown {
            html.push_str("</span>");
        }

        html.push('\n');
    }

    html
}
