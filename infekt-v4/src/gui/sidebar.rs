use iced::widget::{button, column, container, image, svg, text, Space};
use iced::Length::{self, Fill};
use iced::{Element, Theme};

use crate::{InfektActiveScreen, InfektUserAction};

#[derive(Default)]
pub struct InfektSidebar {
    expanded: bool,
}

#[derive(Debug, Clone)]
pub enum InfektSidebarMessage {
    ToggleSidebar,
    ShowMainView,
    ShowPreferences,
    ShowAboutScreen,
    OpenFileDialog,
}

const EXPANDED_WIDTH: Length = Length::Fixed(200.0);
const COLLAPSED_WIDTH: Length = Length::Fixed(50.0);

const LOGO_256: &[u8] = include_bytes!("../../assets/infekt-icons/iNFekt_6_256x256x32.png");
const EXPAND_ICON: &[u8] =
    include_bytes!("../../assets/tabler-icons/outline/layout-sidebar-left-expand.svg");
const COLLAPSE_ICON: &[u8] =
    include_bytes!("../../assets/tabler-icons/outline/layout-sidebar-left-collapse.svg");

impl InfektSidebar {
    pub fn update(&mut self, message: InfektSidebarMessage) -> InfektUserAction {
        match message {
            InfektSidebarMessage::ToggleSidebar => {
                self.expanded = !self.expanded;
                InfektUserAction::None
            }
            InfektSidebarMessage::ShowMainView => {
                InfektUserAction::ShowScreen(InfektActiveScreen::MainView)
            }
            InfektSidebarMessage::ShowPreferences => {
                InfektUserAction::ShowScreen(InfektActiveScreen::Preferences)
            }
            InfektSidebarMessage::ShowAboutScreen => {
                InfektUserAction::ShowScreen(InfektActiveScreen::About)
            }
            InfektSidebarMessage::OpenFileDialog => InfektUserAction::PromptOpenFile,
        }
    }

    pub fn view(&self) -> Element<InfektSidebarMessage> {
        let column = column![
            container(self.logo()).center_x(Fill).center_y(36.0),
            button("Home").on_press(InfektSidebarMessage::ShowMainView),
            button("Open...").on_press(InfektSidebarMessage::OpenFileDialog),
            button("Preferences").on_press(InfektSidebarMessage::ShowPreferences),
            button("About").on_press(InfektSidebarMessage::ShowAboutScreen),
            Space::with_height(Fill),
            button(svg(self.toggle_icon()))
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

    fn toggle_icon(&self) -> svg::Handle {
        svg::Handle::from_memory(if self.expanded {
            COLLAPSE_ICON
        } else {
            EXPAND_ICON
        })
    }

    fn logo(&self) -> Element<'static, InfektSidebarMessage> {
        if self.expanded {
            text("iNFekt").size(26).into()
        } else {
            image(image::Handle::from_bytes(LOGO_256))
                .height(24.0)
                .into()
        }
    }
}
