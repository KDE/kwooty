Kwooty, a friendly newsgroup binary downloader for KDE 4.


Version 0.9.1:
--------------

- Fix bug: main window display property should now be correclty restored on next desktop login


Version 0.9.0:
--------------

- New plugin: "Categories". This plugin allows to transfer downloaded contents to favorites folders defined in the plugin settings.

- Scheduler plugin: bypass feature added. Scheduler can now be bypassed for items manually set on "Start" or "Pause" or both (to be configured in Scheduler settings).

- Scheduler plugin: new nzb items can now automatically be set on Pause when appended (feature is enabled if "Always limit download speed" is checked).

- Scheduler plugin: better download/pause behavior.

- New option "Display tiny file names" in "Display Mode" settings.

- Better distinction of par2 files: to improve readability, name of par2 files is displayed in gray.

- The mean download speed of each server has been added nearby its current download speed in the server statistics panel (feature request).

- Bug fix: maximized setting about server statistics panel was sometimes not correctly restored between kwooty's sessions.


Version 0.8.4:
--------------

- Fix issue #3528297 : shutdown was not working on gnome 3
- Feature request : if kwooty has been minimized in sytem tray, keep main window hidden when a nzb is loaded 


Version 0.8.3:
--------------

- rpmlint issues fix for fedora packaging


Version 0.8.2:
--------------

- French translation (thanks a lot Yurienu !)


Version 0.8.1:
--------------

* Fix regression: when nzb file name was too long, main window width was automatically increased
* Fix bug: scheduler plugin did not pause items when requested if download limit speed was set to "No Limit"
* Fix bug: temporary files from a cancelled nzb collection could not be removed if nzb collection was previously already downloaded


Version 0.8.0:
--------------

- New plugin: Automatic download retry. This plugin allows to download missing or corrupted files again automatically in order to improve chances to retrieve a correct file.

- Improve readability for files whose "download retry" has been requested, the following status will be displayed: "In queue (Retry)"

- Disk I/O reducing: Yenc segments are now on-the-fly decoded once downloaded.

- The "download retry" feature now re-downloads only segments that are either missing or corrupted.

- Feature request: added shortcut towards bandwidth throttle configuration by double clicking on the speed text located on bottom right in the status bar (if bandwidth manager plugin is active).

- When pending downloads are restored at a new start-up, "Pause" status from previous session is now kept.

- Icons next to nzb file name now notify the user to quickly check the current status of the file collection

- Better consistency between Connections/Disconnection icons

- Font color and style of labels in sidebar have been changed to improve readability

* Fix compilation issue in KDE SC 4.8



Version 0.7.3:
--------------

* Fix bug #3437022: pending download restoring at start-up could be lost in certain situations
* Fix regression #3443484: when interet connection was lost then back again, downloads were not restarting automatically
* Fix bug #3444490: when opening nzb file(s) from file manager, the kwooty's bouncing cursor was stop only after timeout
* Before restoring downloads at start-up, checksum of file that store queued downloads is now performed to ensure data integrity
* Fix issue with an incorrect value stored in kwootyrc under certain circumtances and could lead to a crash at next kwooty start-up
* Fix issue in yenc decoder that could consider correct decoded files as corrupted ('decoded (bad CRC)' status) with some yenc encoders.


Version 0.7.2:
--------------

* Fix bug #3386546 that could lead to server connection drop if Bandwidth manager plugin was active with download limit set to "No Limit"
* Fix bug #3390955 that unpacked zipped files format that were not intended to be (ie : .war files) 


Version 0.7.1:
--------------

* Fix issue when decoding uu encoded files whose file name contains white spaces (thanks to Nicholas)
* If message 481 from usenet server is received, try to download segment 1 minute later than considering it as not present
* Fix bug #3361080 (and regression from 0.6.3 by the way) that could lead to server connection drop 


Version 0.7.0:
--------------

- Retry action added (accessible via "right click > Retry" context menu or "Downloads > Retry" menu) that allows to re set in queue files whose download/verify/extract process failed

- Bandwidth management plugin added that allows to limit and schedule download speed

- Active servers will now try to also download files not found by Master server (in a load balancing way if several active servers are set) instead of just being considered as missing

- Basic splitted files (as name.ext.001, name.ext.002) merging process is handled (in addition of rar, zip and 7z archives)

- A decoded file with a bad CRC will be notified to user (with an appropriate icon and status)



Version 0.6.2:
--------------

* Fix compilation issue with KDE SC 4.6 shipped with new power management
* Small update in Czech translation


Version 0.6.1:
--------------

- Czech translation (thanks a lot Pavel !)

Bug fixes :

* Kwallet now reopens the wallet it is closed
* Credentials were not correctly removed from plain text when Kwallet option was enabled
* Watch folder was not working correctly on certain distro


Version 0.6.0:
--------------

- Multi-server support:

  - Up to 4 backup servers can be added.
    Server priority can be managed in settings by dragging and dropping tab to the desired position in "Connection" settings.

  - 4 modes are available for each backup server:

    - Passive: a Passive backup server will download files only not found on Master server and will stay Idle the rest of the time

    - Active: an Active backup server will download files simultaneously with Master server until all pending files are processed

    - Failover: a Failover backup server will work as "Passive" as long as Master server is available. 
		In any case Master server is down, the Failover backup server replaces Master server and downloads queued files until Master server come back available (several Failover backup servers are possible, priority is then managed by tab order in server settings)

    - Disabled: server is not used by server configuration is kept for later usage


- An information bar allowing to watch the following information on a per-server basis has been added (hidden or shown by clicking on the "double arrow" icon at the right of the status bar):
  Server availability, downloaded content size, current download speed, current downloaded file, server name, backup server mode and encryption information.

- Added new icon to identify queued files that need to be downloaded by a backup server 

- Kwallet support

- Pending downloads can be restored without confirmation at startup and / or be saved without confirmation at exit (configurable in "General > Confirmation dialogs" tab)

- Added context menu allows access to frequent actions (Start All, Start, Pause All , etc...)

- Added shortcuts to settings when double clicking on server status or shutdown information on status bar



Version 0.5.1:
--------------
- Display update : parent row height has been increased
- Display update : when parent row is selected or mouse hover, "Progress" column now displays colored background as other columns
- Display update : when a parent row is expanded, "File Name" column automatically adjusts its size according to children "File Name" text size and available window size.
- Behavior fix : when a new kde session is started and kwooty session is loaded, main window remains minimized if "Show system tray icon" is enabled
- Ready for language translation : kwooty.pot has been added in "po" directory. Any translations are welcome ! :D


Version 0.5.0:
--------------
- Systray icon support: download progress is displayed on systray icon (ala amarok).
                        moose over systray icon will provide information tooltip about jobs being processed.  
                        new Kde system tray support will be used (KStatusNotifierItem) if kwooty is compiled against Kde >= 4.4.
                        
- Added Pause all - Start all actions in systray icon context menu.
                        
- Nofication support: user can now be notified when a job is totally finished and when there is not enough free disk space.
  
- Nice process priority option: priority of external programs (par2, unrar, 7z) can now be lowered by settings a predefined or custom value to 'nice' program (configurable in Settings > External Programs > Priotiry tab).
 
- Plugin handling support: kwooty now handles plugins.

- "watch folder" feature has switched from core program to become a plugin part (can now be enabled/disabled in "Settings > Plugins").

- Updated closing confirmation dialog box: cancel quitting is now possible.
   
- Updated download speed display in status bar : speed is displayed in MiB/s when appropriate (instead of KiB/s).

- Preferences gui update.
  
- A few performance optimations.


Version 0.4.0:
--------------
- zip and 7z file extract support (7z or 7za program required).

- Added internal decoder for UUEncode file format.

- Added "Down" and "Up" buttons for better queue management ("Down" and "Up" buttons now replace "Bottom" and "Top" buttons in toolbar, they remain available from "Download" menu and by keyboard shortcuts as well).

- Added "Downloads" button in tool bar: opens current download folder with file manager.

- Remaining time or estimated time of arrival (ETA) of pending files has been added in status bar (you can switch between them in "Settings > Display Modes").

- Free disk space indicator added in status bar (enabled/disabled in "Settings > Display Modes").
  If free disk space is not sufficient to download remaining queued files, a warning icon will appear near free space bar.

- Added Watch folder feature (configurable in "Settings > General > advanced tab") : new .nzb files put in an user-specified watch folder will automatically be enqueued.

- scheduled shutdown time and estimated download time will now be displayed according to system time settings (AM/PM format).

- In "Connection" settings, when host connection port is set to 443 or 563, "Enable SSL connection" checkbox will automatically be set as checked.

* Bug fixes :
 - par2 files download process (mandatory for repairing because direct extract failed due to a Bad CRC archive file) could hang for a .nzb file with multi files-set content. 
 - Archive files with accentuated characters were not fully monitored during extract process (related to utf-8 and accentued characters taken from unrar output).  
 - workaround for QTBUG-7585 (related to Qt 4.6 series): tree expander could disappear during download process file when tree was not expanded.



Version 0.3.2:
--------------
This is a bugs fix release.

* tree expander [+] was not displayed with Qt 4.6.X series .
* Related to previous bug report #2955501, it could happen that some segments EOF were not correctly handled (kwooty did not succeed in finishing download).
  This release should fix these randomly download issues. 


Version 0.3.1:
--------------
This is a bugs fix release.

* SSL connection was not established when certificate could not be verified by authority instance. (bug #2942759)
+ SSL connection is now established even if certificate can not been verified (certificate expired, self-signed certificate, etc...). Detail about certificate verifying has been added in "connection" related tooltip at the bottom-left of the status bar.

* kwooty now handles server response 423 (no such article number) which could be responsible of bug #2955501.



Version 0.3.0:
--------------
- libuu decoding library has been dropped.

- An internal decoder for yEnc file format has been written for replacement :
        advantages : - internal yenc decoding is much more quicker than before
                     - crc integrity of downloaded files is now checked (allowing to enable next feature)
        drawbacks : other usenet coding formats as uuencoded, base64 data are not handled
               
- Par2 files are downloaded only if they are needed to recover broken files (configurable in settings).
        This feature is enabled by default and should be safe :
        if integrity of all downloaded rar files are OK, par2 files are not downloaded but if extracting process (for wathever reason) fails,
        then par2 files will be downloaded and classical archive verifying, repairing, extracting steps will occur.

- System shutdown feature added : it is now possible to schedule system shutdown (halt, standby, suspend, hibernate)
 either when jobs are finished or at a given precise time.
 
- A status progress bar has been added.  
 
- It is now possible to drag and drop .nzb file(s) from file manager to kwooty. As usual download will start immediatly.
   
- Nzb content sorting improvement : all par2 files are now placed at the bottom of the list when nzb files are added.

- workaround for correctly handling  .nzb files encoded with us-ascii format. 


Version 0.2.0:
--------------
 - Save/Restore pending downloads when application is closed/open. 
   Queued downloads are also saved automatically every 5 minutes in order to be restored at next session even if system is halted while kwooty runs.
 - Kwooty now handles Nzb files added from external applications (internet browser or file manager by example) by "open with..." dialog box. 
 - Added icons near next to file names in order to inform about current download status.
 - Added options about Save/Restore feature, "open with..." feature, icons display in settings.
 - Settings rearrangement.
  
  Note : 
  If your internet browser does not suggest kwooty by default in "open with..." dialog, 
  it should be located in /usr/local/bin/kwooty or /usr/bin/kwooty.



Version 0.1.2:
--------------
Fix other forward compilation issue with Karmic Koala.


Version 0.1.1:
--------------
Fix compilation issue with Kubuntu Jaunty.


Version 0.1.0 (first public release):
-------------------------------------

 - Automatic connection to host at start-up
 - Automatic file downloading after openning Nzb file
 - Nzb file queue and priority management
 - Support for automatic files verification/repairing when nzb download content has ended (par2 program required)
 - Support for automatic archive extraction when nzb download content has ended (unrar program required)
 - Built-in SSL connection support
 - Pause/Resume downloads
 - Suspends downloads if disk is full
 
