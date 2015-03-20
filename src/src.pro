TEMPLATE        = app
CONFIG          += qt release warn_on thread
QT += qt3support
macx {
  CONFIG += static
}
INCLUDEPATH     = . moc xpm
MOC_DIR         = moc
OBJECTS_DIR     = tmp
DEPENDPATH      = . 
QTLFLAGS += -DQT3_SUPPORT

HEADERS   = \
            colorbutton.h \
            configdlg.h \
            configitem.h \
            displaywid.h \
            dmm.h \
            dmmbar.h \
            dmmgraph.h \
            dmmprefs.h \
            engnumbervalidator.h \
            executeprefs.h \
            graphprefs.h \
            guiprefs.h \
            integrationprefs.h \
            mainwid.h \
            mainwin.h \
            prefwidget.h \
            printdlg.h \
            qcleanuphandler.h \
            readerthread.h \
            readevent.h \
            recorderprefs.h \
            scaleprefs.h \
            simplecfg.h \
            tipdlg.h

#The following line was changed from FORMS to FORMS3 by qt3to4
FORMS3 = \
             uiconfigdlg.ui \
             uidmmprefs.ui \
             uiexecuteprefs.ui \
             uigraphprefs.ui \
             uiguiprefs.ui \
             uiintegrationprefs.ui \
             uimainwid.ui \
             uiprintdlg.ui \
             uirecorderprefs.ui \
             uiscaleprefs.ui \
             uitipdlg.ui
                        
SOURCES   = \
            colorbutton.cpp \
            configdlg.cpp \
            configitem.cpp \
            displaywid.cpp \
            dmm.cpp \
            dmmbar.cpp \
            dmmgraph.cpp \
            dmmprefs.cpp \
            engnumbervalidator.cpp \
            executeprefs.cpp \
            graphprefs.cpp \
            guiprefs.cpp \
            integrationprefs.cpp \
            main.cpp \
            mainwid.cpp \
            mainwin.cpp \
            prefwidget.cpp \
            printdlg.cpp \
            readerthread.cpp \
            readevent.cpp \
            recorderprefs.cpp \
            scaleprefs.cpp \
            simplecfg.cpp \
            tipdlg.cpp

macx {
  LIBS      = -framework Carbon -framework QuickTime -lz $(QTDIR)/lib/libqt-mt.a
  RC_FILE   = QtDMMIcon.icns
}
TARGET    = qtdmm
VERSION   = 0.9.0
DESTDIR   = ../bin

#The following line was inserted by qt3to4
QT +=  
#The following line was inserted by qt3to4
CONFIG += uic3

