use colornames::Color;

pub fn from_palette_rgb(color: palette::rgb::Rgb) -> Option<Color> {
    let (r, g, b) = color.into_components();
    let r = (r * 255.0) as u8;
    let g = (g * 255.0) as u8;
    let b = (b * 255.0) as u8;
    let hex = format!("#{:02X}{:02X}{:02X}", r, g, b);
    Color::convert_str(hex.as_str())
}

pub fn from_palette_rgba(color: palette::rgb::Rgba) -> Option<Color> {
    // XXX: later maybe don't ignore alpha?
    let (r, g, b, _) = color.into_components();
    let r = (r * 255.0) as u8;
    let g = (g * 255.0) as u8;
    let b = (b * 255.0) as u8;
    let hex = format!("#{:02X}{:02X}{:02X}", r, g, b);
    Color::convert_str(hex.as_str())
}

pub fn to_palette_rgb(color: Color) -> palette::rgb::Rgb {
    palette::rgb::Srgb::from_components(color.rgb()).into()
}

pub fn to_palette_rgba(color: Color) -> palette::rgb::Rgba {
    // XXX: later maybe don't ignore alpha?
    palette::rgb::Srgba::from_components((color.rgb().0, color.rgb().1, color.rgb().2, 255)).into()
}

pub static ALL: &[Color] = &[
    Color::Black,
    Color::BlackBlue,
    Color::Night,
    Color::Charcoal,
    Color::Oil,
    Color::StormyGray,
    Color::LightBlack,
    Color::DarkSteampunk,
    Color::BlackCat,
    Color::Iridium,
    Color::BlackEel,
    Color::BlackCow,
    Color::GrayWolf,
    Color::VampireGray,
    Color::IronGray,
    Color::GrayDolphin,
    Color::CarbonGray,
    Color::AshGray,
    Color::DimGray,
    Color::NardoGray,
    Color::CloudyGray,
    Color::SmokeyGray,
    Color::AlienGray,
    Color::SonicSilver,
    Color::PlatinumGray,
    Color::Granite,
    Color::Gray,
    Color::BattleshipGray,
    Color::SheetMetal,
    Color::DarkGainsboro,
    Color::GunmetalGray,
    Color::ColdMetal,
    Color::StainlessSteelGray,
    Color::DarkGray,
    Color::ChromeAluminum,
    Color::GrayCloud,
    Color::Metal,
    Color::Silver,
    Color::Steampunk,
    Color::PaleSilver,
    Color::GearSteelGray,
    Color::GrayGoose,
    Color::PlatinumSilver,
    Color::LightGray,
    Color::SilverWhite,
    Color::Gainsboro,
    Color::LightSteelGray,
    Color::WhiteSmoke,
    Color::WhiteGray,
    Color::Platinum,
    Color::MetallicSilver,
    Color::BlueGray,
    Color::RomanSilver,
    Color::LightSlateGray,
    Color::SlateGray,
    Color::RatGray,
    Color::SlateGraniteGray,
    Color::JetGray,
    Color::MistBlue,
    Color::SteelGray,
    Color::MarbleBlue,
    Color::SlateBlueGray,
    Color::LightPurpleBlue,
    Color::AzureBlue,
    Color::EstorilBlue,
    Color::BlueJay,
    Color::CharcoalBlue,
    Color::DarkBlueGray,
    Color::DarkSlate,
    Color::DeepSeaBlue,
    Color::NightBlue,
    Color::MidnightBlue,
    Color::Navy,
    Color::DenimDarkBlue,
    Color::DarkBlue,
    Color::LapisBlue,
    Color::NewMidnightBlue,
    Color::EarthBlue,
    Color::CobaltBlue,
    Color::MediumBlue,
    Color::BlueberryBlue,
    Color::CanaryBlue,
    Color::Blue,
    Color::SamcoBlue,
    Color::BrightBlue,
    Color::BlueOrchid,
    Color::SapphireBlue,
    Color::BlueEyes,
    Color::BrightNavyBlue,
    Color::BalloonBlue,
    Color::RoyalBlue,
    Color::OceanBlue,
    Color::DarkSkyBlue,
    Color::BlueRibbon,
    Color::BlueDress,
    Color::NeonBlue,
    Color::DodgerBlue,
    Color::GlacialBlueIce,
    Color::SteelBlue,
    Color::SilkBlue,
    Color::WindowsBlue,
    Color::BlueIvy,
    Color::CyanBlue,
    Color::BlueKoi,
    Color::ColumbiaBlue,
    Color::BabyBlue,
    Color::CornflowerBlue,
    Color::SkyBlueDress,
    Color::Iceberg,
    Color::ButterflyBlue,
    Color::DeepSkyBlue,
    Color::MiddayBlue,
    Color::CrystalBlue,
    Color::DenimBlue,
    Color::DaySkyBlue,
    Color::LightSkyBlue,
    Color::SkyBlue,
    Color::JeansBlue,
    Color::BlueAngel,
    Color::PastelBlue,
    Color::LightDayBlue,
    Color::SeaBlue,
    Color::HeavenlyBlue,
    Color::RobinEggBlue,
    Color::PowderBlue,
    Color::CoralBlue,
    Color::LightBlue,
    Color::LightSteelBlue,
    Color::GulfBlue,
    Color::PastelLightBlue,
    Color::LavenderBlue,
    Color::WhiteBlue,
    Color::Lavender,
    Color::Water,
    Color::AliceBlue,
    Color::GhostWhite,
    Color::Azure,
    Color::LightCyan,
    Color::LightSlate,
    Color::ElectricBlue,
    Color::TronBlue,
    Color::BlueZircon,
    Color::Cyan,
    Color::Aqua,
    Color::BrightCyan,
    Color::Celeste,
    Color::BlueDiamond,
    Color::BrightTurquoise,
    Color::BlueLagoon,
    Color::PaleTurquoise,
    Color::PaleBlueLily,
    Color::LightTeal,
    Color::TiffanyBlue,
    Color::BlueHosta,
    Color::CyanOpaque,
    Color::NorthernLightsBlue,
    Color::BlueGreen,
    Color::MediumAquaMarine,
    Color::AquaSeafoamGreen,
    Color::MagicMint,
    Color::LightAquamarine,
    Color::Aquamarine,
    Color::BrightTeal,
    Color::Turquoise,
    Color::MediumTurquoise,
    Color::DeepTurquoise,
    Color::Jellyfish,
    Color::BlueTurquoise,
    Color::DarkTurquoise,
    Color::MacawBlueGreen,
    Color::LightSeaGreen,
    Color::SeafoamGreen,
    Color::CadetBlue,
    Color::DeepSea,
    Color::DarkCyan,
    Color::TealGreen,
    Color::Teal,
    Color::TealBlue,
    Color::MediumTeal,
    Color::DarkTeal,
    Color::DeepTeal,
    Color::DarkSlateGray,
    Color::Gunmetal,
    Color::BlueMossGreen,
    Color::BeetleGreen,
    Color::GrayishTurquoise,
    Color::GreenishBlue,
    Color::AquamarineStone,
    Color::SeaTurtleGreen,
    Color::DullSeaGreen,
    Color::DarkGreenBlue,
    Color::DeepSeaGreen,
    Color::BottleGreen,
    Color::SeaGreen,
    Color::ElfGreen,
    Color::DarkMint,
    Color::Jade,
    Color::EarthGreen,
    Color::ChromeGreen,
    Color::Mint,
    Color::Emerald,
    Color::IsleOfManGreen,
    Color::MediumSeaGreen,
    Color::MetallicGreen,
    Color::CamouflageGreen,
    Color::SageGreen,
    Color::HazelGreen,
    Color::VenomGreen,
    Color::OliveDrab,
    Color::Olive,
    Color::Ebony,
    Color::DarkOliveGreen,
    Color::MilitaryGreen,
    Color::GreenLeaves,
    Color::ArmyGreen,
    Color::FernGreen,
    Color::FallForestGreen,
    Color::IrishGreen,
    Color::PineGreen,
    Color::MediumForestGreen,
    Color::RacingGreen,
    Color::JungleGreen,
    Color::CactusGreen,
    Color::ForestGreen,
    Color::Green,
    Color::DarkGreen,
    Color::DeepGreen,
    Color::DeepEmeraldGreen,
    Color::HunterGreen,
    Color::DarkForestGreen,
    Color::LotusGreen,
    Color::BroccoliGreen,
    Color::SeaweedGreen,
    Color::ShamrockGreen,
    Color::GreenOnion,
    Color::MossGreen,
    Color::GrassGreen,
    Color::GreenPepper,
    Color::DarkLimeGreen,
    Color::ParrotGreen,
    Color::CloverGreen,
    Color::DinosaurGreen,
    Color::GreenSnake,
    Color::AlienGreen,
    Color::GreenApple,
    Color::LimeGreen,
    Color::PeaGreen,
    Color::KellyGreen,
    Color::ZombieGreen,
    Color::GreenPeas,
    Color::DollarBillGreen,
    Color::FrogGreen,
    Color::TurquoiseGreen,
    Color::DarkSeaGreen,
    Color::BasilGreen,
    Color::GrayGreen,
    Color::LightOliveGreen,
    Color::IguanaGreen,
    Color::CitronGreen,
    Color::AcidGreen,
    Color::AvocadoGreen,
    Color::PistachioGreen,
    Color::SaladGreen,
    Color::YellowGreen,
    Color::PastelGreen,
    Color::HummingbirdGreen,
    Color::NebulaGreen,
    Color::StoplightGoGreen,
    Color::NeonGreen,
    Color::JadeGreen,
    Color::SpringGreen,
    Color::OceanGreen,
    Color::LimeMintGreen,
    Color::MediumSpringGreen,
    Color::AquaGreen,
    Color::EmeraldGreen,
    Color::Lime,
    Color::LawnGreen,
    Color::BrightGreen,
    Color::Chartreuse,
    Color::YellowLawnGreen,
    Color::AloeVeraGreen,
    Color::DullGreenYellow,
    Color::LemonGreen,
    Color::GreenYellow,
    Color::ChameleonGreen,
    Color::NeonYellowGreen,
    Color::YellowGreenGrosbeak,
    Color::TeaGreen,
    Color::SlimeGreen,
    Color::AlgaeGreen,
    Color::LightGreen,
    Color::DragonGreen,
    Color::PaleGreen,
    Color::MintGreen,
    Color::GreenThumb,
    Color::OrganicBrown,
    Color::LightJade,
    Color::LightMintGreen,
    Color::LightRoseGreen,
    Color::ChromeWhite,
    Color::HoneyDew,
    Color::MintCream,
    Color::LemonChiffon,
    Color::Parchment,
    Color::Cream,
    Color::CreamWhite,
    Color::LightGoldenRodYellow,
    Color::LightYellow,
    Color::Beige,
    Color::WhiteYellow,
    Color::Cornsilk,
    Color::Blonde,
    Color::AntiqueWhite,
    Color::LightBeige,
    Color::PapayaWhip,
    Color::Champagne,
    Color::BlanchedAlmond,
    Color::Bisque,
    Color::Wheat,
    Color::Moccasin,
    Color::Peach,
    Color::LightOrange,
    Color::PeachPuff,
    Color::CoralPeach,
    Color::NavajoWhite,
    Color::GoldenBlonde,
    Color::GoldenSilk,
    Color::DarkBlonde,
    Color::LightGold,
    Color::Vanilla,
    Color::TanBrown,
    Color::DirtyWhite,
    Color::PaleGoldenRod,
    Color::Khaki,
    Color::CardboardBrown,
    Color::HarvestGold,
    Color::SunYellow,
    Color::CornYellow,
    Color::PastelYellow,
    Color::NeonYellow,
    Color::Yellow,
    Color::LemonYellow,
    Color::CanaryYellow,
    Color::BananaYellow,
    Color::MustardYellow,
    Color::GoldenYellow,
    Color::BoldYellow,
    Color::SafetyYellow,
    Color::RubberDuckyYellow,
    Color::Gold,
    Color::BrightGold,
    Color::ChromeGold,
    Color::GoldenBrown,
    Color::DeepYellow,
    Color::MacaroniandCheese,
    Color::Amber,
    Color::Saffron,
    Color::NeonGold,
    Color::Beer,
    Color::YellowOrange,
    Color::OrangeYellow,
    Color::Cantaloupe,
    Color::CheeseOrange,
    Color::Orange,
    Color::BrownSand,
    Color::SandyBrown,
    Color::BrownSugar,
    Color::CamelBrown,
    Color::DeerBrown,
    Color::BurlyWood,
    Color::Tan,
    Color::LightFrenchBeige,
    Color::Sand,
    Color::SoftHazel,
    Color::Sage,
    Color::FallLeafBrown,
    Color::GingerBrown,
    Color::BronzeGold,
    Color::DarkKhaki,
    Color::OliveGreen,
    Color::Brass,
    Color::CookieBrown,
    Color::MetallicGold,
    Color::Mustard,
    Color::BeeYellow,
    Color::SchoolBusYellow,
    Color::GoldenRod,
    Color::OrangeGold,
    Color::Caramel,
    Color::DarkGoldenRod,
    Color::Cinnamon,
    Color::Peru,
    Color::Bronze,
    Color::PumpkinPie,
    Color::TigerOrange,
    Color::Copper,
    Color::DarkGold,
    Color::MetallicBronze,
    Color::DarkAlmond,
    Color::Wood,
    Color::KhakiBrown,
    Color::OakBrown,
    Color::AntiqueBronze,
    Color::Hazel,
    Color::DarkYellow,
    Color::DarkMoccasin,
    Color::KhakiGreen,
    Color::MillenniumJade,
    Color::DarkBeige,
    Color::BulletShell,
    Color::ArmyBrown,
    Color::Sandstone,
    Color::Taupe,
    Color::DarkGrayishOlive,
    Color::DarkHazelBrown,
    Color::Mocha,
    Color::MilkChocolate,
    Color::GrayBrown,
    Color::DarkCoffee,
    Color::WesternCharcoal,
    Color::OldBurgundy,
    Color::RedBrown,
    Color::BakersBrown,
    Color::PullmanBrown,
    Color::DarkBrown,
    Color::SepiaBrown,
    Color::DarkBronze,
    Color::Coffee,
    Color::BrownBear,
    Color::RedDirt,
    Color::Sepia,
    Color::Sienna,
    Color::SaddleBrown,
    Color::DarkSienna,
    Color::Sangria,
    Color::BloodRed,
    Color::Chestnut,
    Color::CoralBrown,
    Color::DeepAmber,
    Color::ChestnutRed,
    Color::GingerRed,
    Color::Mahogany,
    Color::RedGold,
    Color::RedFox,
    Color::DarkBisque,
    Color::LightBrown,
    Color::PetraGold,
    Color::BrownRust,
    Color::Rust,
    Color::CopperRed,
    Color::OrangeSalmon,
    Color::Chocolate,
    Color::Sedona,
    Color::PapayaOrange,
    Color::HalloweenOrange,
    Color::NeonOrange,
    Color::BrightOrange,
    Color::FluroOrange,
    Color::PumpkinOrange,
    Color::SafetyOrange,
    Color::CarrotOrange,
    Color::DarkOrange,
    Color::ConstructionConeOrange,
    Color::IndianSaffron,
    Color::SunriseOrange,
    Color::MangoOrange,
    Color::Coral,
    Color::BasketBallOrange,
    Color::LightSalmonRose,
    Color::LightSalmon,
    Color::PinkOrange,
    Color::DarkSalmon,
    Color::Tangerine,
    Color::LightCopper,
    Color::SalmonPink,
    Color::Salmon,
    Color::PeachPink,
    Color::LightCoral,
    Color::PastelRed,
    Color::PinkCoral,
    Color::BeanRed,
    Color::ValentineRed,
    Color::IndianRed,
    Color::Tomato,
    Color::ShockingOrange,
    Color::OrangeRed,
    Color::Red,
    Color::NeonRed,
    Color::ScarletRed,
    Color::RubyRed,
    Color::FerrariRed,
    Color::FireEngineRed,
    Color::LavaRed,
    Color::LoveRed,
    Color::Grapefruit,
    Color::StrawberryRed,
    Color::CherryRed,
    Color::ChilliPepper,
    Color::FireBrick,
    Color::TomatoSauceRed,
    Color::Brown,
    Color::CarbonRed,
    Color::Cranberry,
    Color::SaffronRed,
    Color::CrimsonRed,
    Color::RedWine,
    Color::WineRed,
    Color::DarkRed,
    Color::MaroonRed,
    Color::Maroon,
    Color::Burgundy,
    Color::Vermilion,
    Color::DeepRed,
    Color::GarnetRed,
    Color::RedBlood,
    Color::BloodNight,
    Color::DarkScarlet,
    Color::ChocolateBrown,
    Color::BlackBean,
    Color::DarkMaroon,
    Color::Midnight,
    Color::PurpleLily,
    Color::PurpleMaroon,
    Color::PlumPie,
    Color::PlumVelvet,
    Color::DarkRaspberry,
    Color::VelvetMaroon,
    Color::RosyFinch,
    Color::DullPurple,
    Color::Puce,
    Color::RoseDust,
    Color::PastelBrown,
    Color::RosyPink,
    Color::RosyBrown,
    Color::KhakiRose,
    Color::LipstickPink,
    Color::DuskyPink,
    Color::PinkBrown,
    Color::OldRose,
    Color::DustyPink,
    Color::PinkDaisy,
    Color::Rose,
    Color::DustyRose,
    Color::SilverPink,
    Color::GoldPink,
    Color::RoseGold,
    Color::DeepPeach,
    Color::PastelOrange,
    Color::DesertSand,
    Color::UnbleachedSilk,
    Color::PigPink,
    Color::PalePink,
    Color::Blush,
    Color::MistyRose,
    Color::PinkBubbleGum,
    Color::LightRose,
    Color::LightRed,
    Color::RoseQuartz,
    Color::WarmPink,
    Color::DeepRose,
    Color::Pink,
    Color::LightPink,
    Color::SoftPink,
    Color::PowderPink,
    Color::DonutPink,
    Color::BabyPink,
    Color::FlamingoPink,
    Color::PastelPink,
    Color::RosePink,
    Color::CadillacPink,
    Color::CarnationPink,
    Color::PastelRose,
    Color::BlushRed,
    Color::PaleVioletRed,
    Color::PurplePink,
    Color::TulipPink,
    Color::BashfulPink,
    Color::DarkPink,
    Color::DarkHotPink,
    Color::HotPink,
    Color::WatermelonPink,
    Color::VioletRed,
    Color::HotDeepPink,
    Color::BrightPink,
    Color::RedMagenta,
    Color::DeepPink,
    Color::NeonPink,
    Color::ChromePink,
    Color::NeonHotPink,
    Color::PinkCupcake,
    Color::RoyalPink,
    Color::DimorphothecaMagenta,
    Color::BarbiePink,
    Color::PinkLemonade,
    Color::RedPink,
    Color::Raspberry,
    Color::Crimson,
    Color::BrightMaroon,
    Color::RoseRed,
    Color::RoguePink,
    Color::BurntPink,
    Color::PinkViolet,
    Color::MagentaPink,
    Color::MediumVioletRed,
    Color::DarkCarnationPink,
    Color::RaspberryPurple,
    Color::PinkPlum,
    Color::Orchid,
    Color::DeepMauve,
    Color::Violet,
    Color::FuchsiaPink,
    Color::BrightNeonPink,
    Color::Magenta,
    Color::Fuchsia,
    Color::CrimsonPurple,
    Color::HeliotropePurple,
    Color::TyrianPurple,
    Color::MediumOrchid,
    Color::PurpleFlower,
    Color::OrchidPurple,
    Color::RichLilac,
    Color::PastelViolet,
    Color::Rosy,
    Color::MauveTaupe,
    Color::ViolaPurple,
    Color::Eggplant,
    Color::PlumPurple,
    Color::Grape,
    Color::PurpleNavy,
    Color::SlateBlue,
    Color::BlueLotus,
    Color::Blurple,
    Color::LightSlateBlue,
    Color::MediumSlateBlue,
    Color::PeriwinklePurple,
    Color::VeryPeri,
    Color::BrightGrape,
    Color::BrightPurple,
    Color::PurpleAmethyst,
    Color::BlueMagenta,
    Color::DarkBlurple,
    Color::DeepPeriwinkle,
    Color::DarkSlateBlue,
    Color::PurpleHaze,
    Color::PurpleIris,
    Color::DarkPurple,
    Color::DeepPurple,
    Color::MidnightPurple,
    Color::PurpleMonster,
    Color::Indigo,
    Color::BlueWhale,
    Color::RebeccaPurple,
    Color::PurpleJam,
    Color::DarkMagenta,
    Color::Purple,
    Color::FrenchLilac,
    Color::DarkOrchid,
    Color::DarkViolet,
    Color::PurpleViolet,
    Color::JasminePurple,
    Color::PurpleDaffodil,
    Color::ClematisViolet,
    Color::BlueViolet,
    Color::PurpleSageBush,
    Color::LovelyPurple,
    Color::NeonPurple,
    Color::PurplePlum,
    Color::AztechPurple,
    Color::MediumPurple,
    Color::LightPurple,
    Color::CrocusPurple,
    Color::PurpleMimosa,
    Color::PastelIndigo,
    Color::LavenderPurple,
    Color::RosePurple,
    Color::Viola,
    Color::Periwinkle,
    Color::PaleLilac,
    Color::Lilac,
    Color::Mauve,
    Color::BrightLilac,
    Color::PurpleDragon,
    Color::Plum,
    Color::BlushPink,
    Color::PastelPurple,
    Color::BlossomPink,
    Color::WisteriaPurple,
    Color::PurpleThistle,
    Color::Thistle,
    Color::PurpleWhite,
    Color::PeriwinklePink,
    Color::CottonCandy,
    Color::LavenderPinocchio,
    Color::DarkWhite,
    Color::AshWhite,
    Color::WarmWhite,
    Color::WhiteChocolate,
    Color::CreamyWhite,
    Color::OffWhite,
    Color::SoftIvory,
    Color::CosmicLatte,
    Color::PearlWhite,
    Color::RedWhite,
    Color::LavenderBlush,
    Color::Pearl,
    Color::EggShell,
    Color::OldLace,
    Color::WhiteIce,
    Color::Linen,
    Color::SeaShell,
    Color::BoneWhite,
    Color::Rice,
    Color::FloralWhite,
    Color::Ivory,
    Color::WhiteGold,
    Color::LightWhite,
    Color::Cotton,
    Color::Snow,
    Color::MilkWhite,
    Color::HalfWhite,
    Color::White,
];
