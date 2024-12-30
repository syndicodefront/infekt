mod file_operations;
mod utils;
mod view;

use iced::Task;
use std::path::PathBuf;

use crate::core::nfo_data::NfoData;
use crate::gui::about_screen::{InfektAboutScreen, InfektAboutScreenMessage};
use crate::gui::main_view::{InfektMainView, InfektMainViewMessage};
use crate::gui::sidebar::{InfektSidebar, InfektSidebarMessage};

#[derive(Debug, Clone)]
pub(crate) enum Message {
    NoOp,
    FontLoaded(Result<(), iced::font::Error>),
    SidebarMessage(InfektSidebarMessage),
    MainViewMessage(InfektMainViewMessage),
    AboutScreenMessage(InfektAboutScreenMessage),
    OpenFile(Option<PathBuf>),
}

#[derive(Debug, Clone)]
pub(crate) enum InfektUserAction {
    None,
    ShowScreen(InfektActiveScreen),
    SelectFileForOpening,
    ShowErrorMessage(String),
}

#[derive(Debug, Clone, Default)]
pub(crate) enum InfektActiveScreen {
    #[default]
    MainView,
    Preferences,
    About,
}

#[derive(Default)]
pub(crate) struct InfektApp {
    active_screen: InfektActiveScreen,
    sidebar: InfektSidebar,
    main_view: InfektMainView,
    about_screen: InfektAboutScreen,
    current_nfo_data: NfoData,
    current_nfo_path: Option<PathBuf>,
}

impl InfektApp {
    pub fn new() -> (Self, Task<Message>) {
        let app = Self::default();
        let load_font = |data: &'static [u8]| iced::font::load(data).map(Message::FontLoaded);

        let task = Task::batch(vec![
            load_font(include_bytes!("../assets/fonts/ServerMono-Regular.otf")),
            load_font(include_bytes!(
                "../assets/fonts/Menlo-Regular-NormalMono.ttf"
            )),
        ]);

        (app, task)
    }

    pub fn title(&self) -> String {
        if let Some(file) = &self.current_nfo_path {
            format!(
                "iNFekt NFO Viewer - {}",
                file.file_name().unwrap_or_default().to_string_lossy()
            )
        } else {
            "iNFekt NFO Viewer".to_string()
        }
    }

    pub fn update(&mut self, message: Message) -> Task<Message> {
        let mut task = Task::none();

        let action = match message {
            Message::NoOp => InfektUserAction::None,
            Message::FontLoaded(_) => InfektUserAction::None,

            Message::SidebarMessage(message) => self.sidebar.update(message),
            Message::MainViewMessage(message) => self.main_view.update(message),
            Message::AboutScreenMessage(message) => self.about_screen.update(message),

            Message::OpenFile(file) => self.action_load_new_nfo(file),
        };

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
            InfektUserAction::SelectFileForOpening => {
                task = self.task_open_nfo_file_dialog();
            }
            InfektUserAction::ShowErrorMessage(message) => {
                task = self.task_show_error_message(message);
            }
        }

        task
    }
}
