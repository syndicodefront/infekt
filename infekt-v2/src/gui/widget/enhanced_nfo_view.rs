use std::sync::Arc;

use iced::advanced::layout;
use iced::advanced::renderer::{Quad, Style};
use iced::advanced::widget::tree::{self, Tree};
use iced::advanced::{self, Clipboard, Layout, Shell, Widget};
use iced::alignment;
use iced::mouse;
use iced::widget::canvas::{self, Frame, Text};
use iced::Point;
use iced::{Color, Element, Event, Length, Rectangle, Renderer, Size, Vector};

use crate::core::nfo_data::NfoData;
use crate::core::nfo_renderer_grid::{NfoRendererBlockShape, NfoRendererGrid, NfoRendererLine};
use crate::settings::NfoRenderSettings;

pub struct EnhancedNfoView<'a> {
    render_settings: Arc<NfoRenderSettings>,
    block_width_float: f32,
    block_height_float: f32,
    renderer_grid: Option<&'a NfoRendererGrid>,
}

impl<'a> EnhancedNfoView<'a> {
    pub fn new(render_settings: Arc<NfoRenderSettings>, current_nfo: &'a NfoData) -> Self {
        Self {
            renderer_grid: current_nfo.get_renderer_grid(),
            block_width_float: render_settings.enhanced_view_block_width as f32,
            block_height_float: render_settings.enhanced_view_block_height as f32,
            render_settings,
        }
    }
}

#[derive(Default)]
struct State {
    nfo_id: u64,
    cache: Vec<canvas::Cache>,
}

// Number of lines in one geometry cache entry:
const CACHE_STRIDE_LINES: usize = 100;

impl<Message, Theme> Widget<Message, Theme, Renderer> for EnhancedNfoView<'_> {
    fn tag(&self) -> tree::Tag {
        tree::Tag::of::<State>()
    }

    fn state(&self) -> tree::State {
        tree::State::new(State::default())
    }

    fn size(&self) -> Size<Length> {
        let rows = self.renderer_grid.map(|g| g.height as f32).unwrap_or(0.0);
        let columns = self.renderer_grid.map(|g| g.width as f32).unwrap_or(0.0);

        Size {
            width: Length::Fixed(columns * self.block_width_float),
            height: Length::Fixed(rows * self.block_height_float),
        }
    }

    fn layout(
        &self,
        _tree: &mut Tree,
        _renderer: &Renderer,
        limits: &layout::Limits,
    ) -> layout::Node {
        let rows = self.renderer_grid.map(|g| g.height as f32).unwrap_or(0.0);
        let columns = self.renderer_grid.map(|g| g.width as f32).unwrap_or(0.0);

        layout::atomic(
            limits,
            Length::Fixed(columns * self.block_width_float),
            rows * self.block_height_float,
        )
    }

    fn update(
        &mut self,
        tree: &mut Tree,
        _event: &Event,
        _layout: Layout<'_>,
        _cursor: mouse::Cursor,
        _renderer: &Renderer,
        _clipboard: &mut dyn Clipboard,
        shell: &mut Shell<'_, Message>,
        _viewport: &Rectangle,
    ) {
        let state = tree.state.downcast_mut::<State>();

        if let Some(grid) = self.renderer_grid {
            if grid.id != state.nfo_id {
                state.nfo_id = grid.id;
                state.cache.clear();
                state
                    .cache
                    .resize_with(grid.height / CACHE_STRIDE_LINES + 1, Default::default);
                shell.request_redraw();
            }
        }
    }

    fn draw(
        &self,
        tree: &Tree,
        renderer: &mut Renderer,
        _theme: &Theme,
        _style: &Style,
        layout: Layout<'_>,
        _cursor: mouse::Cursor,
        viewport: &Rectangle,
    ) {
        use iced::advanced::Renderer as _;

        if self.renderer_grid.is_none() {
            return;
        }

        /*println!(
            "NfoViewEnhanced::draw() - viewport: {:?} - bounds: {:?}",
            viewport,
            layout.bounds()
        );*/

        // XXX: heavily WIP !!!

        let state = tree.state.downcast_ref::<State>();
        let bounds = layout.bounds();

        if state.cache.is_empty() {
            eprintln!("NfoViewEnhanced::draw() - cache is uninitialized");

            return;
        }

        let first_visible_line =
            ((viewport.y - bounds.y) / self.block_height_float).floor() as usize;
        let last_visible_line =
            first_visible_line + (viewport.height / self.block_height_float).ceil() as usize;

        let first_cache_index = first_visible_line / CACHE_STRIDE_LINES;
        let last_cache_index = last_visible_line / CACHE_STRIDE_LINES;

        let cache_bounds = Size {
            width: self.block_width_float * self.renderer_grid.unwrap().width as f32,
            height: CACHE_STRIDE_LINES as f32 * self.block_height_float,
        };

        (first_cache_index..=last_cache_index).for_each(|cache_index| {
            let first_line = cache_index * CACHE_STRIDE_LINES;
            let last_line = (cache_index + 1) * CACHE_STRIDE_LINES - 1;

            let y_offset = first_line as f32 * self.block_height_float;

            let geometry =
                state
                    .cache
                    .get(cache_index)
                    .unwrap()
                    .draw(renderer, cache_bounds, |frame| {
                        let grid = self.renderer_grid.unwrap();

                        for line in grid.lines.iter() {
                            if line.row < first_line || line.row > last_line {
                                continue;
                            }

                            line.text_flights.iter().for_each(|flight| {
                                let x = flight.col as f32 * self.block_width_float;
                                let y = line.row as f32 * self.block_height_float - y_offset;

                                // XXX: this is bullshit
                                frame.fill_text(Text {
                                    content: flight.text.clone(),
                                    position: Point { x, y },
                                    size: iced::Pixels(self.block_height_float),
                                    color: Color::from(self.render_settings.text_color),
                                    horizontal_alignment: alignment::Horizontal::Left,
                                    vertical_alignment: alignment::Vertical::Top,
                                    line_height: advanced::text::LineHeight::Absolute(
                                        iced::Pixels(self.block_height_float),
                                    ),
                                    font: iced::Font::with_name("Cascadia Mono"), // XXX: how to take from settings?
                                    shaping: advanced::text::Shaping::Basic,
                                });
                            });
                        }

                        // XXX: pass for blur
                        // XXX: layers?

                        render_blocks(
                            &mut grid
                                .lines
                                .iter()
                                .filter(|l| l.row >= first_line && l.row <= last_line),
                            y_offset,
                            self.render_settings.enhanced_view_block_width,
                            self.render_settings.enhanced_view_block_height,
                            Color::from(self.render_settings.art_color),
                            frame,
                        );
                    });

            renderer.fill_quad(
                Quad {
                    bounds: Rectangle {
                        x: viewport.x,
                        y: viewport.y,
                        width: viewport.width,
                        height: viewport.height,
                    },
                    ..Quad::default()
                },
                Color::from(self.render_settings.background_color),
            );

            let nfo_width_float = self.block_width_float * self.renderer_grid.unwrap().width as f32;

            let bounds_translation = Vector::new(
                (bounds.x + (viewport.width - nfo_width_float) * 0.5).max(bounds.x), // center horizontally
                bounds.y + y_offset,
            );

            renderer.with_translation(bounds_translation, |renderer| {
                use iced::advanced::graphics::geometry::Renderer as _;

                renderer.draw_geometry(geometry);
            });
        });
    }
}

fn render_blocks(
    lines: &mut dyn Iterator<Item = &NfoRendererLine>,
    y_offset: f32,
    block_width: u16,
    block_height: u16,
    block_color: Color,
    frame: &mut Frame,
) {
    let block_size = Size::new(block_width as f32, block_height as f32);

    for line in lines {
        let row = line.row;
        let y = row as f32 * block_size.height - y_offset;

        for block_group in &line.block_groups {
            for (block_index, block_shape) in block_group.blocks.iter().enumerate() {
                let x = (block_group.col + block_index) as f32 * block_size.width;

                let opacity: f32 = match block_shape {
                    NfoRendererBlockShape::FullBlockLightShade => 90.0 / 255.0,
                    NfoRendererBlockShape::FullBlockMediumShade => 140.0 / 255.0,
                    NfoRendererBlockShape::FullBlockDarkShade => 190.0 / 255.0,
                    _ => 1.0,
                };

                draw_block(
                    Point::new(x, y),
                    block_size,
                    block_shape,
                    block_color.scale_alpha(opacity),
                    frame,
                );
            }
        }
    }
}

#[inline]
fn draw_block(
    top_left: Point,
    block_size: Size,
    block_shape: &NfoRendererBlockShape,
    color: Color,
    frame: &mut Frame,
) {
    let half_block_size = Size::new(block_size.width * 0.5, block_size.height * 0.5);
    let half_vertical_block_size = Size::new(half_block_size.width, block_size.height);
    let half_horizontal_block_size = Size::new(block_size.width, half_block_size.height);
    let three_quarters_block_size = Size::new(block_size.width * 0.75, block_size.height * 0.75);

    match block_shape {
        NfoRendererBlockShape::FullBlock
        | NfoRendererBlockShape::FullBlockLightShade
        | NfoRendererBlockShape::FullBlockMediumShade
        | NfoRendererBlockShape::FullBlockDarkShade => {
            frame.fill_rectangle(top_left, block_size, color);
        }
        NfoRendererBlockShape::LowerHalf => {
            frame.fill_rectangle(
                Point::new(top_left.x, top_left.y + half_horizontal_block_size.height),
                half_horizontal_block_size,
                color,
            );
        }
        NfoRendererBlockShape::UpperHalf => {
            frame.fill_rectangle(top_left, half_horizontal_block_size, color);
        }
        NfoRendererBlockShape::RightHalf => {
            frame.fill_rectangle(
                Point::new(top_left.x + half_vertical_block_size.width, top_left.y),
                half_vertical_block_size,
                color,
            );
        }
        NfoRendererBlockShape::LeftHalf => {
            frame.fill_rectangle(top_left, half_vertical_block_size, color);
        }
        NfoRendererBlockShape::BlackSquare => {
            frame.fill_rectangle(
                Point::new(
                    top_left.x + (block_size.width - three_quarters_block_size.width) * 0.5,
                    top_left.y + (block_size.height - three_quarters_block_size.height) * 0.5,
                ),
                three_quarters_block_size,
                color,
            );
        }
        NfoRendererBlockShape::BlackSquareSmall => {
            frame.fill_rectangle(
                Point::new(
                    top_left.x + block_size.width * 0.25,
                    top_left.y + block_size.height * 0.25,
                ),
                half_block_size,
                color,
            );
        }
        _ => {}
    }
}

impl<'a, Message, Theme> From<EnhancedNfoView<'a>> for Element<'a, Message, Theme, Renderer> {
    fn from(w: EnhancedNfoView<'a>) -> Self {
        Self::new(w)
    }
}
