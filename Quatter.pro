TARGET = quatter

LIBS += ../Quatter/Urho3D/lib/libUrho3D.a \
    -lpthread \
    -ldl \
    -lGL

DEFINES += URHO3D_COMPILE_QT

QMAKE_CXXFLAGS += -std=c++1y

INCLUDEPATH += \
    ../Quatter/Urho3D/include \
    ../Quatter/Urho3D/include/Urho3D/ThirdParty \

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    luckey.cpp \
    mastercontrol.cpp \
    inputmaster.cpp \
    quattercam.cpp \
    board.cpp \
    piece.cpp \
    master.cpp \
    effectmaster.cpp

HEADERS += \
    luckey.h \
    mastercontrol.h \
    inputmaster.h \
    quattercam.h \
    board.h \
    piece.h \
    master.h \
    effectmaster.h

unix {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
    isEmpty(BINDIR) {
        BINDIR = $$PREFIX/bin
    }
    isEmpty(DATADIR) {
        DATADIR = $$PREFIX/share
    }
    DEFINES += DATADIR=\\\"$${DATADIR}/quatter\\\"

    target.path = $$BINDIR

    icon.files = quatter.svg
    icon.path = /usr/share/icons/hicolor/scalable/apps/

    pixmap.files = Resources/*
    pixmap.path = $$DATADIR/quatter/

    desktop.files = quatter.desktop
    desktop.path = $$DATADIR/applications/

    appdata.files = quatter.appdata.xml
    appdata.path = $$DATADIR/appdata/

    INSTALLS += target icon pixmap desktop appdata
}
