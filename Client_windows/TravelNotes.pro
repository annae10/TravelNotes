QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    worker.cpp

HEADERS += \
    mainwindow.h \
    worker.h

FORMS += \
    mainwindow.ui

INCLUDEPATH+=C:\OpenSSL-Win64\include
#LIBS += -LC:\OpenSSL-Win64 -lssl -lcrypto -lws2_32

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


CONFIG(release, debug|release):{
    LIBS+=-L"C:/OpenSSL-Win64/lib/VC/x64/MD" -llibssl -llibcrypto -lws2_32
}
CONFIG(debug, debug|release):{
    LIBS+=-L"C:/OpenSSL-Win64/lib/VC/x64/MDd" -llibssl -llibcrypto -lws2_32
}


