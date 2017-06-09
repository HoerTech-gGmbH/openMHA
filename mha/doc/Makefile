# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2002 2004 2005 2006 2007 2008 2009 2010 2011 2012 2013 
# Copyright © 2016 2017 HörTech gGmbH
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
    ifeq ($(LD_LIBRARY_PATH),)
       LD_LIBRARY_PATH := ../libmha/$(BUILD_DIR)/
    else
       LD_LIBRARY_PATH := $(LD_LIBRARY_PATH):$(MHA_INSTALL_DIR)
       export LD_LIBRARY_PATH
    endif 
endif


all:
	$(MAKE) -C flowcharts
	$(MAKE)  MHA_reference.pdf MHA_manual.pdf mhacontrol_manual.pdf
	$(MAKE)  plugins


.PHONY : test images kernel parser plugins mhadoc MHA_plugin_auto_doc.pdf clean

# Don't delete intermediate files
.SECONDARY: 

empty :=
space := $(empty) $(empty)
PLUGINDIRS=$(wildcard ../plugins/*/*/$(PLATFORM_CC)/)
MHA_PLUGINDIRS=$(subst $(space),;,$(PLUGINDIRS))

# Get plugin names
PLUGIN_NAMES:=$(subst $(space),"\n",$(foreach plg,$(PLUGINDIRS),$(word 4,$(subst /,$(space),$(plg)))))

MHADISTS = openMHA

test:
	make -C src all


mhakernel.pdf: kernel
	make -C mhakernel/latex refman.pdf
	cp mhakernel/latex/refman.pdf $@

mhastructure.pdf: mhastructure
	TEXINPUTS="./:../../:../../flowcharts/fmt_png/:" make -C MHA-Structure/latex refman.pdf
	cp MHA-Structure/latex/refman.pdf $@

MHA_reference.pdf: images mhadoc
	cp optparams.sty mhadoc/latex
	cp flowcharts/fmt_png/*.png mhadoc/latex
	cp flowcharts/fmt_pdf/*.pdf mhadoc/latex
	make -C mhadoc/latex refman.pdf
	cp mhadoc/latex/refman.pdf $@

MHA_manual.pdf: user_manual/MHA_manual.pdf
	cp $< $@

mhacontrol_manual.pdf: user_manual/mhacontrol_manual.pdf
	cp $< $@

.PHONY : images user_manual/MHA_manual.pdf user_manual/mhacontrol_manual.pdf

user_manual/MHA_manual.pdf:
	make -C user_manual MHA_manual.pdf

user_manual/mhacontrol_manual.pdf:
	make -C user_manual mhacontrol_manual.pdf

images:
	make -C flowcharts

kernel: images
	rm -Rf mhakernel/latex
	rm -Rf mhakernel/html
	doxygen mhakernel.cfg

mhadoc: images
	rm -Rf mhadoc
	doxygen mhaextern.cfg
	cp hoertechlogo.png mhadoc/html/

mhastructure: images 
	rm -Rf MHA-Structure/latex
	doxygen MHA-Structure.cfg

parser:
	doxygen mhaparser.cfg

plugins: $(MHADISTS:=_plugins.pdf)
	doxygen mhaplugins.cfg

MHA_plugin_auto_doc.pdf:
	rm -f MHA_plugin_auto_doc.*
	MHA_PARSER_PRECISION=6 ./create_doc_for_all_plugins.sh

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

%.pdf: %.tex
	TEXINPUTS=flowcharts/fmt_png::. pdflatex -file-line-error $<
	-bibtex $*
	TEXINPUTS=flowcharts/fmt_png::. pdflatex -file-line-error $<
	makeindex $*
	TEXINPUTS=flowcharts/fmt_png::. pdflatex -file-line-error $<
	rm -f $*.{aux,dvi,log,toc,out,idx,ilg}

clean:
	rm -f $(MHADISTS:=_plugins.*) *~

# Local Variables:
# coding: utf-8-unix
# End:
