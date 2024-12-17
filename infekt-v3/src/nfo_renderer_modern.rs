use std::ops::Add;
use std::ops::Mul;

use crate::font::Font;
use crate::nfo_renderer_grid::NfoRendererBlockShape;
use crate::nfo_renderer_grid::NfoRendererGrid;
use swash::scale::image::Content;
use swash::scale::image::Image;
use swash::scale::Render;
use swash::scale::ScaleContext;
use swash::scale::Source;
use swash::scale::StrikeWith;
use swash::zeno::Format;
use swash::FontRef;
use swash::GlyphId;
use tiny_skia::PremultipliedColorU8;
use tiny_skia::{Color, FillRule, Paint, PathBuilder, Pixmap, Size, Transform};

use cosmic_text::{
    Attrs, Buffer, Color as CosmicTextColor, FontSystem, Metrics, Shaping, SwashCache,
};

struct RenderSettings {
    block_width: u32,
    block_height: u32,
}

pub fn render_nfo_modern(grid: &NfoRendererGrid) -> Option<Pixmap> {
    let settings = RenderSettings {
        block_width: 7,
        block_height: 12,
    };

    let mut pixmap = Pixmap::new(
        grid.width as u32 * settings.block_width,
        grid.height as u32 * settings.block_height,
    )
    .unwrap();

    pixmap.fill(Color::from_rgba8(255, 255, 255, 255));

    render_blocks(
        &grid,
        &settings,
        Color::from_rgba8(180, 60, 60, 255),
        &mut pixmap,
    );
    render_text(
        &grid,
        &settings,
        Color::from_rgba8(0, 0, 0, 255),
        &mut pixmap,
    );

    Some(pixmap)
}

fn render_blocks(
    grid: &NfoRendererGrid,
    settings: &RenderSettings,
    block_color: Color,
    pixmap: &mut Pixmap,
) {
    if !grid.has_blocks {
        return;
    }

    let block_size =
        Size::from_wh(settings.block_width as f32, settings.block_height as f32).unwrap();

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
                    _ => 255,
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

static EXAMPLE_FONT: &[u8] = include_bytes!("../assets/fonts/ServerMono-Regular.otf");

fn render_text(
    grid: &NfoRendererGrid,
    settings: &RenderSettings,
    text_color: Color,
    pixmap: &mut Pixmap,
) {
    /*
        let mut paint = Paint::default();
        paint.set_color(text_color);

        // A FontSystem provides access to detected system fonts, create one per application
        let mut font_system = FontSystem::new();

        // A SwashCache stores rasterized glyphs, create one per application
        let mut swash_cache = SwashCache::new();

        // Text metrics indicate the font size and line height of a buffer
        let metrics = Metrics::new(12.0, 12.0);

        // A Buffer provides shaping and layout for a UTF-8 string, create one per text widget
        let mut buffer = Buffer::new(&mut font_system, metrics);

        buffer.set_monospace_width(&mut font_system, Some(7.0));

        // Borrow buffer together with the font system for more convenient method calls
        //    let mut buffer = buffer.borrow_with(&mut font_system);

        // Set a size for the text buffer, in pixels
        buffer.set_size(&mut font_system, Some(1200.0), Some(700.0));

        // Attributes indicate what font to choose
        let attrs = Attrs::new();

        attrs.family(Family::Monospace);

        for line in &grid.lines {
            let row = line.row;

            for text_flight in &line.text_flights {
                buffer.set_text(&mut font_system, &text_flight.text, attrs, Shaping::Basic);

                if let Some(layouted) = buffer.line_layout(&mut font_system, 0) {
                    for glyph in layouted.first().unwrap().glyphs.iter() {
                        let physical_glyph = glyph.physical((0., 0.), 1.0);

                        swash_cache.with_pixels(
                            &mut font_system,
                            physical_glyph.cache_key,
                            CosmicTextColor::rgba(0, 0, 0, 1),
                            |x, y, _color| {
                                pixmap.fill_rect(
                                    Rect::from_xywh(
                                        (text_flight.col as i32 * 7 + physical_glyph.x + x) as f32,
                                        (row as i32 * 12 + physical_glyph.y + y) as f32,
                                        1.0,
                                        1.0,
                                    )
                                    .unwrap(),
                                    &paint,
                                    Transform::identity(),
                                    None,
                                );
                            },
                        );
                    }
                }
            }
        }
    }
    */

    let font = Font::from_bytes(EXAMPLE_FONT, 0).unwrap();
    let charmap = font.charmap();
    // let mut shape_context = ShapeContext::new();
    let mut scale_context = ScaleContext::new();

    /*
       let mut paint = Paint::default();
       paint.set_color(text_color);
       let transform = Transform::identity();
    */

    for line in &grid.lines {
        let row = line.row;

        for text_flight in &line.text_flights {
            let mut char_col = text_flight.col as u32;

            // XXX: we probably want to actually look at graphemes here.
            // Not sure how well the C++ part even supports that, so it's something for the future.
            for f in text_flight.text.chars() {
                let glyph_id = charmap.map(f);

                let img = render_glyph(
                    &mut scale_context,
                    &font.as_ref(),
                    13.0,
                    glyph_id,
                    text_color,
                );

                if let Some(img) = img {
                    let w = pixmap.width() as usize;
                    let x =
                        (img.placement.left + (char_col * settings.block_width) as i32) as usize;
                    let y =
                        (-img.placement.top + row as i32 * settings.block_height as i32) as usize;
                    let pixels = pixmap.pixels_mut();

                    /*
                    println!(
                        "img-placement: {:?} glyph: {:?} char: {:?} content: {:?} x: {:?} y: {:?}",
                        img.placement, glyph_id, f, img.content, x, y
                    );
                    */

                    match img.content {
                        Content::Mask => {
                            let mut i = 0;
                            for off_y in 0..img.placement.height as usize {
                                for off_x in 0..img.placement.width as usize {
                                    if img.data[i] != 0 {
                                        let idx = w.mul(y + off_y).add(x + off_x);
                                        let mut clr = text_color.clone();
                                        clr.set_alpha(img.data[i] as f32 / 255.0);

                                        pixels[idx as usize] = clr.to_color_u8().premultiply();
                                    }
                                    i += 1;
                                }
                            }
                        }
                        Content::Color => {
                            let mut i = 0;
                            for off_y in 0..img.placement.height as usize {
                                for off_x in 0..img.placement.width as usize {
                                    let idx = w.mul(y + off_y).add(x + off_x);
                                    pixels[idx as usize] = PremultipliedColorU8::from_rgba(
                                        img.data[i],
                                        img.data[i + 1],
                                        img.data[i + 2],
                                        img.data[i + 3],
                                    )
                                    .unwrap();
                                    i += 4;
                                }
                            }
                        }
                        Content::SubpixelMask => {
                            let color = text_color.to_color_u8();

                            let mut i = 0;
                            for off_y in 0..img.placement.height as usize {
                                for off_x in 0..img.placement.width as usize {
                                    let color_r = color.red() as u32;
                                    let color_g = color.green() as u32;
                                    let color_b = color.blue() as u32;
                                    let color_a = color.alpha() as u32;
                                    let mask_r = ((img.data[i] as u32) * color_a) >> 8;
                                    let mask_g = ((img.data[i + 1] as u32) * color_a) >> 8;
                                    let mask_b = ((img.data[i + 2] as u32) * color_a) >> 8;

                                    let idx = w.mul(y + off_y).add(x + off_x);
                                    let pixel = &mut pixels[idx as usize];

                                    let pixel_r = pixel.red() as u32;
                                    let pixel_g = pixel.green() as u32;
                                    let pixel_b = pixel.blue() as u32;
                                    let r = ((pixel_r * (255 - mask_r)) + (color_r * mask_r)) >> 8;
                                    let g = ((pixel_g * (255 - mask_g)) + (color_g * mask_g)) >> 8;
                                    let b = ((pixel_b * (255 - mask_b)) + (color_b * mask_b)) >> 8;

                                    *pixel = tiny_skia::PremultipliedColorU8::from_rgba(
                                        r as u8, g as u8, b as u8, 255,
                                    )
                                    .unwrap();

                                    i += 4;
                                }
                            }
                        }
                    }

                    /*
                                        if let Some(tmp) = RgbaImage::from_raw(
                                            img.placement.width,
                                            img.placement.height,
                                            img.data.clone(),
                                        ) {
                                            tmp.save(format!("glyph-{}.png", glyph_id).as_str())
                                                .unwrap();
                                            println!("Saved glyph-{}.png", glyph_id);
                                        }
                    */

                    /*
                    pixmap.fill_rect(
                        Rect::from_xywh(
                            text_flight.col as f32 * 7.0,
                            row as f32 * 12.0,
                            img.placement.width as f32,
                            img.placement.height as f32,
                        ).unwrap(),
                        &paint,
                        transform,
                        Some(&Mask::from_pixmap(
                            PixmapRef::from_bytes(
                                &img.data,
                                img.placement.width,
                                img.placement.height,
                            )
                            .unwrap(),
                            tiny_skia::MaskType::Luminance
                        )),
                    );
                    */

                    /*
                                        pixmap.draw_pixmap(
                                            text_flight.col as i32 * 7,
                                            row as i32 * 12,
                                            PixmapRef::from_bytes(&img.data, img.placement.width, img.placement.height)
                                                .unwrap(),
                                            &PixmapPaint::default(),
                                            Transform::identity(),
                                            None,
                                        );
                    */
                }

                char_col += 1;
            }

            /*let mut cluster = CharCluster::new();

            let mut character_index = -1 as i32;
            let mut parser = Parser::new(
                Script::Latin,
                text_flight.text.graphemes(true).enumerate().flat_map(
                    |(glyph_index, unicode_segment)| {
                        unicode_segment.chars().map(move |character| {
                            character_index += 1;
                            Token {
                                ch: character,
                                offset: character_index as u32,
                                len: character.len_utf8() as u8,
                                info: character.into(),
                                data: glyph_index as u32,
                            }
                        })
                    },
                ),
            );

            //let mut shaper = shape_context.builder(font.as_ref()).size(14.0).build();

            let charmap = font.as_ref().charmap();

            cluster.map(|ch| charmap.map(ch));
            //shaper.add_cluster(&cluster);
            */

            /*let mut glyph_data = Vec::new();

            shaper.shape_with(|glyph_cluster| {
                for glyph in glyph_cluster.glyphs {
                    let position = (glyph.data as f32 * glyph_width, glyph.y);
                    glyph_data.push((glyph.id, position));
                }
            });

            if glyph_data.is_empty() {
                continue;
            }*/
        }
    }
}

fn render_glyph(
    context: &mut ScaleContext,
    font: &FontRef,
    size: f32,
    glyph_id: GlyphId,
    color: Color,
) -> Option<Image> {
    if glyph_id == 0 {
        return None;
    }

    let mut scaler = context.builder(*font).size(size).hint(true).build();
    let color_u8 = color.to_color_u8();

    let img = Render::new(&[
        Source::ColorOutline(0),
        Source::ColorBitmap(StrikeWith::BestFit),
        Source::Outline,
    ])
    .default_color([
        color_u8.red(),
        color_u8.green(),
        color_u8.blue(),
        color_u8.alpha(),
    ])
    .format(Format::Alpha)
    .render(&mut scaler, glyph_id);

    if let Some(img) = img {
        if (img.placement.width != 0) && (img.placement.height != 0) {
            return Some(img);
        }
    }

    None
}
