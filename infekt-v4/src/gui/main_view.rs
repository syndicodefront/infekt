use iced::widget::{column, scrollable, text, canvas};
use iced::Element;
use iced::Length::Fill;
use iced_aw::{TabLabel, Tabs};

use crate::InfektUserAction;

use super::nfo_view_rendered::NfoViewRendered;

use iced::mouse;
use iced::{Color, Rectangle, Renderer, Theme};

#[derive(Default)]
pub struct InfektMainView {
    active_tab: TabId,
}

#[derive(Clone, PartialEq, Eq, Debug, Default, Copy)]
pub enum TabId {
    #[default]
    Rendered,
    Classic,
    TextOnly,
    FileInfo,
}

#[derive(Debug, Clone)]
pub enum InfektMainViewMessage {
    TabSelected(TabId),
}

impl InfektMainView {
    pub fn update(&mut self, message: InfektMainViewMessage) -> InfektUserAction {
        match message {
            InfektMainViewMessage::TabSelected(selected) => self.active_tab = selected,
        }

        InfektUserAction::None
    }

    pub fn view(&self) -> Element<InfektMainViewMessage> {
        // XXX: why do we have to push the contents of all tabs,
        // when the active tab is the only one that will be displayed?

        Tabs::new(InfektMainViewMessage::TabSelected)
            .push(
                TabId::Rendered,
                TabLabel::Text("Rendered".to_owned()),
                scrollable(NfoViewRendered::new(7, 12))
                //scrollable(canvas(FooBar {}).width(Fill).height(10000.0))
                    .width(Fill)
                    .height(Fill),
            )
            .push(
                TabId::Classic,
                TabLabel::Text("Classic".to_owned()),
                column![text("Classic")],
            )
            .push(
                TabId::TextOnly,
                TabLabel::Text("Text-Only".to_owned()),
                column![text("Text-Only")],
            )
            .push(
                TabId::FileInfo,
                TabLabel::Text("File Information".to_owned()),
                column![text("File Information")],
            )
            .set_active_tab(&self.active_tab)
            .tab_bar_position(iced_aw::TabBarPosition::Top)
            .into()
    }
}

/*
struct FooBar {}

impl<Message> canvas::Program<Message> for FooBar {
    type State = ();

    fn draw(
        &self,
        _state: &Self::State,
        renderer: &Renderer,
        _theme: &Theme,
        bounds: Rectangle,
        _cursor: mouse::Cursor,
    ) -> Vec<canvas::Geometry> {
        println!("FooBar::draw() - bounds: {:?}", bounds);

        let mut frame = canvas::Frame::new(renderer, bounds.size());

        // We create a `Path` representing a simple circle
        let circle = canvas::Path::circle(frame.center(), 100.0);

        // And fill it with some color
        frame.fill(&circle, Color::BLACK);

        // Then, we produce the geometry
        vec![frame.into_geometry()]
    }
}
*/
