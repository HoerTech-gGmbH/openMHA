# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2005 2006 2007 2011 2013 2016 2017 HörTech gGmbH
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

MHA_CHART_FMT = eps m dia png xfig
MHA_CHART_FILES = $(wildcard $(patsubst %,*.%,$(MHA_CHART_FMT)))
MHA_CHARTS = $(basename $(notdir $(MHA_CHART_FILES))) mha_structure_now delaysum

all: eps png pdf

.PHONY: eps png pdf

clean:
	rm -f *~ fmt_eps/*.eps fmt_png/*.png fmt_pdf/*.pdf fmt_emf/*.emf

eps: $(patsubst %,fmt_eps/%.eps,$(MHA_CHARTS))

png: $(patsubst %,fmt_png/%.png,$(MHA_CHARTS))

pdf: $(patsubst %,fmt_pdf/%.pdf,$(MHA_CHARTS))

emf: $(patsubst %,fmt_emf/%.emf,$(MHA_CHARTS))

fmt_eps/%.eps: %.dia
	dia -n -e $@ $<

fmt_eps/%.eps: %.xfig
	fig2dev -L eps $< $@

fmt_eps/mha_structure.eps: mha_structure.xfig
	fig2dev -L eps -D +40,50,58,60 $< $@

fmt_eps/mha_structure_now.eps: mha_structure.xfig
	fig2dev -L eps -D +50,59,60 $< $@

fmt_eps/adm.eps: adm.xfig
	fig2dev -L eps -D +50,51,60,61 $< $@

fmt_eps/delaysum.eps: adm.xfig
	fig2dev -L eps -D +50,51,70,71 $< $@

NOJVM=$(shell matlab -nodisplay -r "ver;quit" | grep -i "Matlab Version" | grep -q 201 || echo " -nojvm")

fmt_eps/%.eps: %.m
	matlab $(NOJVM) -nodisplay -r "try;$*;catch;end;quit"
	reset || true
	echo ""
	mv $(notdir $@) $@

fmt_eps/%.eps: %.eps
	cp $< $@

fmt_eps/%.eps: %.png
	pngtopnm $< | pnmtops -noturn > $@

fmt_eps/standalone.eps: standalone.xfig \
                        fmt_eps/dia_microphone.eps \
                        fmt_eps/dia_speaker.eps

fmt_png/%.png: %.png
	cp $< $@

fmt_png/%.png: fmt_eps/%.eps
	pstopnm -stdout -xsize 3000 -portrait $< | pnmscale 0.25 | pnmcrop | pnmtopng > $@

fmt_pdf/%.pdf: fmt_eps/%.eps
	epstopdf --nocompress --outfile=$@ $<

fmt_emf/%.emf: fmt_eps/%.eps
	pstoedit -f emf $< $@

fmt_png/%.png: %.dot
	dot -Tpng < $< > $@

# Local Variables:
# coding: utf-8-unix
# End:
