TEMPLATE        = app
CONFIG          += qt release warn_on thread

TARGET    = qtdmm
VERSION   = 0.9.5
DESTDIR   = ../bin

CONFIG += uic3
QT += qt3support

CONFIG += uic3

macx {
  CONFIG += static
  LIBS      = -framework Carbon -framework QuickTime -lz $(QTDIR)/lib/libqt-mt.a
  RC_FILE   = QtDMMIcon.icns
}
UI_DIR=tmp/ui
MOC_DIR=tmp/moc
OBJECTS_DIR=tmp/obj
RCC_DIR=tmp/rcc

QTLFLAGS += -DQT3_SUPPORT

HEADERS   = 
            sources/colorbutton.h \
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
            sources/qcleanuphandler.h \
            sources/readerthread.h \
            sources/readevent.h \
            sources/recorderprefs.h \
            sources/scaleprefs.h \
            sources/simplecfg.h \
            sources/tipdlg.h

FORMS3 =     forms/uiconfigdlg.ui \
             forms/uidmmprefs.ui \
             forms/uiexecuteprefs.ui \
             forms/uigraphprefs.ui \
             forms/uiguiprefs.ui \
             forms/uiintegrationprefs.ui \
             forms/uimainwid.ui \
             forms/uiprintdlg.ui \
             forms/uirecorderprefs.ui \
             forms/uiscaleprefs.ui \
             forms/uitipdlg.ui
                        
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
            sources/readevent.cpp \
            sources/recorderprefs.cpp \
            sources/scaleprefs.cpp \
            sources/simplecfg.cpp \
            sources/tipdlg.cpp

