#TCM_CMD_EPS = t$(subst .,,$(suffix $<)) -toEPS $@ $<

MHA_CHART_FMT = eps m dia png xfig
MHA_CHART_FILES = $(wildcard $(patsubst %,*.%,$(MHA_CHART_FMT)))
MHA_CHARTS = $(basename $(notdir $(MHA_CHART_FILES))) mha_structure_now delaysum

all: dirs eps png pdf

.PHONY: dirs eps png pdf

clean:
	rm -f *~ fmt_eps/*.eps fmt_png/*.png fmt_pdf/*.pdf fmt_emf/*.emf

dirs:
	mkdir -p fmt_eps fmt_png fmt_pdf

eps: $(patsubst %,fmt_eps/%.eps,$(MHA_CHARTS))

png: $(patsubst %,fmt_png/%.png,$(MHA_CHARTS))

pdf: $(patsubst %,fmt_pdf/%.pdf,$(MHA_CHARTS))

emf: $(patsubst %,fmt_emf/%.emf,$(MHA_CHARTS))

#fmt_eps/%.eps: %.gt
#	$(TCM_CMD_EPS)
#
#fmt_eps/%.eps: %.esd
#	$(TCM_CMD_EPS)
#
#fmt_eps/%.eps: %.efd
#	$(TCM_CMD_EPS)
#
#fmt_eps/%.eps: %.scd
#	$(TCM_CMD_EPS)
#
#fmt_eps/%.eps: %.std
#	$(TCM_CMD_EPS)
#
#fmt_eps/%.eps: %.gd
#	$(TCM_CMD_EPS)

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

fmt_png/%.png: fmt_eps/%.eps
	pstopnm -stdout -xsize 3000 -portrait $< | pnmscale 0.25 | pnmcrop | pnmtopng > $@

fmt_png/%.png: %.png
	cp $< $@

fmt_pdf/%.pdf: fmt_eps/%.eps
	epstopdf --nocompress --outfile=$@ $<

fmt_emf/%.emf: fmt_eps/%.eps
	pstoedit -f emf $< $@

fmt_png/%.png: %.dot
	dot -Tpng < $< > $@
