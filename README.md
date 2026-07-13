
# foo_uie_playlist_tree

[foo_uie_playlist_tree](https://github.com/stuerp/foo_uie_playlist_tree/releases) is a [foobar2000](https://www.foobar2000.org/) component to organize playlists in a tree view.

It attempts to recreate the functionality of [foo_plorg](https://wiki.hydrogenaudio.org/index.php?title=Foobar2000:Components/Playlist_Organizer_%28foo_plorg%29).

## Features

- Supports the Default User Interface (DUI) and the [Columns User Interface](https://yuo.be/columns-ui) (CUI).
- Supports dark mode.
- Supports foobar2000 2.0 and later (Intel 32-bit/64-bit and ARM64EC version).
- Custom Title Formatting fields
- User defined tree images
- Drag & drop from foobar2000, Explorer and other applications.
- Supports the foobar2000 playlist recycle bin.

## Requirements

- [foobar2000](https://www.foobar2000.org/download) v2.0 or later. ![foobar2000](https://www.foobar2000.org/button-small.png)
- Tested on Microsoft Windows 10 and later.
- Tested with [Columns UI](https://yuo.be/columns-ui) 2.1.0.

## Getting started

- Double-click `foo_uie_playlist_tree.fbk2-component`.

or

- Import `foo_uie_playlist_tree.fbk2-component` into foobar2000 using the "*File / Preferences / Components / Install...*" menu item.

## Developing

### Requirements

To build the code you need:

- [Microsoft Visual Studio 2026 Community Edition](https://visualstudio.microsoft.com/downloads/) or later
- [foobar2000 SDK](https://www.foobar2000.org/SDK) 2025-03-07
- [Windows Template Library (WTL)](https://github.com/Win32-WTL/WTL) 10.0.10320
- [Columns UI SDK](https://yuo.be/columns-ui-sdk) 8.0.0

To create the deployment package you need:

- [PowerShell 7.2](https://github.com/PowerShell/PowerShell) or later

### Setup

Create the following directory structure:

    3rdParty
        columns_ui-sdk
        WTL10_10320
    bin
    bin.x86
    foo_uie_playlist_tree
    int
    out
    sdk

- `3rdParty/columns_ui-sdk` contains the Columns UI SDK.
- `3rdParty/WTL10_10320` contains WTL 10.0.10320.
- `bin` contains a portable version of foobar2000 64-bit for debugging purposes.
- `bin.x86` contains a portable version of foobar2000 32-bit for debugging purposes.
- `foo_uie_playlist_tree` contains the [Git](https://github.com/stuerp/foo_uie_playlist_tree) repository.
- `int` receives the intermedidiate files of the build.
- `out` receives a deployable version of the component.
- `sdk` contains the foobar2000 SDK.

### Building

Open `foo_uie_playlist_tree.sln` with Visual Studio and build the solution.

### Packaging

To create the component first build the x86 configuration and next the x64 configuration.

## Change Log

v1.1.0.0, 2026-xx-xx

- 

## Acknowledgements / Credits

- Peter Pawlowski for the [foobar2000](https://www.foobar2000.org/) audio player. ![foobar2000](https://www.foobar2000.org/button-small.png)

## Links

- Home page: [https://github.com/stuerp/foo_uie_playlist_tree](https://github.com/stuerp/foo_uie_playlist_tree)
- Repository: [https://github.com/stuerp/foo_uie_playlist_tree.git](https://github.com/stuerp/foo_uie_playlist_tree.git)
- Issue tracker: [https://github.com/stuerp/foo_uie_playlist_tree/issues](https://github.com/stuerp/foo_uie_playlist_tree/issues)
- Support: [Hydrogen Audio foobar2000 forum](https://hydrogenaud.io/index.php/topic,129734.0.html)

## License

![License: MIT](https://img.shields.io/badge/license-MIT-yellow.svg)
