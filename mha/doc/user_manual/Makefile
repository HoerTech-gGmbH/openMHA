all: MHA_manual.pdf MHA_writing_fitting_rules.pdf libaudprof_manual.pdf

#TEXINPUTS = "./:../:./plugins/:../technical_report/:../flowcharts/fmt_eps/:../flowcharts/fmt_png/:../flowcharts/fmt_pdf/:"
TEXINPUTS = "./:../:./plugins/:../technical_report/:../flowcharts/fmt_eps/:../flowcharts/fmt_pdf/:../flowcharts/:"

%.pdf: %.tex
	TEXINPUTS=$(TEXINPUTS) pdflatex $<
	TEXINPUTS=$(TEXINPUTS) pdflatex $<
	TEXINPUTS=$(TEXINPUTS) makeindex $(basename $<)
	TEXINPUTS=$(TEXINPUTS) pdflatex $<

%.aux: %.tex
	TEXINPUTS=$(TEXINPUTS) pdflatex $<

clean:
	rm -Rf *.log *.lot *.lof *.aux *.dep *.dvi *.toc *~ *.blg *.bbl MHA_manual.pdf plug_*.tex

MHA_manual.pdf: MHA_manual.tex ../mha.sty $(wildcard *.tex)

libaudprof_manual.pdf: libaudprof_manual.tex ../mha.sty

mhacontrol_manual.pdf: mhacontrol_manual.tex ../mha.sty

MHACONTROL_PLUGS = audiometerbackend transducers dc_simple finetuning addsndfile fftfilterbank altplugs

mhacontrol_manual.tex: $(patsubst %,plug_%.tex,$(MHACONTROL_PLUGS))

.PHONY : plug_*.tex

plug_%.tex:
	generatemhaplugindoc --ofname=$@ --category-macro='invisible' --plugin-macro='subsubsection' $*
