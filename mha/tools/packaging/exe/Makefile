.PHONY: exe

exe:
	./find_and_copy_dependencies.sh
	makensis mha.nsi
	rm -rf LICENSE bin mfiles examples doc
clean:
	rm -rf LICENSE bin mfiles examples doc tmp* *exe

# Local Variables:
# compile-command: "make"
# End:
