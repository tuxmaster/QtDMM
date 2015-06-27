Name:		QtDMM
Version:	0.9.5
Release:	1%{?dist}
Summary:	Application DMM's

Group:		Applications/Engineering
License:	GPLv3
URL:		http://www.mtoussaint.de/qtdmm.html
Source0:	%{name}-%{version}

BuildRequires:	qt5-qtserialport-devel

%description
Appication for dmm's.

%prep
%setup -q


%build
qmake-qt5
make %{?_smp_mflags}


%install
make install DESTDIR=%{buildroot}


%files
%doc



%changelog
* Sat Jun 27 2015 Frank Büttner <frank@familie-büttner.de> 0.9.5-1
- start
