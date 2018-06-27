#MHAPlugin qmake project generated from template
TEMPLATE = lib

CONFIG -= qt
CONFIG -= app_bundle
CONFIG += console
CONFIG += plugin
CONFIG += no_plugin_name_prefix

SOURCES += %ProjectName%.cpp

HEADERS += %ProjectName%.h

OTHER_FILES += %ProjectName%.cfg %ProjectName%.m

INCLUDEPATH += %IncludePath% %ExtraIncludePath%

LIBS += -L %LibMHAPath% -lopenmha

TARGET = %ProjectName%

debug: DEFINES+=DEBUG

DLLDESTDIR = %InstallPath%

