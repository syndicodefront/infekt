use iced::advanced::layout;
use iced::advanced::renderer::Style;
use iced::advanced::widget::tree::{self, Tree};
use iced::advanced::{self, Clipboard, Layout, Shell, Widget};
use iced::alignment;
use iced::event;
use iced::mouse;
use iced::widget::canvas::{self, Text};
use iced::window::{self};
use iced::Point;
use iced::{Color, Element, Event, Length, Rectangle, Renderer, Size, Vector};

use crate::core::nfo_data::NfoData;
use crate::core::nfo_renderer_grid::NfoRendererGrid;

pub struct NfoViewRendered<'a> {
    block_width: u32,
    block_height: u32,
    renderer_grid: Option<&'a NfoRendererGrid>,
}

impl<'a> NfoViewRendered<'a> {
    pub fn new(block_width: u32, block_height: u32, current_nfo: &'a NfoData) -> Self {
        Self {
            block_width,
            block_height,
            renderer_grid: current_nfo.get_renderer_grid(),
        }
    }
}

#[derive(Default)]
struct State {
    nfo_id: u64,
    cache: canvas::Cache,
}

impl<Message, Theme> Widget<Message, Theme, Renderer> for NfoViewRendered<'_> {
    fn tag(&self) -> tree::Tag {
        tree::Tag::of::<State>()
    }

    fn state(&self) -> tree::State {
        tree::State::new(State::default())
    }

    fn size(&self) -> Size<Length> {
        let columns = self.renderer_grid.map(|g| g.width).unwrap_or(0);
        let rows = self.renderer_grid.map(|g| g.height).unwrap_or(0);

        Size {
            width: Length::Fixed(columns as f32 * self.block_width as f32),
            height: Length::Fixed(rows as f32 * self.block_height as f32),
        }
    }

    fn layout(
        &self,
        _tree: &mut Tree,
        _renderer: &Renderer,
        limits: &layout::Limits,
    ) -> layout::Node {
        let columns = self.renderer_grid.map(|g| g.width).unwrap_or(0);
        let rows = self.renderer_grid.map(|g| g.height).unwrap_or(0);

        layout::atomic(
            limits,
            columns as f32 * self.block_width as f32,
            rows as f32 * self.block_height as f32,
        )
    }

    fn on_event(
        &mut self,
        tree: &mut Tree,
        event: Event,
        _layout: Layout<'_>,
        _cursor: mouse::Cursor,
        _renderer: &Renderer,
        _clipboard: &mut dyn Clipboard,
        _shell: &mut Shell<'_, Message>,
        _viewport: &Rectangle,
    ) -> event::Status {
        let state = tree.state.downcast_mut::<State>();

        if let Event::Window(window::Event::RedrawRequested(_now)) = event {
            //state.cache.clear();
            //shell.request_redraw(RedrawRequest::NextFrame);
        }

        if self
            .renderer_grid
            .is_some_and(|grid| grid.id != state.nfo_id)
        {
            state.cache.clear();
            state.nfo_id = self.renderer_grid.unwrap().id;

            println!("NfoViewRendered::on_event() - cache cleared");
        }

        event::Status::Ignored
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

        println!(
            "NfoViewRendered::draw() - viewport: {:?} - bounds: {:?}",
            viewport,
            layout.bounds()
        );

        let state = tree.state.downcast_ref::<State>();
        let bounds = layout.bounds();

        let geometry = state.cache.draw(renderer, bounds.size(), |frame| {
            // We create a `Path` representing a simple circle
            let circle = canvas::Path::circle(frame.center(), 100.0);

            println!("re-rendered");

            frame.fill_text(Text {
                content: "Hello, world!".to_owned(),
                position: Point {
                    x: 100.0,
                    y: 1000.0,
                },
                size: iced::Pixels(20.0),
                color: Color::WHITE,
                horizontal_alignment: alignment::Horizontal::Left,
                vertical_alignment: alignment::Vertical::Top,
                line_height: advanced::text::LineHeight::Absolute(iced::Pixels(
                    self.block_height as f32,
                )),
                font: iced::Font::with_name("Server Mono"),
                shaping: advanced::text::Shaping::Basic,
            });

            // And fill it with some color
            frame.fill(&circle, Color::from_rgba8(200, 0, 0, 1.0));
        });

        renderer.with_translation(Vector::new(bounds.x, bounds.y), |renderer| {
            use iced::advanced::graphics::geometry::Renderer as _;

            renderer.draw_geometry(geometry);
        });

        /*
        renderer.fill_quad(
            Quad {
                bounds: Rectangle {
                    x: layout.bounds().x,
                    y: layout.bounds().y,
                    width: 200.0,
                    height: 200.0,
                },
                border: border::rounded(200.0),
                ..Quad::default()
            },
            Color::WHITE,
        );
        */
    }
}

impl<'a, Message, Theme> From<NfoViewRendered<'a>> for Element<'a, Message, Theme, Renderer> {
    fn from(w: NfoViewRendered<'a>) -> Self {
        Self::new(w)
    }
}
