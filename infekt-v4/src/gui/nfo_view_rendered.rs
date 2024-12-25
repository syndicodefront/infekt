
use iced::advanced::layout::{self, Layout};
use iced::advanced::renderer;
use iced::advanced::widget::{self, Widget};
use iced::border;
use iced::mouse;
use iced::{Color, Element, Length, Rectangle, Size};

pub struct NfoViewRendered {
    radius: f32,
}

impl NfoViewRendered {
    pub fn new(radius: f32) -> Self {
        Self { radius }
    }
}

pub fn nfo_view_rendered(radius: f32) -> NfoViewRendered {
    NfoViewRendered::new(radius)
}

impl<Message, Theme, Renderer> Widget<Message, Theme, Renderer> for NfoViewRendered
where
    Renderer: renderer::Renderer,
{
    fn size(&self) -> Size<Length> {
        Size {
            width: Length::Shrink,
            height: Length::Shrink,
        }
    }

    fn layout(
        &self,
        _tree: &mut widget::Tree,
        _renderer: &Renderer,
        _limits: &layout::Limits,
    ) -> layout::Node {
        layout::Node::new(Size::new(self.radius * 2.0, 15000.0))
    }

    fn draw(
        &self,
        _state: &widget::Tree,
        renderer: &mut Renderer,
        _theme: &Theme,
        _style: &renderer::Style,
        layout: Layout<'_>,
        _cursor: mouse::Cursor,
        viewport: &Rectangle,
    ) {
        println!("NfoViewRendered::draw() - viewport: {:?} - bounds: {:?}", viewport, layout.bounds());
        renderer.fill_quad(
            renderer::Quad {
                bounds: Rectangle {
                    x: layout.bounds().x,
                    y: layout.bounds().y,
                    width: self.radius * 2.0,
                    height: self.radius * 2.0,
                },
                border: border::rounded(self.radius),
                ..renderer::Quad::default()
            },
            Color::BLACK,
        );
    }
}

impl<Message, Theme, Renderer> From<NfoViewRendered>
    for Element<'_, Message, Theme, Renderer>
where
    Renderer: renderer::Renderer,
{
    fn from(circle: NfoViewRendered) -> Self {
        Self::new(circle)
    }
}
