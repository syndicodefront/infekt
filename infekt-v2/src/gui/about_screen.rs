use iced::widget::{
    column, container, iced as iced_logo, image, rich_text, row, span, text, space,
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
    HyperlinkClicked(LinkTarget),
}

#[derive(Debug, Clone)]
pub enum LinkTarget {
    InfektWebsite,
}

const LOGO_256: &[u8] = include_bytes!("../../assets/infekt-icons/iNFekt_6_256x256x32.png");

impl InfektAboutScreen {
    pub fn update(&mut self, message: Message) -> Action {
        match message {
            Message::FetchInformation(sysinfo) => {
                self.sysinfo = Some(*sysinfo);
            }
            Message::HyperlinkClicked(_) => {
                open::that_detached("https://infekt.ws").unwrap();
            }
        }

        Action::None
    }

    pub fn on_before_shown(&mut self) -> Option<Task<Message>> {
        if self.sysinfo.is_none() {
            Some(system::information().map(|info| Message::FetchInformation(Box::new(info))))
        } else {
            None
        }
    }

    pub fn view(&self) -> Element<'_, Message> {
        let padding = 15;
        let left = column![container(self.logo()),].padding(padding);

        let right = column![
            text("iNFekt NFO Viewer").size(32),
            rich_text![
                span(format!("Version {}\n", env!("CARGO_PKG_VERSION"))),
                span("© syndicode.org 2010-2025\n"),
                span("Project Homepage: "),
                span("infekt.ws").underline(true).link(LinkTarget::InfektWebsite),
                span("\n\n"),
                span("This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation.")
            ]
            .on_link_click(Message::HyperlinkClicked)
            .font(iced::Font::DEFAULT),
            space::vertical(),
            match self.sysinfo {
                Some(ref sysinfo) => {
                    rich_text![
                        span(sysinfo_line("Operating System", &sysinfo.system_version)),
                        span(format!("Graphics Backend: {}\n", sysinfo.graphics_backend)),
                    ]
                    .on_link_click(Message::HyperlinkClicked)
                    .font(iced::Font::DEFAULT)
                }
                None => rich_text![],
            },
            space::vertical(),
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
            format!("{label}: {value}\n")
        }
        None => {
            format!("{label}: <unknown>\n")
        }
    }
}
