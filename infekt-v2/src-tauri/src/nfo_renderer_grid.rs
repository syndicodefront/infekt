use crate::infekt_core;
use cxx::UniquePtr;
use std::u32;

#[derive(Clone, serde::Serialize)]
#[serde(rename_all = "camelCase")]
pub struct NfoRendererGrid {
    pub width: usize,
    pub height: usize,
    pub lines: Vec<NfoRendererLine>,
    pub has_blocks: bool,
}

#[derive(Clone, serde::Serialize)]
#[serde(rename_all = "camelCase")]
pub struct NfoRendererLine {
    pub row: usize,
    pub block_groups: Vec<NfoRendererBlockGroup>,
    pub text_flights: Vec<NfoRendererTextFlight>,
    pub links: Vec<NfoRendererLink>,
}

#[derive(Clone, serde::Serialize)]
#[serde(rename_all = "camelCase")]
pub struct NfoRendererBlockGroup {
    pub col: usize,
    pub blocks: Vec<NfoRendererBlockShape>,
}

#[derive(Clone, Debug, PartialEq, Eq, serde_repr::Serialize_repr)]
#[repr(i32)]
pub enum NfoRendererBlockShape {
    NoBlock = 0,
    Whitespace,
    WhitespaceInText,
    FullBlock,
    FullBlockLightShade,
    FullBlockMediumShade,
    FullBlockDarkShade,
    LowerHalf,
    UpperHalf,
    RightHalf,
    LeftHalf,
    BlackSquare,
    BlackSquareSmall,
}

#[derive(Clone, serde::Serialize)]
#[serde(rename_all = "camelCase")]
pub struct NfoRendererTextFlight {
    pub col: usize,
    pub text: String,
}

#[derive(Clone, serde::Serialize)]
#[serde(rename_all = "camelCase")]
pub struct NfoRendererLink {
    pub col: usize,
    pub text: String,
    pub url: String,
}

pub fn get_block_shape(char: u32, in_text: bool) -> NfoRendererBlockShape {
    match char {
        0x0000 | 0x0009 | 0x0020 => {
            if in_text {
                NfoRendererBlockShape::WhitespaceInText
            } else {
                NfoRendererBlockShape::Whitespace
            }
        }
        0x2588 => NfoRendererBlockShape::FullBlock,
        0x2591 => NfoRendererBlockShape::FullBlockLightShade,
        0x2592 => NfoRendererBlockShape::FullBlockMediumShade,
        0x2593 => NfoRendererBlockShape::FullBlockDarkShade,
        0x2580 => NfoRendererBlockShape::UpperHalf,
        0x2584 => NfoRendererBlockShape::LowerHalf,
        0x258C => NfoRendererBlockShape::LeftHalf,
        0x2590 => NfoRendererBlockShape::RightHalf,
        0x25A0 => NfoRendererBlockShape::BlackSquare,
        0x25AA => NfoRendererBlockShape::BlackSquareSmall,
        _ => NfoRendererBlockShape::NoBlock,
    }
}

pub fn make_renderer_grid(nfo: &UniquePtr<infekt_core::ffi::CNFOData>) -> NfoRendererGrid {
    let width = nfo.GetGridWidth();
    let height = nfo.GetGridHeight();

    let mut renderer_grid = NfoRendererGrid {
        width,
        height,
        lines: Vec::with_capacity(height),
        has_blocks: false,
    };

    for row in 0..height {
        let mut line = NfoRendererLine {
            row,
            block_groups: Vec::new(),
            text_flights: Vec::new(),
            links: Vec::new(),
        };

        let mut text_started = false;
        let mut text_buf_first_col = usize::MAX;
        let mut text_buf = String::with_capacity(128);
        let mut link_url: Option<String> = None;

        let mut block_group_first_col = usize::MAX;
        let mut block_group_buf: Vec<NfoRendererBlockShape> = Vec::new();

        for col in 0..width {
            let grid_char = nfo.GetGridCharUtf32(row, col);

            if grid_char == 0 { // EOL
                break;
            }

            let block_shape = get_block_shape(grid_char, text_started);

            if block_shape == NfoRendererBlockShape::NoBlock
                || block_shape == NfoRendererBlockShape::WhitespaceInText
            {
                // It's not a block, so must be text.
                text_started = true;

                if text_buf_first_col == usize::MAX {
                    text_buf_first_col = col;
                }

                let link = nfo.GetLinkUrlUtf8(row, col);
                let now_in_link = !link.is_empty();
                let buffer_is_link = link_url != None;

                if now_in_link != buffer_is_link {
                    text_buf.truncate(text_buf.trim_end_matches(' ').len());

                    if !text_buf.is_empty() {
                        if buffer_is_link {
                            line.links.push(NfoRendererLink {
                                col: text_buf_first_col,
                                text: text_buf,
                                url: link_url.unwrap(),
                            });
                        } else {
                            line.text_flights.push(NfoRendererTextFlight {
                                col: text_buf_first_col,
                                text: text_buf,
                            });
                        }
                    }

                    text_buf = String::with_capacity(128);
                    text_buf_first_col = col;
                    link_url = now_in_link.then(|| link.to_string());
                }

                text_buf.push_str(unsafe {
                    std::str::from_utf8_unchecked(nfo.GetGridCharUtf8(row, col).as_bytes())
                });
            } else if block_shape != NfoRendererBlockShape::Whitespace {
                // It's a block!

                if !block_group_buf.is_empty()
                    && col != block_group_first_col + block_group_buf.len()
                {
                    // End of block group.
                    line.block_groups.push(NfoRendererBlockGroup {
                        col: block_group_first_col,
                        blocks: block_group_buf,
                    });

                    block_group_buf = Vec::new();
                }

                if block_group_buf.is_empty() {
                    block_group_first_col = col;
                }

                block_group_buf.push(block_shape);

                if text_started {
                    // Preserve whitespace between non-whitespace chars.
                    text_buf.push(' ');
                }

                renderer_grid.has_blocks = true;
            } else if text_started {
                // Preserve whitespace between non-whitespace chars.
                text_buf.push(' ');
            }
        }

        if !block_group_buf.is_empty() {
            line.block_groups.push(NfoRendererBlockGroup {
                col: block_group_first_col,
                blocks: block_group_buf,
            });
        }

        text_buf.truncate(text_buf.trim_end_matches(' ').len());

        if !text_buf.is_empty() {
            if link_url != None {
                line.links.push(NfoRendererLink {
                    col: text_buf_first_col,
                    text: text_buf,
                    url: link_url.unwrap(),
                });
            } else {
                line.text_flights.push(NfoRendererTextFlight {
                    col: text_buf_first_col,
                    text: text_buf,
                });
            }
        }

        renderer_grid.lines.push(line);
    }

    renderer_grid
}
