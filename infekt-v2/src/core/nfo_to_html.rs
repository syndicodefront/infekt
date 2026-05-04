use htmlentity::entity::{encode, encode_char, CharacterSet, EncodeType, ICodedDataTrait};

use super::nfo_data::NfoData;
use super::nfo_renderer_grid::{get_block_shape, NfoRendererBlockShape};

#[derive(PartialEq, Clone, Copy)]
enum CharFlightType {
    Unknown,
    Block,
    Text,
    Link,
}

pub(super) fn nfo_to_html_classic(nfo: &NfoData) -> String {
    let mut html = String::with_capacity(nfo.grid_height() * 80);

    let width = nfo.grid_width();
    let height = nfo.grid_height();

    for row in 0..height {
        let mut previous_type = CharFlightType::Unknown;

        for col in 0..width {
            let grid_char = nfo.grid_char(row, col).map(|c| c as u32).unwrap_or(0);

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
                let raw_link_url = nfo.link_url(row, col).unwrap_or("");

                if !raw_link_url.is_empty() {
                    link_url = raw_link_url.to_owned();
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
