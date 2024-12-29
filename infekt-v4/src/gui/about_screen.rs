use iced::widget::{
    column, container, iced as iced_logo, image, rich_text, row, span, text, Space,
};
use iced::Length::Fill;
use iced::{system, Element, Task};

use crate::app::InfektUserAction;

#[derive(Default)]
pub struct InfektAboutScreen {
    sysinfo: Option<system::Information>,
}

#[derive(Debug, Clone)]
pub enum InfektAboutScreenMessage {
    _Dummy,
    FetchInformation(system::Information),
}

const LOGO_256: &[u8] = include_bytes!("../../assets/infekt-icons/iNFekt_6_256x256x32.png");
// const LOGO_SVG: &[u8] = include_bytes!("../../assets/infekt-icons/logo_v2.svg");

impl InfektAboutScreen {
    pub fn update(&mut self, message: InfektAboutScreenMessage) -> InfektUserAction {
        if let InfektAboutScreenMessage::FetchInformation(sysinfo) = message {
            self.sysinfo = Some(sysinfo);
        }

        InfektUserAction::None
    }

    pub fn on_before_shown(&mut self) -> Option<Task<InfektAboutScreenMessage>> {
        if self.sysinfo.is_none() {
            Some(system::fetch_information().map(InfektAboutScreenMessage::FetchInformation))
        } else {
            None
        }
    }

    pub fn view(&self) -> Element<InfektAboutScreenMessage> {
        let padding = 15;
        let left = column![container(self.logo()),].padding(padding);

        let right = column![
            text("iNFekt NFO Viewer").size(32),
            rich_text([
                span(format!("Version {}\n", env!("CARGO_PKG_VERSION"))),
                span("© syndicode.org 2010-2025\n"),
                span("Project Homepage: "),
                span("infekt.ws").underline(true), // XXX: does not work: .link("https://infekt.ws"),
                span("\n\n"),
                span("This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation.")
            ]),
            Space::with_height(Fill),
            match self.sysinfo {
                Some(ref sysinfo) => {
                    rich_text([
                        span(sysinfo_line("Operating System", &sysinfo.system_version)),
                        span(format!("Graphics Backend: {}\n", sysinfo.graphics_backend)),
                    ])
                }
                None => rich_text([]),
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

    fn logo(&self) -> Element<'static, InfektAboutScreenMessage> {
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
