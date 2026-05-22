# Legacy C++ Maintenance

iNFekt 1.x is the patch-maintained C++ implementation. It remains buildable in-tree, but Rust 2.x is the default product and root workspace.

The C++ tree lives under `legacy/cpp/`:

- `src/` contains the C++ application, CLI, renderer, plugins, updater, GTK port, and platform support code.
- `project/` contains the Visual Studio solution, project files, resource scripts, and installer scripts.
- `dependencies/` contains the C++ dependency headers and library placeholders.
- `release/`, `tools/`, and `build-out/` contain the 1.x release, helper, and build-output support files.

Configure the CMake-based C++ build from the repository root:

```sh
cmake -S legacy/cpp -B build/legacy-cpp
```

The Visual Studio solution is available at `legacy/cpp/project/iNFekt.sln`.

New development should happen in the Rust workspace unless it is specifically a 1.x patch.
