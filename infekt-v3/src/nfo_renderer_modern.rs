use crate::nfo_renderer_grid::NfoRendererBlockShape;
use crate::nfo_renderer_grid::NfoRendererGrid;
use tiny_skia::{Color, FillRule, Paint, PathBuilder, Pixmap, Size, Transform};

pub fn render_nfo_modern(grid: &NfoRendererGrid) -> Option<Pixmap> {
    let mut pixmap = Pixmap::new((grid.width * 7) as u32, (grid.height * 12) as u32).unwrap();

    pixmap.fill(Color::from_rgba8(255, 255, 255, 255));

    render_blocks(&grid, Color::from_rgba8(180, 60, 60, 255), &mut pixmap);

    Some(pixmap)
}

fn render_blocks(grid: &NfoRendererGrid, block_color: Color, pixmap: &mut Pixmap) {
    if !grid.has_blocks {
        return;
    }

    let block_size = Size::from_wh(7.0, 12.0).unwrap();

    let mut path_builder = PathBuilder::new();
    let mut paint = Paint::default();
    let mut previous_opacity = 1;

    paint.set_color(block_color);

    for line in &grid.lines {
        let row = line.row;
        let y = row as f32 * block_size.height();

        for block_group in &line.block_groups {
            let mut block_index = 0;

            for block_shape in &block_group.blocks {
                let x = (block_group.col + block_index) as f32 * block_size.width();

                let opacity = match block_shape {
                    NfoRendererBlockShape::FullBlockLightShade => 90,
                    NfoRendererBlockShape::FullBlockMediumShade => 140,
                    NfoRendererBlockShape::FullBlockDarkShade => 190,
                    _ => 1,
                };

                if opacity != previous_opacity {
                    if !path_builder.is_empty() {
                        pixmap.fill_path(
                            &path_builder.finish().unwrap(),
                            &paint,
                            FillRule::Winding,
                            Transform::identity(),
                            None,
                        );

                        path_builder = PathBuilder::new();
                    }

                    let mut new_color = block_color.clone();
                    new_color.apply_opacity(opacity as f32 / 255.0);

                    paint.set_color(new_color);
                    previous_opacity = opacity;
                }

                draw_block(x, y, &block_size, &block_shape, &mut path_builder);

                block_index += 1;
            }
        }
    }

    if path_builder.is_empty() {
        return;
    }

    pixmap.fill_path(
        &path_builder.finish().unwrap(),
        &paint,
        FillRule::Winding,
        Transform::identity(),
        None,
    );
}

fn draw_block(
    x: f32,
    y: f32,
    block_size: &Size,
    block_shape: &NfoRendererBlockShape,
    path_builder: &mut PathBuilder,
) {
    let half_block_size =
        Size::from_wh(block_size.width() * 0.5, block_size.height() * 0.5).unwrap();
    let half_vertical_block_size =
        Size::from_wh(half_block_size.width(), block_size.height()).unwrap();
    let half_horizontal_block_size =
        Size::from_wh(block_size.width(), half_block_size.height()).unwrap();
    let three_quarters_block_size =
        Size::from_wh(block_size.width() * 0.75, block_size.height() * 0.75).unwrap();

    match block_shape {
        NfoRendererBlockShape::FullBlock
        | NfoRendererBlockShape::FullBlockLightShade
        | NfoRendererBlockShape::FullBlockMediumShade
        | NfoRendererBlockShape::FullBlockDarkShade => {
            path_builder.push_rect(block_size.to_rect(x, y).unwrap());
        }
        NfoRendererBlockShape::LowerHalf => {
            path_builder.push_rect(
                half_horizontal_block_size
                    .to_rect(x, y + half_horizontal_block_size.height())
                    .unwrap(),
            );
        }
        NfoRendererBlockShape::UpperHalf => {
            path_builder.push_rect(half_horizontal_block_size.to_rect(x, y).unwrap());
        }
        NfoRendererBlockShape::RightHalf => {
            path_builder.push_rect(
                half_vertical_block_size
                    .to_rect(x + half_vertical_block_size.width(), y)
                    .unwrap(),
            );
        }
        NfoRendererBlockShape::LeftHalf => {
            path_builder.push_rect(half_vertical_block_size.to_rect(x, y).unwrap());
        }
        NfoRendererBlockShape::BlackSquare => {
            let bs = three_quarters_block_size;

            path_builder.push_rect(
                bs.to_rect(
                    x + (block_size.width() - bs.width()) * 0.5,
                    y + (block_size.height() - bs.height()) * 0.5,
                )
                .unwrap(),
            );
        }
        NfoRendererBlockShape::BlackSquareSmall => {
            path_builder.push_rect(
                half_block_size
                    .to_rect(
                        x + block_size.width() * 0.25,
                        y + block_size.height() * 0.25,
                    )
                    .unwrap(),
            );
        }
        _ => {}
    }
}
