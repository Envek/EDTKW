# -------------------------------------------------
# Project created by QtCreator 2010-05-19T10:52:38
# -------------------------------------------------
TARGET = EDTKW
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp
HEADERS += mainwindow.h
FORMS += mainwindow.ui
RESOURCES += icons.qrc
OTHER_FILES += EDTKW.rc

# Чтобы в Windows у программы была иконка
win32:RC_FILE = EDTKW.rc
