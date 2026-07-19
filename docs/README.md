
# foo_uie_playlist_tree User Guide

Welcome to [foo_uie_playlist_tree](https://github.com/stuerp/foo_uie_playlist_tree/releases). This guide will help you understand how to use the component effectively and get the most out of its features.

---

## Table of Contents

1. [Introduction](#introduction)
2. [Features](#features)
3. [Getting Started](#getting-started)
4. [Context Menu](#context-menu)
5. [Drag & Drop](#drag--drop)
6. [Configuring](#configuring)
7. [Troubleshooting](#troubleshooting)
8. [FAQs](#faqs)
9. [Reference Material](#reference-material)
10. [History](#history)
11. [Acknowledgements and Credits](#acknowledgements-and-credits)
12. [Support](#support)

---

## Introduction

[foo_uie_playlist_tree](https://github.com/stuerp/foo_uie_playlist_tree/releases) is a [foobar2000](https://www.foobar2000.org/) component to organize playlists in a tree view.

It attempts to recreate the functionality of [foo_plorg](https://wiki.hydrogenaudio.org/index.php?title=Foobar2000:Components/Playlist_Organizer_%28foo_plorg%29).

---

### System Requirements

- [foobar2000](https://www.foobar2000.org/download) v2.0 or later (Intel 32-bit/64-bit and ARM64EC version). ![foobar2000](https://www.foobar2000.org/button-small.png)
- Tested on Microsoft Windows 10 and later.

### Download foo_uie_playlist_tree

You can download the component from the foobar2000 [Components](https://www.foobar2000.org/components/view/foo_uie_playlist_tree+%28x64%29) repository or from the GitHub [Releases](https://github.com/stuerp/foo_uie_playlist_tree/releases) page.

### Installation

- Double-click `foo_uie_playlist_tree.fbk2-component` or import `foo_uie_playlist_tree.fbk2-component` using the foobar2000 Preferences dialog. Select the **File / Preferences / Components** menu item, select the **Components** page and click the **Install...** button.
- Follow the foobar2000 instructions.

> [!Tip]
> To verify if the installation was successful open the foobar2000 Preferences using the **File / Preferences / Components** menu item and look for **Playlist Tree** in the **Installed components** list.

You can experiment with the component without modifying your layout by selecting **Playlist Tree** from the **View** menu.

---

## Features

- Supports the Default User Interface (DUI) and the [Columns User Interface](https://yuo.be/columns-ui) (CUI).
- Supports dark mode.
- Supports foobar2000 2.0 and later (Intel 32-bit/64-bit and ARM64EC version).
- Custom Title Formatting fields
- Configurable tree node images
- Drag & drop from foobar2000, Explorer and other applications.
- Supports the foobar2000 playlist recycle bin.

---

## Getting Started

The first time the component opens it loads all the playlists that are currently open and adds them to the root of the tree.

---

## Context mMnu

Right-click on the component window to display the context menu. The following items are available:

`New`

Creates a new folder or playlist item.

By default the new item gets created as a child of a folder item or below a playlist item over which the mouse pointer hovers.

If no item is highlighted the new item gets created at the bottom of the tree.

`Rename`

Allows you to rename the highlighted item.

`Remove`

Removes the highlighted item.

> [!Note]
> Removing a folder also removes all the items it contains.

`Sort`

Sorts the contents of the highlighed folder or the siblings of the selected playlist.

`Save playlist...`

Saves the playlist the mouse pointer is hovering over.

`Save all playlists...`

Saves all the playlists.

`Load playlist...`

Loads a playlist.

`Restore`

Allows you to restore a playlist that has been removed during the current session from the foobar200 playlist recycle bin.

> [!Note]
> This submenu only appears when one ore more playlists were removed.
> Ther `Clear history` menu items empties the playlist recycle bin.

---

## Drag & Drop

### Within the control

You can reorder the items of the tree using drag and drop.

When dropping a folder or playlist node think about how the drop target is divided into three zones:

- A top zone that occupies the top quarter of a node
- A bottom zone that occupies the bottom quarter of a node
- A center zone that is double as high as the top or the bottom zone.

Drop an item (folder or playlist) on the center zone of a folder to add it to that folder. Dropping an item on a playlist is ignored.

Drop an item (folder or playlist) on the top zone of an item to insert it before that item.

Drop an item (folder or playlist) on the bottom zone of an item to insert it after that item.

### From foobar2000

You can drag items from any playlist or from the media library.

When you drop one or more tracks on a playlist item the tracks get added to the playlist.

Drop one or more tracks on a folder item or anywhere on the panel and a new playlist gets created with the dragged tracks.

### From other applications

You can drag items from Windows Explorer or other applications that process files on the tree.

---

## Configuring

A preferences page is used to change all parameters used by the component. You can find it by selecting the **File / Preferences** menu item and then selecting **Display / Playlist Tree** in the dialog.

`Apply`

Applies the configuration to the component.

`OK`

Closes the dialog box and preserves the changes to the configuration during the current session.

> [!Note]
> The settings will only be saved to your profile when foobar2000 is closed.

`Cancel`

Closes the dialog box and undoes any changes to the configuration.

### Title Formatting Support

Where noted you can use the foobar2000 [Title Formatting](https://wiki.hydrogenaudio.org/index.php?title=Foobar2000:Title_Formatting_Introduction).

The component provides the following custom fields:

| Name                  | Description |
| --------------------- | ----------- |
| %node_name%           | The name of the item, either a playlist or a folder. |
| %node_is_folder%      | Boolean value for use in $if( ...) expressions and is true when the item is a folder. |
| %node_item_count%     | The number of playlists in a folder or number of items in a playlist. |
| %playlist_duration%   | The duration of the playlist (in seconds). |
| %playlist_size%       | The size of the playlist (in bytes). |
| %fb2k_path%           | The directory path of the foobar2000 installation. |
| %fb2k_component_path% | The directory path of the component. |
| %fb2k_profile_path%   | The directory path of the foobar2000 profile. |
| Any Windows environment variable | |

### Text Format

This setting determines how the text of a tree node gets formatted.

Here are some examples:

``` foobar2000
- %node_name%$if(%node_is_folder%,,' ('%node_item_count%')')
- %node_name%$if(%node_is_folder%,,' ('%node_item_count%') '%playlist_size%' bytes')
```

> [!Note]
> This setting supports title formatting.

### Node Image

You can configure the image that is used for each node type.

### Node Type

This setting determines for which node type the image will be selected.

- Folder: A folder in the tree
- Playlist: A playlist in the tree
- Playlist (Playing): The playlist from which foobar2000 is playing audio. This can be different from the currently displayed playlist.

### File Path

This setting determines which file will be used to create node images. Many Windows executables (.exe) and libraries (.dll) contain icons. The component can extract those icons and create node images from them.

Here are some examples:

- `imageres.dll` *Default*
- `shell32.dll`
- `foobar2000.exe`
- `%fb2k_component_path%\SomeIconLibrary.icl`

> [!Note]
> This setting supports title formatting.

### Image List

Select the image for the selected node type from this list.

---

## Troubleshooting

🔧 *Work in Progress*

---

## FAQs

🔧 *Work in Progress*

---

## Reference Material

This chapter contains some reference material I consulted during the development of foo_uie_playlist_tree.

### foobar2000

- [foobar2000 Development](https://wiki.hydrogenaud.io/index.php?title=Foobar2000:Development:Overview)

### Windows User Interface

- [Desktop App User Interface](https://learn.microsoft.com/en-us/windows/win32/windows-application-ui-development)
- [Windows User Experience Interaction Guidelines](https://learn.microsoft.com/en-us/windows/win32/uxguide/guidelines)
- [Windows Controls](https://learn.microsoft.com/en-us/windows/win32/controls/window-controls)
- [Control Library](https://learn.microsoft.com/en-us/windows/win32/controls/individual-control-info)
- [Resource-Definition Statements](https://learn.microsoft.com/en-us/windows/win32/menurc/resource-definition-statements)
- [About Custom Draw](https://learn.microsoft.com/en-us/windows/win32/controls/about-custom-draw)
- [Control Parts and States](https://learn.microsoft.com/en-us/windows/win32/controls/parts-and-states)
- [Visuals, Layout](https://learn.microsoft.com/en-us/windows/win32/uxguide/vis-layout)
- [The Old New Thing](https://devblogs.microsoft.com/oldnewthing/tag/what-a-drag)

### Various

- [Windows Icons](https://diymediahome.org/windows-icons-reference-list-with-details-locations-images/)
- [Using Explorer's fancy drag/drop effects in your own programs](https://devblogs.microsoft.com/oldnewthing/20210512-00/?p=105208), Raymond Che, The Old New Thing, 2021-05-12
- [Drag/drop effects: The little drop information box](https://devblogs.microsoft.com/oldnewthing/20210513-00/?p=105212), Raymond Che, The Old New Thing, 2021-05-13

---

## History

The history of foo_uie_playlist_tree development is available in a separate [document](History.md).

---

## Acknowledgements and Credits

- Peter Pawlowski for the [foobar2000](https://www.foobar2000.org/) audio player. ![foobar2000](https://www.foobar2000.org/button-small.png)
- [Niels Lohmann](https://github.com/nlohmann) for [JSON for Modern C++](https://github.com/nlohmann/json).

---

## Support

For further assistance:

🌐 Home page: [https://github.com/stuerp/foo_uie_playlist_tree](https://github.com/stuerp/foo_uie_playlist_tree)<br>
🌐 Repository: [https://github.com/stuerp/foo_uie_playlist_tree.git](https://github.com/stuerp/foo_uie_playlist_tree.git)<br>
🌐 Issue tracker: [https://github.com/stuerp/foo_uie_playlist_tree/issues](https://github.com/stuerp/foo_uie_playlist_tree/issues)<br>
🌐 Support: [Hydrogen Audio foobar2000 forum](https://hydrogenaud.io/index.php/topic,129734.0.html)<br>
