Name:       aliendalvik-control

%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}
Summary:    Aliendalvik control
Version:    1.0.0
Release:    1
Group:      Qt/Qt
License:    WTFPL
URL:        https://github.com/CODeRUS/aliendalvik-control
Source0:    %{name}-%{version}.tar.bz2
Requires:   sailfishsilica-qt5 >= 0.10.9
Requires:   aliendalvik
Requires:   sailfish-version >= 2.0.0
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  desktop-file-utils

%description
D-Bus daemon for sending commands to aliendalvik

%prep
%setup -q -n %{name}-%{version}

%build

%qtc_qmake5

%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%qmake5_install

%pre
if /sbin/pidof aliendalvik-control > /dev/null; then
killall aliendalvik-control
fi

%post
systemctl-user restart aliendalvik-control

%preun
/usr/bin/aliendalvik-control restore
if /sbin/pidof aliendalvik-control > /dev/null; then
killall aliendalvik-control
fi

%files
%defattr(-,root,root,-)
%attr(4755, root, root) %{_bindir}/aliendalvik-control
#%{_bindir}/aliendalvik-gui
%{_datadir}/dbus-1/services/org.coderus.aliendalvikcontrol.service
%{_libdir}/systemd/user/aliendalvik-control.service
%{_datadir}/applications/android-open-url.desktop
%{_datadir}/applications/android-open-url-selector.desktop