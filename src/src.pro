QT       += core gui widgets printsupport serialport
CONFIG   += c++14 no_keywords

TEMPLATE        = app

TARGET    = qtdmm
VERSION   = 0.9.5
DESTDIR   = ../bin

UI_DIR=../tmp/ui
MOC_DIR=../tmp/moc
OBJECTS_DIR=../tmp/obj
RCC_DIR=../tmp/rcc

TRANSLATIONS    =  translations/qtdmm_de.ts

QMAKE_EXTRA_COMPILERS += lrelease
lrelease.input         = TRANSLATIONS
lrelease.output        = ${QMAKE_FILE_BASE}.qm
lrelease.commands      = $$[QT_INSTALL_BINS]/lrelease-qt5 ${QMAKE_FILE_IN} -qm translations/${QMAKE_FILE_BASE}.qm
lrelease.CONFIG       += no_link target_predeps

FORMS =      forms/uiconfigdlg.ui \
	     forms/uidmmprefs.ui \
	     forms/uiexecuteprefs.ui \
	     forms/uigraphprefs.ui \
	     forms/uiguiprefs.ui \
	     forms/uiintegrationprefs.ui \
	     forms/uimainwid.ui \
	     forms/uiprintdlg.ui \
	     forms/uirecorderprefs.ui \
	     forms/uiscaleprefs.ui \
	     forms/uitipdlg.ui \
	     forms/uimainwin.ui

HEADERS   = sources/colorbutton.h \
	    sources/configdlg.h \
	    sources/configitem.h \
	    sources/displaywid.h \
	    sources/dmm.h \
	    sources/dmmbar.h \
	    sources/dmmgraph.h \
	    sources/dmmprefs.h \
	    sources/engnumbervalidator.h \
	    sources/executeprefs.h \
	    sources/graphprefs.h \
	    sources/guiprefs.h \
	    sources/integrationprefs.h \
	    sources/mainwid.h \
	    sources/mainwin.h \
	    sources/prefwidget.h \
	    sources/printdlg.h \
	    sources/readerthread.h \
	    sources/readevent.h \
	    sources/recorderprefs.h \
	    sources/scaleprefs.h \
	    sources/tipdlg.h \
	    sources/Settings.h \
	    sources/config.h

SOURCES   = sources/colorbutton.cpp \
	    sources/configdlg.cpp \
	    sources/configitem.cpp \
	    sources/displaywid.cpp \
	    sources/dmm.cpp \
	    sources/dmmbar.cpp \
	    sources/dmmgraph.cpp \
	    sources/dmmprefs.cpp \
	    sources/engnumbervalidator.cpp \
	    sources/executeprefs.cpp \
	    sources/graphprefs.cpp \
	    sources/guiprefs.cpp \
	    sources/integrationprefs.cpp \
	    sources/main.cpp \
	    sources/mainwid.cpp \
	    sources/mainwin.cpp \
	    sources/prefwidget.cpp \
	    sources/printdlg.cpp \
	    sources/readerthread.cpp \
	    sources/recorderprefs.cpp \
	    sources/scaleprefs.cpp \
	    sources/tipdlg.cpp \
	    sources/Settings.cpp

INCLUDEPATH +=sources

RESOURCES += \
    Resources.qrc

OTHER_FILES += ../QtDMM.spec \
	       ../QtDMM.desktop \
	../CHANGELOG
