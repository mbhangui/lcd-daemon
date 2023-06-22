all: debian.tar.gz lcd-daemon.dsc
version=$(shell cat ../conf-version)
arch=$(shell uname -m)
email=$(shell cat ../conf-email)

edit = sed \
	-e 's,@version\@,$(version),g' \
	-e "s,@release\@,"`cat ../conf-release`",g" \
	-e 's,@email\@,$(email),g' \
	-e 's,@prefix\@,$(prefix),g'

changelog: ../doc/ChangeLog Makefile ../conf-version ../conf-release \
../conf-email
	../catChangeLog --debian --name=lcd-daemon --state=stable \
		--urgency=low ../doc/ChangeLog > $@

control: control.in ../conf-email
	$(edit) $@.in > $@

copyright: copyright.in ../conf-email
	$(edit) $@.in > $@

lcd-daemon.dsc: lcd-daemon.dsc.in Makefile ../conf-version ../conf-release ../conf-email
	$(edit) $@.in > $@

clean:
	/bin/rm -f changelog control copyright lcd-daemon.dsc

debian.tar.gz: changelog compat control copyright lcd-daemon.dirs \
lcd-daemon.install README source/format postinst preinst prerm \
rules README
	cd .. && tar cf - debian/changelog debian/compat debian/control \
		debian/copyright debian/lcd-daemon.dirs \
		debian/lcd-daemon.install debian/README debian/source/format \
		debian/rules debian/postinst debian/preinst debian/prerm | gzip -c >debian/$@

