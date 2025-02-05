mod app;
mod core;
mod gui;
mod settings;

use app::InfektApp;
use iced::window::{self, icon};

pub fn main() -> iced::Result {
    iced::application(InfektApp::title, InfektApp::update, InfektApp::view)
        .window(initial_window_settings())
        .settings(iced::Settings {
            antialiasing: true,
            fonts: vec![
                include_bytes!("../assets/fonts/CascadiaMono.ttf").into(), // font name: Cascadia Mono
                include_bytes!("../assets/fonts/Andale Mono.ttf").into(),  // font name: Andale Mono
                include_bytes!(
                    "../assets/fonts/Menlo-Regular-NormalMono.ttf" // font name: Menlo Nerd Font Mono
                ).into(),
                include_bytes!("../assets/fonts/FiraMono-Regular.ttf").into(), // font name: Fira Mono
            ],
            ..iced::Settings::default()
        })
        .scale_factor(|_| 1.0) // https://github.com/iced-rs/iced/issues/2657#issuecomment-2566536858
        .window_size(iced::Size::new(850.0, 700.0))
        .centered()
        .theme(InfektApp::theme)
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
