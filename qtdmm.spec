%bcond test 1
%global	toolchain gcc
%global _pkg_extra_cxxflags -Werror

Name:		qtdmm
Version:	26.1
# Für RC's -p -e rcX
# Sonst -p -e weglassen
Release:	%autorelease -p -e rc1
Summary:	DMM Readout Software Including a Configurable Recorder
License:	AGPL-3.0-or-later
URL:		https://www.qtdmm.de
BuildSystem:	cmake
%if %{with test}
BuildOption:	-DBUILD_TESTING=ON
%endif
Source0:	%{name}-%{version}.tar.gz
BuildRequires:	appdata-tools desktop-file-utils gcc-c++
BuildRequires:	cmake(Qt6Bluetooth) cmake(Qt6Charts) cmake(Qt6LinguistTools) cmake(Qt6SerialPort) cmake(Qt6Svg)
BuildRequires:	cmake(hidapi)
BuildRequires:	pkgconfig(cups)
Suggests:	sigrok-cli

%description
QtDMM is a graphical multimeter reader and logger based on Qt.
It reads more than 150 digital multimeters over serial and USB cables,
Bluetooth LE and the network.

%install
%cmake_install
# Wenn da denn getrennt ist
#%%find_lang %%{name} --with-qt

# Wenn es denn Übersetzungen eineln sind
#%%files -f %%{name}.lang

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/%{name}.desktop
appstream-util validate-relax --nonet %{buildroot}%{_metainfodir}/*.metainfo.xml
QT_QPA_PLATFORM=offscreen %ctest

%files
%license LICENSE
%doc AUTHORS README.md CHANGELOG
%{_bindir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
%{_mandir}/man1/%{name}.1*
%{_metainfodir}/*.xml

%changelog
%autochangelog
