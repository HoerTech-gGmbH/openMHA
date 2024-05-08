.PHONY: install build


#openMHA project directory
REPO=../../../..

include $(REPO)/config.mk

install: usr-local-bin \
         usr-local-lib \
         mfiles \
         matfiles \
         examples \
         reference_algorithms \
         docfiles \
         devfiles

build: 
	rm -rf homebrew-build-dir
	mkdir homebrew-build-dir
	$(MAKE) -C "$(REPO)" "DESTDIR=$$(realpath homebrew-build-dir)" install
	rm -f "homebrew-build-dir/$(PREFIX)/bin/thismha.sh"
	chmod 755 "homebrew-build-dir/$(PREFIX)/bin/"*
	chmod 755 "homebrew-build-dir/$(PREFIX)/lib/"*

usr-local-bin: build
	mkdir -p "$(DESTDIR)$(PREFIX)/bin"
	cp -r "homebrew-build-dir/$(PREFIX)/bin/"* "$(DESTDIR)$(PREFIX)/bin"

usr-local-lib: build
	mkdir -p "$(DESTDIR)$(PREFIX)/lib"
	cp -r "homebrew-build-dir/$(PREFIX)/lib/"* "$(DESTDIR)$(PREFIX)/lib"

examples: build
	mkdir -p "$(DESTDIR)$(PREFIX)/share/openmha/examples"
	cp -r "$(REPO)/examples/"* "$(DESTDIR)$(PREFIX)/share/openmha/examples"

reference_algorithms: build
	mkdir -p "$(DESTDIR)$(PREFIX)/share/openmha/reference_algorithms"
	cp -r "${REPO}/reference_algorithms/"* "$(DESTDIR)$(PREFIX)/share/openmha/reference_algorithms"

mfiles: build
	mkdir -p "$(DESTDIR)$(PREFIX)/lib/openmha/mfiles"
	mkdir -p "$(DESTDIR)$(PREFIX)/lib/openmha/mfiles/mhagui_PHL_generic_hearing_aid_callbacks"
	cp "$(REPO)/mha/tools/mfiles/"*m "$(DESTDIR)$(PREFIX)/lib/openmha/mfiles"
	cp "$(REPO)/mha/tools/mfiles/mhagui_PHL_generic_hearing_aid_callbacks/"*m "$(DESTDIR)$(PREFIX)/lib/openmha/mfiles/mhagui_PHL_generic_hearing_aid_callbacks"
	cp "$(REPO)/mha/tools/mfiles/mhactl_java.jar" "$(DESTDIR)$(PREFIX)/lib/openmha/mfiles"

matfiles: build
	mkdir -p "$(DESTDIR)$(PREFIX)/share/openmha"
	cp "$(REPO)/mha/tools/mfiles/"*.mat "$(DESTDIR)$(PREFIX)/share/openmha/"
	cp "$(REPO)/mha/tools/mfiles/"*.cfg "$(DESTDIR)$(PREFIX)/share/openmha/"

docfiles: build
	mkdir -p "$(DESTDIR)$(PREFIX)/share/doc/openmha/"
	cp "$(REPO)/"COPYING "$(DESTDIR)$(PREFIX)/share/doc/openmha/"
	cp "$(REPO)/"*.md "$(DESTDIR)$(PREFIX)/share/doc/openmha/"
	for d in application_manual calibration_manual developer_manual gui_manual matlab_coder_integration plugins starting_guide; do \
		pdf=openMHA_$$d.pdf; \
		git show origin/gh-pages:docs/$$pdf >"$(DESTDIR)$(PREFIX)/share/doc/openmha/"$$pdf; \
	done

devfiles: build
	mkdir -p "$(DESTDIR)$(PREFIX)/share/openmha"
	cp "$(REPO)/"config.mk "$(DESTDIR)$(PREFIX)/share/openmha/"
	mkdir -p "$(DESTDIR)$(PREFIX)/include/openmha"
	cp "$(REPO)/mha/libmha/src/*.h "$(DESTDIR)$(PREFIX)/include/openmha"
	cp "$(REPO)/mha/libmha/src/*.hh "$(DESTDIR)$(PREFIX)/include/openmha"

clean:
	@rm -rf homebrew-build-dir
