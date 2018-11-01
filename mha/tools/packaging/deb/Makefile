ALL_CSV=$(wildcard *.csv)

CODENAME=$(shell lsb_release -sc)

REPO=../../../..

all: build pack

include $(REPO)/config.mk

GITMODIFIED=$(shell test -z "`cd $(REPO) && git status --porcelain -uno`" || echo "-modified")
COMMITHASH=$(shell cd $(REPO) && git log -1 --abbrev=7 --pretty='format:%h')
PACKAGEVER=$(shell git log --abbrev=7 --pretty='format:%h' | wc -l)
FULLVERSION=$(shell (echo "\#include \"$(REPO)/mha/libmha/src/mha.h\""; echo 'mhaversion=MHA_VERSION_MAJOR  MHA_VERSION_MINOR  MHA_VERSION_RELEASE' )| cpp | grep -e 'mhaversion='|sed -e 's/mhaversion=//1' -e 's/ /./g' )-$(PACKAGEVER)-$(COMMITHASH)$(GITMODIFIED)-gcc$(GCC_VER)
CURRENTPATH=$(shell pwd)

build:
	(cd $(REPO) && make all)

pack:
	rm -Rf hoertech
	mkdir -p hoertech/$(CODENAME)
	for c in $(ALL_CSV); do (cd $(REPO) && BUILD_DIR=$(BUILD_DIR) mhamakedeb $(CURRENTPATH)/$$c $(FULLVERSION) ); done
	mv $(REPO)/*.deb hoertech/$(CODENAME)
