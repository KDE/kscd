
docs_DATA = index-1.html index-2.html index-3.html index-4.html \
	index-5.html index-6.html index-7.html index-8.html \
        index.html logotp3.gif\
	kscd1.gif kscd2.gif kscd2a.gif kscd3.gif kscd4.gif\
	kscd5.gif kscd6.gif kscd7.gif kscd8.gif kscd9.gif kscd10.gif\
	kscd11.gif kscd13.gif kscd14.gif kscd15.gif kscd16.gif 

docsdir = $(kde_htmldir)/en/kscd

EXTRA_DIST = $(docs_DATA) index.sgml index.nif

doc: index.sgml index.nif
	ksgml2html index en
