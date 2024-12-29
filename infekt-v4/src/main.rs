mod app;
mod core;
mod gui;

use app::InfektApp;

pub fn main() -> iced::Result {
    iced::application(InfektApp::title, InfektApp::update, InfektApp::view)
        .antialiasing(true)
        .centered()
        .window_size(iced::Size::new(850.0, 700.0))
        .theme(|_state| iced::Theme::Dark) // somewhat tolearable, will be replaced with theme that matches the NFO
        .run()
}
