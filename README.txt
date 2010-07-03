Kwooty, a friendly newsgroup binary downloader for KDE 4.

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
 
