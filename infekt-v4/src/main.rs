mod app;
mod core;
mod gui;

use app::InfektApp;
use iced::window::{self, icon};

pub fn main() -> iced::Result {
    iced::application(InfektApp::title, InfektApp::update, InfektApp::view)
        .window(initial_window_settings())
        .antialiasing(true)
        .centered()
        .window_size(iced::Size::new(850.0, 700.0))
        .theme(|_state| iced::Theme::Dark) // somewhat tolearable, will be replaced with theme that matches the NFO
        .run_with(InfektApp::new)
}

fn initial_window_settings() -> window::Settings {
    window::Settings {
        icon: window_settings_application_icon(),
        min_size: Some(iced::Size::new(600.0, 450.0)),
        ..iced::window::Settings::default()
    }
}

fn window_settings_application_icon() -> Option<icon::Icon> {
    let bytes = std::io::Cursor::new(include_bytes!("../assets/infekt-icons/iNFekt.ico"));
    let icon_dir = ico::IconDir::read(bytes).unwrap();

    let idx = icon_dir.entries().iter().find(|e| e.width() == 48);

    if let Some(entry) = idx {
        let img = entry.decode().unwrap().rgba_data().to_vec();
        let icon = iced::window::icon::from_rgba(img, entry.width(), entry.height()).unwrap();

        Some(icon)
    } else {
        eprintln!("Could not find 48x48 icon in iNFekt.ico");

        None
    }
}
