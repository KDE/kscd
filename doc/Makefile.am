
docs_DATA = index-1.html index-2.html index-3.html index-4.html \
	index-5.html index-6.html index-7.html index-8.html \
        index.html logotp3.png\
	kscd1.png kscd2.png kscd2a.png kscd3.png kscd4.png\
	kscd5.png kscd6.png kscd7.png kscd8.png kscd9.png kscd10.png\
	kscd11.png kscd13.png kscd14.png kscd15.png kscd16.png 

docsdir = $(kde_htmldir)/en/kscd

EXTRA_DIST = $(docs_DATA) index.sgml index.nif

doc: index.sgml index.nif
	ksgml2html index en
