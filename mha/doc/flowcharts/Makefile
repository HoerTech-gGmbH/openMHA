ALLXFIGFILES = $(shell echo *.xfig)

all: $(ALLXFIGFILES:.xfig=.png) $(ALLXFIGFILES:.xfig=.pdf)

%.png: %.xfig
	fig2dev -L png $< $@

%.pdf: %.xfig
	fig2dev -L pdf $< $@
