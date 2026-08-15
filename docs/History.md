
# foo_uie_playlist_tree History

v1.5.1.0, 2026-08-15
- Fixed: Calculation of display dimensions during startup. (Regression)

v1.5.0.0, 2026-08-14

- Improved: A couple of usability tweaks
  - Press Ctrl when you expand or collapse a folder item to expand or collapse the complete branch.
  - When creating a new folder the tree item will automatically enter Edit mode.
  - New setting to disable the horizontal scrollbar.
  - New setting to disable expanding the drop target after a drop.
  - New context menu item to display the preferences page.
  - New context menu item to 'freeze' the expanded or collapsed state of a folder.
  - New images for an autoplaylist, a playing autoplaylist and a frozen folder.
  - Auto Complete shows any node that contains the typed text, not just the nodes that start with the text.
- Breaking change:
  - `%node_item_count%`, `%playlist_duration%` and `%playlist_size%` are now unformatted so that they can be used by the foobar2000 math functions.
  - `%node_item_count_locale%`, `%playlist_duration_locale%` and `%playlist_size_locale%` are formatted using your Windows Locale number format settings.
- Fixed: A couple of small render bugs when switching between the light and dark theme.
- Fixed: The item indentation was not calculated correctly.
- Fixed: Dark Mode support for the Search box when focused.
- Fixed: Loading images from a path that contains title formatting fields. (Regression)
- Fixed: Some regional number formats (such as french/France "fr-FR") containing a Narrow No-Break Space (0x202F) did not render correctly.

v1.4.3.0, 2026-08-04

* Improved: Rendering of tree node drag & drop.
  * Reduced flicker.
  * Removed all artifacts caused by the insertion marker (hopefully).
  * Since the switch to custom rendering the drag image was broken.

v1.4.2.0, 2026-08-02

* New: The images can be loaded from all [Windows Imaging Component](https://learn.microsoft.com/en-us/windows/win32/wic/-wic-about-windows-imaging-codec) (WIC) supported formats (ICO, PNG, GIF, TIFF, JPEG, WebP, etc...).
* New: Context menu item to show the autoplaylist UI if available.
* New: The left margin will adjust itself depending on folders being used or not.
* Fixed: The size of some playlists was not calculated correctly.

v1.4.1.0, 2026-08-01

* New: Option to hide the Quick Search text box.
* Improved: Optimized startup a little bit.
* Fixed: Rendering a horizontally scrolled treeview.
* Fixed: Playlist Restore menu was broken. (Regression)

v1.4.0.0, 2026-07-31

* New: A `Playlist` submenu will be added to the context menu that contains the standard foobar2000 context menu for a playlist when one is selected.
* New: Quick Search text box to quickly highlight a folder or playlist. The text box supports Auto Complete.
* New: Image type `Folder (Locked)` for folders that contain playlists that are locked for removal.
* Improved: Lock restrictions are applied more fine-grained.
* Improved: A folder that contains at least one playlist that is locked for removal can no longer be removed.
* Changed: Removed support for `\n`. The foobar2000 function `$crlf()` achieves the same result.
* Fixed: Resource leak in Preferences dialog.

v1.3.1.0, 2026-07-26

* Improved: Dropping files or playlist items on a folder creates a playlist in that folder.
* Fixed: Font changes not correctly applied at startup.
* Fixed: Incorrect redraw behavior during a drag & drop operation from outside the tree view.
* Fixed: Folders could not be renamed. (Regression)

v1.3.0.0, 2026-07-25

* New: Playlist lock support
  * Context menu to add or remove fine-grained lock restrictions.
  * The name of the lock will be displayed at the bottom of the menu.
  * New image **Playlist (Locked)**
  * New custom fields: %node_is_locked% and %node_lock_name%
* New: Font support
  * DUI uses the **Playlists** font, when enabled.
  * CUI uses the **Common (list items)** font, when enabled.
* Fixed: The expanded/collapsed state of a folder was not restored at startup.

v1.2.0.0, 2026-07-23

* New: You can select the image file path using a standard file dialog.
* New: Configurable node image size.
* New: Configurable tool tip.
* New: %playlist_size_natural% and %playlist_duration_natural% formatting fields.
* Improved: Minor cosmetic changes.

v1.1.1.0, 2026-07-21

* New: You can use the middle mouse button to remove a folder or playlist.
* Improved: Major optimization of drop operations from playlists or the media library.
* Improved: Minor code tweaks to better support switching between light and dark mode.
* Fixed: Select the last active playlist upon startup.

v1.1.0.0, 2026-07-16

* New: [User Guide](https://github.com/stuerp/foo_uie_playlist_tree/blob/main/docs/README.md).
* New: **Load playlist**, **Save playlist** and **Save all playlists** commands.
* New: ARM64EC version.
* Improved: Usability
* Improved: DUI and CUI rendering.
* Fixed: A nasty bug in the serialization/deserialization of the component state that occurred only in the Release build. As a result this version will reset the state to its default.

v1.0.0.0, 2026-07-13, *"Scratchin' the itch"*

* Initial release.
