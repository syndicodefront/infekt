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
#[allow(clippy::enum_variant_names)]
pub(crate) enum Message {
    FontLoaded(Result<(), iced::font::Error>),
    SidebarMessage(InfektSidebarMessage),
    MainViewMessage(InfektMainViewMessage),
    AboutScreenMessage(InfektAboutScreenMessage),
    OpenFile(Option<PathBuf>),
}

#[derive(Debug, Clone)]
pub(crate) enum Action {
    None,
    ShowScreen(ActiveScreen),
    SelectFileForOpening,
    ShowErrorMessage(String),
}

#[derive(Debug, Clone, Default)]
pub(crate) enum ActiveScreen {
    #[default]
    MainView,
    Preferences,
    About,
}

#[derive(Default)]
pub(crate) struct InfektApp {
    active_screen: ActiveScreen,
    sidebar: InfektSidebar,
    main_view: InfektMainView,
    about_screen: InfektAboutScreen,
    current_nfo: NfoData,
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
        if self.current_nfo.is_loaded() {
            format!(
                "iNFekt NFO Viewer - {}",
                self.current_nfo.get_file_name().unwrap_or_default()
            )
        } else {
            "iNFekt NFO Viewer".to_string()
        }
    }

    pub fn update(&mut self, message: Message) -> Task<Message> {
        let mut task = Task::none();

        let action = match message {
            // Message::NoOp => Action::None,
            Message::FontLoaded(_) => Action::None,

            Message::SidebarMessage(message) => self.sidebar.update(message),
            Message::MainViewMessage(message) => self.main_view.update(message),
            Message::AboutScreenMessage(message) => self.about_screen.update(message),

            Message::OpenFile(file) => self.action_load_new_nfo(file),
        };

        match action {
            Action::None => {}
            Action::ShowScreen(screen) => {
                self.active_screen = screen;

                match self.active_screen {
                    ActiveScreen::MainView => {}
                    ActiveScreen::Preferences => {}
                    ActiveScreen::About => {
                        let result = self.about_screen.on_before_shown();

                        if let Some(new_task) = result {
                            task = new_task.map(Message::AboutScreenMessage);
                        }
                    }
                }
            }
            Action::SelectFileForOpening => {
                task = self.task_open_nfo_file_dialog();
            }
            Action::ShowErrorMessage(message) => {
                self.show_error_message_popup(message);
            }
        }

        task
    }
}
