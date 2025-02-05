use iced::widget::{
    column, container, iced as iced_logo, image, rich_text, row, span, text, Space,
};
use iced::Length::Fill;
use iced::{system, Element, Task};

use crate::app::Action;

#[derive(Default)]
pub struct InfektAboutScreen {
    sysinfo: Option<system::Information>,
}

#[derive(Debug, Clone)]
pub enum Message {
    FetchInformation(Box<system::Information>),
    OpenInfektWebsite(u64),
}

const LOGO_256: &[u8] = include_bytes!("../../assets/infekt-icons/iNFekt_6_256x256x32.png");
// const LOGO_SVG: &[u8] = include_bytes!("../../assets/infekt-icons/logo_v2.svg");

impl InfektAboutScreen {
    pub fn update(&mut self, message: Message) -> Action {
        match message {
            Message::FetchInformation(sysinfo) => {
                self.sysinfo = Some(*sysinfo);
            }
            Message::OpenInfektWebsite(_) => {}
        }

        Action::None
    }

    pub fn on_before_shown(&mut self) -> Option<Task<Message>> {
        if self.sysinfo.is_none() {
            Some(system::fetch_information().map(|info| Message::FetchInformation(Box::new(info))))
        } else {
            None
        }
    }

    pub fn view(&self) -> Element<Message> {
        let padding = 15;
        let left = column![container(self.logo()),].padding(padding);

        let right = column![
            text("iNFekt NFO Viewer").size(32),
            rich_text![
                span(format!("Version {}\n", env!("CARGO_PKG_VERSION"))),
                span("© syndicode.org 2010-2025\n"),
                span("Project Homepage: "),
                span("infekt.ws").underline(true).link(0u64),
                span("\n\n"),
                span("This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation.")
            ]
            .on_link_click(Message::OpenInfektWebsite)
            .font(iced::Font::DEFAULT),
            Space::with_height(Fill),
            match self.sysinfo {
                Some(ref sysinfo) => {
                    rich_text![
                        span(sysinfo_line("Operating System", &sysinfo.system_version)),
                        span(format!("Graphics Backend: {}\n", sysinfo.graphics_backend)),
                    ]
                    .on_link_click(Message::OpenInfektWebsite)
                    .font(iced::Font::DEFAULT)
                }
                None => rich_text![],
            },
            Space::with_height(Fill),
            text("GUI built with"),
            iced_logo(28)
        ]
        .padding(padding)
        .width(Fill)
        .height(Fill);

        row![left, right].padding(padding).into()
    }

    fn logo(&self) -> Element<'static, Message> {
        image(image::Handle::from_bytes(LOGO_256))
            .height(64.0)
            .into()
    }
}

fn sysinfo_line(label: &str, value: &Option<String>) -> String {
    match value {
        Some(value) => {
            format!("{}: {}\n", label, value)
        }
        None => {
            format!("{}: <unknown>\n", label)
        }
    }
}
