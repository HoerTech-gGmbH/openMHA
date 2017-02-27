info_TEXINFOS = fftw.texi

fftw_TEXINFOS = fftw.texi version.texi

HTML_FILES = equation-1.gif fftw_1.html fftw_4.html fftw_7.html \
	fftw_foot.html equation-2.gif fftw_2.html fftw_5.html fftw_8.html \
	fftw_toc.html equation-3.gif fftw_3.html fftw_6.html fftw_9.html \
        fftw_10.html equation-4.gif 

EXTRA_DIST = fftw.ps $(HTML_FILES) texi2html rfftwnd.fig rfftwnd.gif
PERL = @PERL@
DVIPS = dvips -Z

html:	$(fftw_TEXINFOS) rfftwnd.gif
	$(PERL) texi2html -split_chapter -no_toc_href fftw.texi

$(HTML_FILES): html

# fig2dev gif output sucks.
# rfftwnd.gif: rfftwnd.fig
# 	fig2dev -L gif rfftwnd.fig rfftwnd.gif

fftw.ps: rfftwnd.eps

rfftwnd.eps: rfftwnd.fig
	fig2dev -L ps -m .7 rfftwnd.fig rfftwnd.eps

clean-local:
	rm -f *.html
