#-------------------------------------------------
#
# Project created by QtCreator 2018-01-04T20:32:36
#
#-------------------------------------------------

QT += core gui
QT += printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = "TikZ Previewer"
TEMPLATE = app
ICON = ../development/icon/icon_magnifying_glass.icns

#
# Path to poppler.
#

INCLUDEPATH += /sw/include/poppler/qt5
LIBS        += -L/sw/lib

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#
# Pre-build actions.
#

# Generate icon (it is assumed that the build machine has pdflatex and sundry
# image processing utilities).
system( cd ../development/icon && ./make_icon.sh )

# Collect list of contributors.
system( git log | grep Author: | uniq | sort | awk \'{for (i=2; i<NF; i++) printf $i \" \"; print $NF}\' > ../CONTRIBUTORS.md)

LIBS += -lpoppler-qt5

SOURCES += \
        main.cpp \
        tikz_previewer.cpp \
    logger.cpp \
    third-party/pugixml/pugixml.cpp \
    preferences.cpp \
    qgraphicsview_gesture.cpp

HEADERS += \
        tikz_previewer.h \
    logger.h \
    third-party/pugixml/pugiconfig.hpp \
    third-party/pugixml/pugixml.hpp \
    preferences.h \
    defined_values.h \
    qgraphicsview_gesture.h

FORMS += \
        tikz_previewer.ui

DISTFILES += \
    ../development/icon/icon_magnifying_glass.tex.pdf \
    ../development/icon/make_icon.sh \
    ../development/setup_dylibs_for_deployment.sh \
    ../development/icon/icon_magnifying_glass.tex

RESOURCES += \
    icons.qrc
