Name:		QtDMM
Version:	0.9.6
Release:	1%{?dist}
Summary:	Application for dmm's
Summary(de):	Anwedung für DMM's
Group:		Applications/Engineering
License:	GPLv3
URL:		http://www.mtoussaint.de/qtdmm.html
Source0:	%{name}-%{version}.tar.xz

BuildRequires:	qt5-qtserialport-devel qt5-qttools-devel desktop-file-utils

%description
Application for dmm's.

%description -l de
Eine Anwendung für DMM's.

%post
/bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :

%postun
if [ $1 -eq 0 ] ; then
    /bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null
    /usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
fi

%posttrans
/usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :

%prep
%setup -q -n %{name}

%build
qmake-qt5
make %{?_smp_mflags}

%install
mkdir -p %{buildroot}/usr/bin
mkdir -p %{buildroot}%_qt5_translationdir
mkdir -p %{buildroot}%{_datadir}/pixmaps
install bin/qtdmm %{buildroot}/usr/bin/qtdmm
install -m 644 src/translations/*.qm %{buildroot}%_qt5_translationdir/
install -m 644 .pics/large/qtdmm.png %{buildroot}%{_datadir}/pixmaps/qtdmm.png
desktop-file-install --dir=%{buildroot}%{_datadir}/applications QtDMM.desktop
%find_lang %{name} --with-qt

%files -f %{name}.lang
%{_bindir}/*
%{_datadir}/pixmaps/qtdmm.png
%{_datadir}/applications/QtDMM.desktop
%doc AUTHORS COPYING LICENSE README CHANGELOG

%changelog
* XXXX Frank Büttner <frank@familie-büttner.de> 0.9.6-1
- update to 0.9.6
- German package description

* Sat Jun 27 2015 Frank Büttner <frank@familie-büttner.de> 0.9.5-1
- start
