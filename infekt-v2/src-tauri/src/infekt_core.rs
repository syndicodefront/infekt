#[cxx::bridge]
pub mod ffi {
    enum ENfoCharset {
        NFOC_AUTO = 1,
        NFOC_UTF16,
        NFOC_UTF8_SIG,
        NFOC_UTF8,
        NFOC_CP437,
        NFOC_CP437_IN_UTF8,
        NFOC_CP437_IN_UTF16,
        NFOC_CP437_STRICT,
        NFOC_WINDOWS_1252,
        NFOC_CP437_IN_CP437,
        NFOC_CP437_IN_CP437_IN_UTF8,

        _NFOC_MAX,
    }

    unsafe extern "C++" {
        include!("stdafx.h");
        include!("nfo_data.h");

        type CNFOData;
        type ENfoCharset;

        pub fn new_nfo_data() -> UniquePtr<CNFOData>;
        pub fn LoadFromFile(self: Pin<&mut CNFOData>, path: &CxxString) -> bool;
        pub fn GetCharset(self: &CNFOData) -> ENfoCharset;
    }
}
