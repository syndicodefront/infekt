use iced::advanced::layout;
use iced::advanced::renderer::{Quad, Style};
use iced::advanced::widget::tree::{self, Tree};
use iced::advanced::{self, Clipboard, Layout, Shell, Widget};
use iced::alignment;
use iced::{border, Point};
use iced::event;
use iced::mouse;
use iced::widget::canvas::{self, Text};
use iced::window::{self, RedrawRequest};
use iced::{Background, Color, Element, Event, Length, Radians, Rectangle, Renderer, Size, Vector};

pub struct NfoViewRendered {
    block_width: u32,
    block_height: u32,
}

impl NfoViewRendered {
    pub fn new(block_width: u32, block_height: u32) -> Self {
        Self {
            block_width,
            block_height,
        }
    }
}

#[derive(Default)]
struct State {
    cache: canvas::Cache,
}

impl<Message, Theme> Widget<Message, Theme, Renderer> for NfoViewRendered {
    fn tag(&self) -> tree::Tag {
        tree::Tag::of::<State>()
    }

    fn state(&self) -> tree::State {
        tree::State::new(State::default())
    }

    fn size(&self) -> Size<Length> {
        Size {
            width: Length::Fixed(80.0 * self.block_width as f32),
            height: Length::Fixed(10000.0 * self.block_height as f32),
        }
    }

    fn layout(
        &self,
        _tree: &mut Tree,
        _renderer: &Renderer,
        limits: &layout::Limits,
    ) -> layout::Node {
        layout::atomic(
            limits,
            80.0 * self.block_width as f32,
            10000.0 * self.block_height as f32,
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
        shell: &mut Shell<'_, Message>,
        _viewport: &Rectangle,
    ) -> event::Status {
        let _state = tree.state.downcast_mut::<State>();

        if let Event::Window(window::Event::RedrawRequested(_now)) = event {
            //state.cache.clear();
            //shell.request_redraw(RedrawRequest::NextFrame);
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
                line_height: advanced::text::LineHeight::Absolute(iced::Pixels(self.block_height as f32)),
                font: iced::Font::MONOSPACE,
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

impl<Message, Theme> From<NfoViewRendered> for Element<'_, Message, Theme, Renderer> {
    fn from(circle: NfoViewRendered) -> Self {
        Self::new(circle)
    }
}
