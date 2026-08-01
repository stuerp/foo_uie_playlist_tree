
# foo_uie_playlist_tree History

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
