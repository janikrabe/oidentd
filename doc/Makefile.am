man_MANS = \
	oidentd.8 \
	oidentd.conf.5 \
	oidentd_masq.conf.5

DISTCLEANFILES = \
	oidentd.8 \
	oidentd.conf.5 \
	oidentd_masq.conf.5

EXTRA_DIST = \
	$(man_MANS) \
	oidentd.8.adoc \
	oidentd.conf.5.adoc \
	oidentd_masq.conf.5.adoc \
	rfc1413

SUFFIXES = .adoc

oidentd.8: oidentd.8.adoc

oidentd.conf.5: oidentd.conf.5.adoc

oidentd_masq.conf.5: oidentd.conf.5.adoc

.adoc:
	asciidoctor \
		--backend manpage \
		--doctype manpage \
		-a "sysconfdir=$(sysconfdir)" \
		-a "mansource=@PACKAGE_STRING@" \
		$<
