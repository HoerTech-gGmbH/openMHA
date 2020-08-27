# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2002 2004 2005 2006 2007 2008 2009 2010 2011 2012 2013 
# Copyright © 2016 2017 2018 2019 2020 HörTech gGmbH
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

include ../../config.mk
include ../../magic.mk

ifeq (linux,$(PLATFORM))
    LD_LIBRARY_PATH := $(LD_LIBRARY_PATH):../libmha/$(BUILD_DIR)/
    export LD_LIBRARY_PATH
endif

MHANAME = openMHA

all: $(MHANAME)_developer_manual.pdf $(MHANAME)_application_manual.pdf $(MHANAME)_gui_manual.pdf $(MHANAME)_calibration_manual.pdf $(MHANAME)_starting_guide.pdf plugins

.PHONY : images plugins mhadoc clean

# Don't delete intermediate files
.SECONDARY: 

empty :=
space := $(empty) $(empty)
PLUGINDIRS=$(wildcard ../plugins/*/$(PLATFORM_CC)/)
MHA_PLUGINDIRS=$(subst $(space),;,$(PLUGINDIRS))

# Get plugin names
PLUGIN_NAMES:=$(subst $(space),"\n",$(filter-out io,$(foreach plg,$(PLUGINDIRS),$(word 3,$(subst /,$(space),$(plg))))))

MHADISTS = openMHA

$(MHANAME)_developer_manual.pdf: images mhadoc
	cp -lv --remove-destination optparams.sty mhadoc/latex
	cp -lv --remove-destination flowcharts/fmt_png/*.png mhadoc/latex
	cp -lv --remove-destination flowcharts/fmt_pdf/*.pdf mhadoc/latex
	$(MAKE) -C mhadoc/latex refman.pdf
	cp -lv --remove-destination mhadoc/latex/refman.pdf $@

$(MHANAME)_application_manual.pdf: user_manual/$(MHANAME)_application_manual.pdf
	cp -lv --remove-destination $< $@

$(MHANAME)_calibration_manual.pdf: calibration/$(MHANAME)_calibration_manual.pdf
	cp -lv --remove-destination $< $@

$(MHANAME)_gui_manual.pdf: user_manual/$(MHANAME)_gui_manual.pdf
	cp -lv --remove-destination $< $@

$(MHANAME)_starting_guide.pdf: starting_guide/$(MHANAME)_starting_guide.pdf
	cp -lv --remove-destination $< $@

.PHONY : images user_manual/$(MHANAME)_application_manual.pdf user_manual/$(MHANAME)_gui_manual.pdf

user_manual/$(MHANAME)_application_manual.pdf: images mhadoc
	$(MAKE) -C user_manual $(MHANAME)_application_manual.pdf

user_manual/$(MHANAME)_gui_manual.pdf: images mhadoc
	$(MAKE) -C user_manual $(MHANAME)_gui_manual.pdf

calibration/$(MHANAME)_calibration_manual.pdf: images mhadoc
	$(MAKE) -C calibration $(MHANAME)_calibration_manual.pdf

starting_guide/$(MHANAME)_starting_guide.pdf: images mhadoc
	$(MAKE) -C starting_guide $(MHANAME)_starting_guide.pdf

images:
	$(MAKE) -C images
	$(MAKE) -C flowcharts

mhadoc: images
	rm -Rf mhadoc
	doxygen mhaextern.cfg
	cp -v hoertechlogo.png mhadoc/html/

plugins: $(MHADISTS:=_plugins.pdf)

%_plugin.list: 
	echo $(PLUGIN_NAMES) | sort > $@

%_plugin_section.tex: %_plugin.list
	MHA_PARSER_PRECISION=5 LD_LIBRARY_PATH="$(LD_LIBRARY_PATH):$(MATLAB_LIBS)" MHA_LIBRARY_PATH="$(MHA_PLUGINDIRS)" ../frameworks/$(BUILD_DIR)/generatemhaplugindoc -o $@ -c subsection -p subsubsection `cat $<` 

%_plugins.tex: %_plugin.list
	MHA_PARSER_PRECISION=5 LD_LIBRARY_PATH="$(LD_LIBRARY_PATH):$(MATLAB_LIBS)" MHA_LIBRARY_PATH="$(MHA_PLUGINDIRS)" ../frameworks/$(BUILD_DIR)/generatemhaplugindoc -o temp$@ -c section -p subsection `cat $<` 
	cat plugin_doc_prefix.tex | sed -e 's/__DISTID__/$*/g' > $@
	cat temp$@ >> $@
	cat plugin_doc_postfix.tex >> $@
	rm -f temp$@

%.pdf: %.tex images
	TEXINPUTS=flowcharts/fmt_png::. pdflatex -file-line-error $<
	-bibtex $*
	TEXINPUTS=flowcharts/fmt_png::. pdflatex -file-line-error $<
	makeindex $*
	TEXINPUTS=flowcharts/fmt_png::. pdflatex -file-line-error $<
	rm -f $*.{aux,dvi,log,toc,out,idx,ilg}

clean:
	rm -f $(MHADISTS:=_plugins.*) *~ $(MHANAME)_developer_manual.pdf $(MHANAME)_application_manual.pdf $(MHANAME)_gui_manual.pdf
	$(MAKE) -C user_manual clean

# Local Variables:
# coding: utf-8-unix
# End:
