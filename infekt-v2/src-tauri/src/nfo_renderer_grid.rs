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
