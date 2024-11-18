QT += core
QT += serialbus

TEMPLATE = lib
CONFIG += plugin

CONFIG += c++20

win32-g++:              CONFIG(debug,   release | debug)      :DESTDIR = C:/Qt/$${QT_VERSION}/mingw_64/plugins/canbus
else:win32-g++:         CONFIG(release, release | debug)      :DESTDIR = C:/Qt/$${QT_VERSION}/mingw_64/plugins/canbus
else:win32:!win32-g++:  CONFIG(debug,   release | debug)      :DESTDIR = C:/Qt/$${QT_VERSION}/msvc2019_64/plugins/canbus
else:win32:!win32-g++:  CONFIG(release, release | debug)      :DESTDIR = C:/Qt/$${QT_VERSION}/msvc2019_64/plugins/canbus

SOURCES += \
    CHAI_CAN.cpp \
    ChaiLibWraps.cpp

HEADERS += \
    CHAICanBusPlugin.h \
    CHAI_CAN.h \
    ChaiLibWraps.h

DISTFILES += CHAI_CAN.json

# Default rules for deployment.
unix
{
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += $$PWD/CHAI_2_14_0/include
DEPENDPATH  += $$PWD/CHAI_2_14_0/include

win32:!win32-g++:
{
    !contains(QMAKE_TARGET.arch, x86_64)
    {
        LIBS            += -L$$PWD/CHAI_2_14_0/x64/ -lchai
        PRE_TARGETDEPS  += $$PWD/CHAI_2_14_0/x32/chai.lib
    }

    contains(QMAKE_TARGET.arch, x86_64)
    {
        LIBS            += -L$$PWD/CHAI_2_14_0/x64/ -lchai
        PRE_TARGETDEPS  += $$PWD/CHAI_2_14_0/x64/chai.lib
    }
}
