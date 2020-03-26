#
# spec file for package freealut (1.1.0)
#

# norootforbuild
# neededforbuild SDL SDL-devel aalib aalib-devel alsa alsa-devel arts audiofile esound esound-devel glib2 glib2-devel libogg libogg-devel libstdc++-devel libvorbis libvorbis-devel openal openal-devel pkgconfig resmgr slang slang-devel smpeg smpeg-devel xorg-x11-devel xorg-x11-libs

BuildRequires: aaa_base acl attr bash bind-utils bison bzip2 coreutils cpio cpp cracklib cvs cyrus-sasl db devs diffutils e2fsprogs file filesystem fillup findutils flex gawk gdbm-devel glibc glibc-devel glibc-locale gpm grep groff gzip info insserv klogd less libacl libattr libgcc libnscd libselinux libstdc++ libxcrypt libzio m4 make man mktemp module-init-tools ncurses ncurses-devel net-tools netcfg openldap2-client openssl pam pam-modules patch permissions popt procinfo procps psmisc pwdutils rcs readline sed strace syslogd sysvinit tar tcpd texinfo timezone unzip util-linux vim zlib zlib-devel autoconf automake binutils gcc gdbm gettext libtool perl rpm SDL SDL-devel aalib aalib-devel alsa alsa-devel arts audiofile esound esound-devel glib2 glib2-devel libogg libogg-devel libstdc++-devel libvorbis libvorbis-devel openal openal-devel resmgr slang slang-devel smpeg smpeg-devel xorg-x11-devel xorg-x11-libs dialog expat fontconfig fontconfig-devel freetype2 freetype2-devel gnome-filesystem jack liblcms libjpeg libmng libpng libsndfile libtiff pciutils pkgconfig qt3 aaa_skel ash bind-libs gpg libgcj logrotate openslp suse-build-key suse-release tcsh

Name:         freealut
License:      LGPL
Group:        System/Libraries
Autoreqprov:  on
Version:      1.1.0
Release:      1
URL:          http://www.openal.org/
Summary:      Open Audio Library Utility Toolkit
Source:       freealut-%{version}.tar.gz
BuildRoot:    %{_tmppath}/%{name}-%{version}-build

%description
freealut is a highly portable Open Source implementation of ALUT, a
utility toolkit for OpenAL. ALUT makes managing of OpenAL contexts,
loading sounds in various formats and creating waveforms very easy.



Authors:
--------
    Erik Hofman <erik@ehofman.com>
    Steve Baker <sjbaker1@airmail.net>
    Sven Panne <sven.panne@aedion.de>

%package devel
Summary:      Static libraries, header files and tests for the freealut library
Requires:     freealut = %{version}
Group:        Development/Libraries/C and C++

%description devel
freealut is a highly portable Open Source implementation of ALUT, a
utility toolkit for OpenAL. ALUT makes managing of OpenAL contexts,
loading sounds in various formats and creating waveforms very easy.



Authors:
--------
    Erik Hofman <erik@ehofman.com>
    Steve Baker <sjbaker1@airmail.net>
    Sven Panne <sven.panne@aedion.de>

%debug_package
%prep
%setup -q

%build
%{?suse_update_config:%{suse_update_config -f admin/autotools}}
test -f configure || ./autogen.sh
export CFLAGS="$RPM_OPT_FLAGS"
./configure --prefix=%{_prefix}
make

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install

# documentation
install -m 755 -d $RPM_BUILD_ROOT%{_defaultdocdir}/%{name}
install -m 644 AUTHORS COPYING ChangeLog NEWS README doc/alut.css doc/alut.html \
	       $RPM_BUILD_ROOT%{_defaultdocdir}/%{name}

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc %{_defaultdocdir}/%{name}/AUTHORS
%doc %{_defaultdocdir}/%{name}/COPYING
%doc %{_defaultdocdir}/%{name}/ChangeLog
%doc %{_defaultdocdir}/%{name}/NEWS
%doc %{_defaultdocdir}/%{name}/README
%{_libdir}/libalut.so.*

%files devel
%defattr(-,root,root)
%doc %{_defaultdocdir}/%{name}/alut.html
%doc %{_defaultdocdir}/%{name}/alut.css
%{_prefix}/bin/freealut-config
%{_includedir}/AL/alut.h
%{_libdir}/libalut.a
%{_libdir}/libalut.la
%{_libdir}/libalut.so
%{_libdir}/pkgconfig/freealut.pc

%changelog -n freealut
* Wed Apr 12 2006 - sven.panne@aedion.de
- Handle version via configure
* Thu Dec 15 2005 - sven.panne@aedion.de
- Added freealut-config and freealut.pc
* Mon Nov 28 2005 - sven.panne@aedion.de
- Fixed build dependencies
* Mon Oct 10 2005 - sven.panne@aedion.de
- Initial version
