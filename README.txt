Kwooty is a friendly newsgroup binary downloader that uses .nzb files as entry files.

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
 
