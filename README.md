iNFekt
===
[![](https://img.shields.io/website-up-down-green-red/https/infekt.ws.svg?label=website)](https://infekt.ws/)
[![](https://img.shields.io/github/tag/syndicodefront/infekt.svg)](https://infekt.ws/index.html#downloads)

A text viewer application that has been carefully designed around its main task: viewing and presenting [NFO files](http://en.wikipedia.org/wiki/.nfo). It comes with three different view modes (Rendered, Classic and Text Only), export functionality and lots of options!

iNFekt 2.x is the soon-to-be-current Rust implementation. The previous C++ 1.x codebase remains in this repository for patch maintenance under [`legacy/cpp`](docs/legacy-cpp.md).

The Rust workspace is the default entrypoint:

```sh
cargo build --workspace
cargo run -p infekt
cargo run -p infekt-cli -- --help
cargo run -p xtask -- bundle --release
```

The C++ 1.x maintenance tree can still be configured explicitly:

```sh
cmake -S legacy/cpp -B build/legacy-cpp
```

- [Official website](https://infekt.ws/)
- [Downloads](https://infekt.ws/index.html#downloads)
- [FAQ](FAQ.md)
- [Changelog](CHANGELOG.md)
- [Legacy C++ maintenance notes](docs/legacy-cpp.md)

## Screenshots

<div>
  <img src="https://infekt.ws/screens/infekt-screen-1.png" height="250">
  <img src="https://infekt.ws/screens/infekt-screen-2.png" height="250">
  <img src="https://infekt.ws/screens/infekt-screen-3.png" height="250">
  <img src="https://infekt.ws/screens/infekt-screen-4.png" height="250">
</div>
