all: structure_openmha.pdf structure_openmha.png


structure_openmha.pdf: structure_openmha.dot
	dot -Tpdf $< > $@

structure_openmha.png: structure_openmha.dot
	dot -Tpng $< > $@
