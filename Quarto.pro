LIBS += ../Urho3DQtemplate/Urho3D/lib/libUrho3D.a \
    -lpthread \
    -ldl \
    -lGL

DEFINES += URHO3D_COMPILE_QT

QMAKE_CXXFLAGS += -std=c++1y

INCLUDEPATH += \
    ../Urho3DQtemplate/Urho3D/include \
    ../Urho3DQtemplate/Urho3D/include/Urho3D/ThirdParty \

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    luckey.cpp \
    mastercontrol.cpp \
    inputmaster.cpp \
    templatecam.cpp \
    urho.cpp

HEADERS += \
    luckey.h \
    mastercontrol.h \
    inputmaster.h \
    templatecam.h \
    urho.h