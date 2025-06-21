mod file_operations;
mod theme;
mod utils;
mod view;

use iced::{Task, Theme};
use std::path::PathBuf;
use std::sync::Arc;

use crate::core::nfo_data::NfoData;
use crate::gui::about_screen::{self, InfektAboutScreen};
use crate::gui::main_view::{self, InfektMainView};
use crate::gui::preferences::{self, InfektPreferencesScreen};
use crate::gui::sidebar::{self, InfektSidebar};
use crate::settings::NfoRenderSettings;

#[derive(Debug, Clone)]
#[allow(clippy::enum_variant_names)]
pub(crate) enum Message {
    NoOp,
    MainWindowCreated(Option<iced::window::Id>),
    SidebarMessage(sidebar::Message),
    MainViewMessage(main_view::Message),
    PreferencesScreenMessage(preferences::Message),
    AboutScreenMessage(about_screen::Message),
    OpenFile(Option<PathBuf>),
    RenderSettingsChanged(Arc<NfoRenderSettings>),
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
    main_window_id: Option<iced::window::Id>,

    active_screen: ActiveScreen,
    sidebar: InfektSidebar,
    main_view: InfektMainView,
    preferences_screen: InfektPreferencesScreen,
    about_screen: InfektAboutScreen,

    theme: Theme,
    active_render_settings: Arc<NfoRenderSettings>,
    current_nfo: NfoData,
}

impl InfektApp {
    pub fn new() -> (Self, Task<Message>) {
        let app = Self {
            theme: Theme::Dark,
            ..Self::default()
        };

        let task = Task::batch(vec![
            iced::window::get_oldest().map(Message::MainWindowCreated),
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
            Message::NoOp => Action::None,

            Message::MainWindowCreated(window_id) => {
                self.main_window_id = window_id;
                // self.theme = theme::create_theme(self.active_render_settings.clone());

                Action::None
            }

            Message::SidebarMessage(message) => self.sidebar.update(message),
            Message::MainViewMessage(message) => self.main_view.update(message),
            Message::PreferencesScreenMessage(message) => self.preferences_screen.update(message),
            Message::AboutScreenMessage(message) => self.about_screen.update(message),

            Message::OpenFile(file) => self.action_load_new_nfo(file),

            Message::RenderSettingsChanged(settings) => {
                self.active_render_settings = settings;

                // self.theme = theme::create_theme(self.active_render_settings.clone());

                // XXX: improve?
                self.main_view
                    .update(main_view::Message::RenderSettingsChanged(
                        self.active_render_settings.clone(),
                    ))
            }
        };

        match action {
            Action::None => {}
            Action::ShowScreen(screen) => {
                self.active_screen = screen;

                match self.active_screen {
                    ActiveScreen::MainView => {}
                    ActiveScreen::Preferences => {
                        let result = self.preferences_screen.on_before_shown(self.active_render_settings.clone());

                        if let Some(new_task) = result {
                            task = new_task.map(Message::PreferencesScreenMessage);
                        }
                    }
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
                task = self.show_error_message_popup(message);
            }
        }

        task
    }

    /*pub fn theme(&self) -> Theme {
        self.theme.clone()
    }*/
}
