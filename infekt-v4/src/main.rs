use std::path::{Path, PathBuf};

use iced::widget::{column, row, text};
use iced::Length::Fill;
use iced::{Alignment, Element, Size, Task};
use rfd::AsyncFileDialog;

mod core;
mod gui;

use crate::gui::sidebar::InfektSidebar;
use gui::sidebar::InfektSidebarMessage;

use crate::gui::main_view::InfektMainView;
use gui::main_view::InfektMainViewMessage;

use crate::gui::about_screen::InfektAboutScreen;
use gui::about_screen::InfektAboutScreenMessage;

pub fn main() -> iced::Result {
    iced::application("iNFekt NFO Viewer", InfektApp::update, InfektApp::view)
        .antialiasing(true)
        .centered()
        .window_size(Size::new(850.0, 700.0))
        .theme(|_state| iced::Theme::Dark) // somewhat tolearable, will be replaced with theme that matches the NFO
        .run()
}

#[derive(Default)]
struct InfektApp {
    active_screen: InfektActiveScreen,
    sidebar: InfektSidebar,
    main_view: InfektMainView,
    about_screen: InfektAboutScreen,
}

#[derive(Debug, Clone)]
enum Message {
    SidebarMessage(InfektSidebarMessage),
    MainViewMessage(InfektMainViewMessage),
    AboutScreenMessage(InfektAboutScreenMessage),
    OpenFile(Option<PathBuf>),
}

#[derive(Debug, Clone)]
enum InfektUserAction {
    None,
    ShowScreen(InfektActiveScreen),
    PromptOpenFile,
}

#[derive(Debug, Clone, Default)]
enum InfektActiveScreen {
    #[default]
    MainView,
    Preferences,
    About,
}

impl InfektApp {
    fn update(&mut self, message: Message) -> Task<Message> {
        let action = match message {
            Message::SidebarMessage(message) => self.sidebar.update(message),
            Message::MainViewMessage(message) => self.main_view.update(message),
            Message::AboutScreenMessage(message) => self.about_screen.update(message),
            Message::OpenFile(file) => {
                if let Some(file) = file {
                    println!("Opening file: {:?}", file);
                }

                InfektUserAction::None
            }
        };

        let mut task = Task::none();

        match action {
            InfektUserAction::None => {}
            InfektUserAction::ShowScreen(screen) => {
                self.active_screen = screen;

                match self.active_screen {
                    InfektActiveScreen::MainView => {}
                    InfektActiveScreen::Preferences => {}
                    InfektActiveScreen::About => {
                        let result = self.about_screen.on_before_shown();

                        if let Some(new_task) = result {
                            task = new_task.map(Message::AboutScreenMessage);
                        }
                    }
                }
            }
            InfektUserAction::PromptOpenFile => {
                task = Task::perform(
                    async {
                        let file = AsyncFileDialog::new()
                            .add_filter("Block Art Files", &["nfo", "diz", "asc", "txt"])
                            .pick_file()
                            .await;

                        if let Some(file) = file {
                            Some(file.path().to_path_buf())
                        } else {
                            None
                        }
                    },
                    |f| Message::OpenFile(f),
                )
            }
        }

        task
    }

    fn view(&self) -> Element<Message> {
        let sidebar = self.sidebar.view().map(Message::SidebarMessage);
        let content = column![]
            .push(match self.active_screen {
                InfektActiveScreen::MainView => self.main_view.view().map(Message::MainViewMessage),
                InfektActiveScreen::Preferences => text("Preferences").into(),
                InfektActiveScreen::About => {
                    self.about_screen.view().map(Message::AboutScreenMessage)
                }
            })
            .height(Fill)
            .width(Fill);

        row![sidebar, content]
            .padding(0)
            .align_y(Alignment::Start)
            .into()
    }
}
