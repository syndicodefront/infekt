use iced::widget::{button, column, container, image, text, Space};
use iced::Length::{self, Fill};
use iced::{Alignment, Element, Theme};

use crate::InfektUserAction;

#[derive(Default)]
pub struct InfektSidebar {
    expanded: bool,
}

#[derive(Debug, Clone, Copy)]
pub enum InfektSidebarMessage {
    ToggleSidebar,
    Increment,
    Decrement,
}

const EXPANDED_WIDTH: Length = Length::Fixed(200.0);
const COLLAPSED_WIDTH: Length = Length::Fixed(50.0);

impl InfektSidebar {
    pub fn update(&mut self, message: InfektSidebarMessage) -> InfektUserAction {
        match message {
            InfektSidebarMessage::ToggleSidebar => {
                self.expanded = !self.expanded;
                InfektUserAction::None
            }
            InfektSidebarMessage::Increment => InfektUserAction::Increment,
            InfektSidebarMessage::Decrement => InfektUserAction::Decrement,
        }
    }

    pub fn view(&self) -> Element<InfektSidebarMessage> {
        let column = column![
            container(logo(self.expanded)).center_x(Fill).center_y(36.0),
            button("Increment").on_press(InfektSidebarMessage::Increment),
            button("Decrement").on_press(InfektSidebarMessage::Decrement),
            Space::with_height(Fill),
            button(if self.expanded { "<<" } else { ">>" })
                .width(Fill)
                .on_press(InfektSidebarMessage::ToggleSidebar),
        ]
        .spacing(20);

        container(column)
            .style(|theme: &Theme| {
                let palette = theme.extended_palette();

                container::Style::default().background(palette.background.strong.color)
            })
            .height(Fill)
            .width(self.current_width())
            .into()
    }

    fn current_width(&self) -> Length {
        if self.expanded {
            EXPANDED_WIDTH
        } else {
            COLLAPSED_WIDTH
        }
    }
}

fn logo(expanded: bool) -> Element<'static, InfektSidebarMessage> {
    if expanded {
        text("iNFekt")
        .size(24)
        .into()
    } else {
        image("assets/infekt-icons/iNFekt_6_256x256x32.png")
        .height(24.0)
        .into()
    }
}
