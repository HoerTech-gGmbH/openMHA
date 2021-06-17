# Processor architecture: x86_64, i686, armv7, aarch64
ARCH=$(shell uname -m)
# Debian processor architecture: amd64, i386, armhf, arm64
DEBARCH=$(shell dpkg-architecture --query DEB_BUILD_ARCH)
# Operating System: MinGW, Linux, Darwin
PLATFORM=$(shell uname -s | sed -e s/Linux/linux/ -e "s/MINGW.*/MinGW/")
# Version number of default GCC compiler, prepended with a dash (e.g. -9)
GCCVER=$(shell gcc -dumpversion | head -1|sed -e 's/.*[[:blank:]]//' -e 's/\([0-9]*\)\.\([0-9]*\)\.\([0-9]*\)/-\1/' -e 's/^[0-9]/-&/1')
# Build directory without and with gcc version number
BUILD_DIR_SANS_VER=$(ARCH)-$(PLATFORM)-gcc
BUILD_DIR=$(BUILD_DIR_SANS_VER)$(GCCVER)
# Directory for storage of shared libraries
LIB_DIR=$(BUILD_DIR)/lib
# Linker flags
LDLIBS=-lDSLmio-core
LDFLAGS=-L$(LIB_DIR)
# Debian package version
VERSION=${shell cat version}-${shell lsb_release -sr}
# CODENAME is the name of the debian or ubuntu distribution on which this
# Makefile executes and for which the debian packages are created.
# CODENAME is e.g. "bionic" for ubuntu 18.04 or "focal" for ubuntu 20.04
CODENAME=$(shell lsb_release -sc)

export ARCH
export LIB_DIR
export VERSION

debs/$(CODENAME)/dsl-wrapper_$(VERSION)_$(DEBARCH).deb: dsl-wrapper.csv dsl-wrapper.control dsl-wrapper.dep version $(LIB_DIR)/dsl.oct
	mhamakedeb $< $(VERSION)
	mkdir -p debs/$(CODENAME)
	mv *.deb debs/$(CODENAME)

$(LIB_DIR)/dsl.oct: dsl_wrap.cpp $(BUILD_DIR)/.directory
	mkoctfile -I$(BUILD_DIR)/include/ -o $@ $< $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/.directory:
	rm -f $(BUILD_DIR)
	ln -s $(BUILD_DIR_SANS_VER) $(BUILD_DIR)
	touch $@

%_wrap.cpp: %.i Makefile
	swig -octave -o $@ $<

# Debian package management by Jenkins:
# New Debian Packages are stored in our storage for debian repositories.
# The storage is cleaned of old packages depending on the current branch.
#
# Glossary:
#
# supply:
# The packages stored here have just been built. Location is
# project-specific. For openMHA, it is ./mha/tools/packaging/deb/hoertech/$SYSTEM/
# regardless of $BRANCH_NAME.
# The packages here may be new or they may be rebuilds of
# existing versions (e.g. if someone clicks build-now while
# there is no new revision). Rebuilds of existing versions
# will not be used, but will cause an update of the timestamp of the respective
# files in storage.  New packages are copied to storage
#
# storage:
# The packages here are kept persistently across builds.
# Old packets (timestamp older than some threshold) will be deleted for
# branch development.
# Storage location:
# /var/lib/jenkins/packages-for-aptly/STORAGE/$PROJECT/$BRANCH_NAME/$SYSTEM/
# on the host, which is mounted to /STORAGE/$PROJECT/$BRANCH_NAME/$SYSTEM/
# in the container.
#
#
# $PROJECT:
# openMHA, liblsl, liblsl-matlab, tascarpro, more may be added
#
# $BRANCH_NAME:
# master, uploaded to apt.hoertech.de, and development, uploaded to
# aptdev.hoertech.de. BRANCH_NAME is set by Jenkins
#
# SYSTEMs:
# xenial, bionic, jessie, etc. Available SYSTEMs that
# contain packages are detected automatically with $(wildcard)

# There will by $SYSTEM subdirectories below this directory.
# These subdirectories then contain the package files.
SUPPLY_DIR = debs/

PROJECT = dsl-wrapper

# There will be $SYSTEM subdirectories below this directory.
STORAGE_DIR = /STORAGE/$(PROJECT)/$(BRANCH_NAME)/

# How many days to keep debian packages in storage that are superceded by a
# newer version
RETENTION = 14

storage: pruned-storage-$(BRANCH_NAME)

# Delete debian packages in storage older than RETENTION days
pruned-storage-%: updated-storage-%
	@echo uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu
	@echo Begin pruning storage...
	@echo nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn
	find $(STORAGE_DIR) -name "*.deb" -type f -mtime +$(RETENTION) -delete -print
	-rmdir $(STORAGE_DIR)/*   #  delete empty subdirs if there are any
	@echo uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu
	@echo Storage pruning finished.
	@echo nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn

# Never delete old packages in the master database
pruned-storage-master: updated-storage-master
	@echo uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu
	@echo "Keep all existing packages on branch master"
	@echo nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn

# copy the $SYSTEM/packages.deb tree without overwriting existing package files.
# then, update the timestamps for all files that are now in the supply to
# prevent deletion of latest files because they might be too old.
updated-storage-$(BRANCH_NAME):
	@echo uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu
	@echo Begin updating storage...
	@echo nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn
	mkdir -p $(STORAGE_DIR)
	cp -anv $(SUPPLY_DIR)* $(STORAGE_DIR)
	cd $(SUPPLY_DIR) && find . -name \*.deb -exec touch $(STORAGE_DIR){} \;
	@echo uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu
	@echo Storage update finished.
	@echo nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn


# Local Variables:
# coding: utf-8-unix
# End:
