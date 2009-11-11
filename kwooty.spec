BuildRequires: libkde4-devel uudeview update-desktop-files

Name:         kwooty
License:      GPL
Group:        Productivity/Networking/News/Clients
Summary:      A friendly nzb news grabber client
Version:      0.1.2
Release:      1
URL:          http://sourceforge.net/projects/kwooty/
BuildRoot:    %{_tmppath}/%{name}-%{version}-build
Source0:      %name-%{version}.tar.gz
%kde4_runtime_requires

%description
Kwooty is a friendly nzb newsgroup binary grabber.
Its main features are:
- Direct file downloading after opening .Nzb file
- File queue and priority management
- Support for automatic files verification/repairing (par2 program required)
- Support for automatic archive extraction (unrar program required)
- Built-in SSL connection support
- Pause/Resume downloads


%prep
# extract the source and go into the kwooty directory
%setup -q

%build
%cmake_kde4 -d build --
%make_jobs

%install
cd build
%makeinstall
%suse_update_desktop_file kwooty 


%clean
# clean up the hard disc after build
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/usr/bin/kwooty
/usr/share/appl*/*/kwooty.desktop
/usr/share/icons/hicolor/*/apps/kwooty.png
/usr/share/kde4/apps/kwooty
/usr/share/kde4/config.kcfg

%changelog
* Sat Oct 31 2009 Xavier Lefage <xavire.kwooty@gmail.com> 0.1.2  
- Fix forward compilation issues 
