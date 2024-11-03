QT       += core gui
QT       += quick qml
QT       += core multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    App/app.cpp \
    Audio/AudioInput.cpp \
    Audio/AudioOutput.cpp \
    Network/Client.cpp \
    Network/webrtc.cpp \
    SocketIO/internal/sio_client_impl.cpp \
    SocketIO/internal/sio_packet.cpp \
    SocketIO/sio_client.cpp \
    SocketIO/sio_socket.cpp \
    main.cpp \


HEADERS += \
    App/app.h \
    Audio/AudioInput.h \
    Audio/AudioOutput.h \
    Network/Client.h \
    Network/webrtc.h \
    SocketIO/internal/sio_client_impl.h \
    SocketIO/internal/sio_packet.h \
    SocketIO/sio_client.h \
    SocketIO/sio_message.h \
    SocketIO/sio_socket.h

FORMS += \

#Libs

#windows configuration
win32: PATH_TO_OPUS = "F:/Program Files (x86)/MMD.Soor/University/Term 7/CN/CAs/Lib/opus"
win32: PATH_TO_LIBDATACHANNEL = "F:/Program Files (x86)/MMD.Soor/University/Term 7/CN/CAs/Lib/libdatachannel"
win32: PATH_TO_SIO = "F:/Program Files (x86)/MMD.Soor/University/Term 7/CN/CAs/Lib/socket.io-client-cpp"

win32: INCLUDEPATH += $$PATH_TO_LIBDATACHANNEL/include
win32: LIBS += -L$$PATH_TO_LIBDATACHANNEL/Windows/Mingw64 -ldatachannel.dll

win32: LIBS += -LE:/Qt/Tools/OpenSSLv3/Win_x64/bin -lcrypto-3-x64 -lssl-3-x64
win32: INCLUDEPATH += E:/Qt/Tools/OpenSSLv3/Win_x64/include


win32: INCLUDEPATH += $$PATH_TO_OPUS/include
win32: LIBS += -L$$PATH_TO_OPUS/Windows/Mingw64 -lopus

win32: LIBS += -lws2_32
win32: LIBS += -lssp

win32: INCLUDEPATH += $$PATH_TO_SIO/lib/websocketpp
win32: INCLUDEPATH += $$PATH_TO_SIO/lib/asio/asio/include
win32: INCLUDEPATH += $$PATH_TO_SIO/lib/rapidjson/include


win32: DEFINES += ASIO_STANDALONE
win32: DEFINES += _WEBSOCKETPP_CPP11_STL_
win32: DEFINES += _WEBSOCKETPP_CPP11_FUNCTIONAL_
win32: DEFINES += SIO_TLS

#macOS configuration
macx: PATH_TO_LIBDATACHANNEL = "/Users/amirparsamobed/Documents/University/Term 7/Computer Network/Github Repositories/CN_CA1_lib/libdatachannel"
macx: PATH_TO_SIO = "/Users/amirparsamobed/Documents/University/Term 7/Computer Network/Github Repositories/CN_CA1_lib/socket.io-client-cpp"

macx: INCLUDEPATH += $$PATH_TO_LIBDATACHANNEL/include
macx: LIBS += -L$$PATH_TO_LIBDATACHANNEL/build -ldatachannel

macx: LIBS += -L/opt/homebrew/opt/openssl/lib -lssl -lcrypto
macx: INCLUDEPATH += /opt/homebrew/opt/openssl/include

macx: INCLUDEPATH += /usr/local/include/opus
macx: LIBS += -L/usr/local/lib -lopus

macx: LIBS += -lpthread

macx: INCLUDEPATH += $$PATH_TO_SIO/lib/websocketpp
macx: INCLUDEPATH += $$PATH_TO_SIO/lib/asio/asio/include
macx: INCLUDEPATH += $$PATH_TO_SIO/lib/rapidjson/include

macx: INCLUDEPATH += $$PATH_TO_SIO/lib/websocketpp
macx: INCLUDEPATH += $$PATH_TO_SIO/lib/asio/asio/include
macx: INCLUDEPATH += $$PATH_TO_SIO/lib/rapidjson/include

macx: DEFINES += ASIO_STANDALONE
macx: DEFINES += _WEBSOCKETPP_CPP11_STL_
macx: DEFINES += _WEBSOCKETPP_CPP11_FUNCTIONAL_
macx: DEFINES += SIO_TLS

# Default rules for deployment.
QMAKE_CXXFLAGS += -fstack-protector
# QMAKE_LFLAGS += -fuse-ld=lld
CONFIG += no_keywords
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    Main.qml \
    Network/SignalingServer.js \
    UI/Main.qml
