Kwooty is a friendly newsgroup binary downloader that uses .nzb as entry files.

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
 
