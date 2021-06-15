.PHONY: exe

exe: clean
	mkdir -p bin src
	cp ../../nalnl2wrapper.exe bin
	cp ../../*.hh ../../*.cpp src
	makensis nalnl2wrapper.nsi
	rm -rf bin src
clean:
	rm -rf bin src
