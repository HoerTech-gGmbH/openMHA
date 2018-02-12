#MHAPlugin qmake project generated from template
TEMPLATE = lib

CONFIG -= qt
CONFIG -= app_bundle
CONFIG += console
CONFIG += plugin
CONFIG += no_plugin_name_prefix

SOURCES += acPooling_wave.cpp

HEADERS += acPooling_wave.h

OTHER_FILES += acPooling_wave.cfg acPooling_wave.m

INCLUDEPATH += /home/genckamil/Work/MHA/Repository/mha/mha/libmha/src

LIBS += -L /home/genckamil/Work/MHA/Repository/mha/mha/libmha/x86_64-linux-gcc-4.8 -lMHAToolbox

TARGET = acPooling_wave

debug: DEFINES+=DEBUG

DLLDESTDIR = 


