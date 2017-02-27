all:
	@echo converting 1 ...
	perl bfnnconv.pl fftw-faq.bfnn
	@echo converting 2 ...
	perl bfnnconv.pl fftw-faq.bfnn

install-html: all
	cp fftw-faq.html/* $(HOME)/WWW/faq/

clean:
	rm -f *~ core a.out *.lout *.ps *.info *.ascii *.xrefdb *.post
	rm -rf *.html
