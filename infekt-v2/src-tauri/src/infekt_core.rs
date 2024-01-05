use cxx::CxxString;

#[cxx::bridge]
pub mod ffi {
    unsafe extern "C++" {
        include!("stdafx.h");
        include!("nfo_data.h");

        type CNFOData;

        pub fn new_nfo_data() -> UniquePtr<CNFOData>;
        pub fn IsInError(self: &CNFOData) -> bool;
        pub fn LoadFromFile(self: Pin<&mut CNFOData>, path: &CxxString) -> bool;
        // pub fn LoadStripped(self: &CNFOData) -> bool;
    }
}
