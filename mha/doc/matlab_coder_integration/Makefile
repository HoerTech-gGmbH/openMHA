# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2021 HörTech gGmbH
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

MHANAME=openMHA

all: $(MHANAME)_matlab_coder_integration.pdf

TEXINPUTS = ""

%.pdf: %.tex
	TEXINPUTS=$(TEXINPUTS) pdflatex $<
	TEXINPUTS=$(TEXINPUTS) pdflatex $<
	TEXINPUTS=$(TEXINPUTS) makeindex $(basename $<)
	TEXINPUTS=$(TEXINPUTS) pdflatex $<

%.aux: %.tex
	TEXINPUTS=$(TEXINPUTS) pdflatex $<

clean:
	rm -Rf *.log *.lot *.lof *.aux *.dep *.dvi *.toc *~ *.blg *.bbl *.ilg *.ind *.idx *.out \
  $(MHANAME)_application_manual.pdf $(MHANAME)_gui_manual.pdf plug_*.tex

$(MHANAME)_matlab_coder_integration: $(MHANAME)_matlab_coder_integration.tex ../openMHAdoxygen.sty $(wildcard *.tex)

# Local Variables:
# coding: utf-8-unix
# End:
