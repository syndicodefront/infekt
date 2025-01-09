use anyhow::Result;
use pico_args::Arguments;

pub struct BundleArgs {
    pub release: bool,
    pub target: Option<String>,
}

impl BundleArgs {
    pub fn parse(args: &mut Arguments) -> Result<Self> {
        let instance = Self {
            release: args.contains("--release"),
            target: args.opt_value_from_str("--target")?,
        };

        Ok(instance)
    }
}
