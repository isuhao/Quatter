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

    pixmap.files = resources/linux/icons/marklar.xpm
    pixmap.path = $$DATADIR/pixmaps

    icon.files = resources/linux/icons/hicolor/*
    icon.path = $$DATADIR/icons/hicolor

    desktop.files = resources/linux/marklar.desktop
    desktop.path = $$DATADIR/applications/

    appdata.files = Resources/*
    appdata.path = $$DATADIR/quatter/

    INSTALLS += target icon pixmap desktop appdata
}
