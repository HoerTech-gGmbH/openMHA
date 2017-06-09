# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2005 2006 2007 2008 2012 2013 2016 2017 HörTech gGmbH
#
# openMHA is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, version 3 of the License.
#
# openMHA is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License, version 3 for more details.
#
# You should have received a copy of the GNU Affero General Public License, 
# version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.

include ../../../config.mk
include ../../../magic.mk

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

MHA_manual.pdf: MHA_manual.tex ../openMHAdoxygen.sty $(wildcard *.tex)

libaudprof_manual.pdf: libaudprof_manual.tex ../openMHAdoxygen.sty

mhacontrol_manual.pdf: mhacontrol_manual.tex ../openMHAdoxygen.sty

MHACONTROL_PLUGS = audiometerbackend transducers dc_simple finetuning addsndfile fftfilterbank altplugs

mhacontrol_manual.tex: $(patsubst %,plug_%.tex,$(MHACONTROL_PLUGS))

.PHONY : plug_*.tex

plug_%.tex:
	LD_LIBRARY_PATH=../../libmha/$(BUILD_DIR):"$$LD_LIBRARY_PATH" ../../frameworks/$(BUILD_DIR)/generatemhaplugindoc --ofname=$@ --category-macro='invisible' --plugin-macro='subsubsection' $*

# Local Variables:
# coding: utf-8-unix
# End:
