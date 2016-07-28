#--------------------------------------------------------------------------------------------------------------
#
# @file     disp3D.pro
# @author   Lorenz Esch <Lorenz.Esch@tu-ilmenauu.de>;
#           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
# @version  1.0
# @date     November, 2015
#
# @section  LICENSE
#
# Copyright (C) 2015, Lorenz Esch and Matti Hamalainen. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that
# the following conditions are met:
#     * Redistributions of source code must retain the above copyright notice, this list of conditions and the
#       following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
#       the following disclaimer in the documentation and/or other materials provided with the distribution.
#     * Neither the name of MNE-CPP authors nor the names of its contributors may be used
#       to endorse or promote products derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
# PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
#
# @brief    This project file builds the new display 3D library which depends on the qt3d module.
#
#--------------------------------------------------------------------------------------------------------------

include(../../mne-cpp.pri)

TEMPLATE = lib

QT       += widgets 3dcore 3drender 3dinput 3dextras charts

DEFINES += DISP3DNEW_LIBRARY

TARGET = Disp3D
TARGET = $$join(TARGET,,MNE$${MNE_LIB_VERSION},)
CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,d)
}
.
RESOURCES += $$PWD/disp3d.qrc \

LIBS += -L$${MNE_LIBRARY_DIR}
CONFIG(debug, debug|release) {
    LIBS += -lMNE$${MNE_LIB_VERSION}Genericsd \
            -lMNE$${MNE_LIB_VERSION}Utilsd \
            -lMNE$${MNE_LIB_VERSION}Fsd \
            -lMNE$${MNE_LIB_VERSION}Fiffd \
            -lMNE$${MNE_LIB_VERSION}Mned \
            -lMNE$${MNE_LIB_VERSION}Inversed \
            -lMNE$${MNE_LIB_VERSION}Dispd \
            -lMNE$${MNE_LIB_VERSION}DispChartsd
}
else {
    LIBS += -lMNE$${MNE_LIB_VERSION}Generics \
            -lMNE$${MNE_LIB_VERSION}Utils \
            -lMNE$${MNE_LIB_VERSION}Fs \
            -lMNE$${MNE_LIB_VERSION}Fiff \
            -lMNE$${MNE_LIB_VERSION}Mne \
            -lMNE$${MNE_LIB_VERSION}Inverse \
            -lMNE$${MNE_LIB_VERSION}Disp \
            -lMNE$${MNE_LIB_VERSION}DispCharts
}

DESTDIR = $${MNE_LIBRARY_DIR}

contains(MNECPP_CONFIG, build_MNECPP_Static_Lib) {
    CONFIG += staticlib
    DEFINES += BUILD_MNECPP_STATIC_LIB
}
else {
    CONFIG += dll

    #
    # win32: copy dll's to bin dir
    # unix: add lib folder to LD_LIBRARY_PATH
    #
    win32 {
        FILE = $${DESTDIR}/$${TARGET}.dll
        BINDIR = $${DESTDIR}/../bin
        FILE ~= s,/,\\,g
        BINDIR ~= s,/,\\,g
        QMAKE_POST_LINK += $${QMAKE_COPY} $$quote($${FILE}) $$quote($${BINDIR}) $$escape_expand(\\n\\t)
    }
}

SOURCES += \
    view3D.cpp \
    3DObjects/data3Dtreemodel.cpp \
    3DObjects/data3Dtreedelegate.cpp \
    3DObjects/subject/subjecttreeitem.cpp \
    3DObjects/common/metatreeitem.cpp \
    3DObjects/brain/brainsurfacetreeitem.cpp \
    3DObjects/brain/brainsurfacesettreeitem.cpp \
    3DObjects/brain/brainannotationtreeitem.cpp \
    3DObjects/brain/brainhemispheretreeitem.cpp \
    3DObjects/brain/brainrtsourcelocdatatreeitem.cpp \
    3DObjects/brain/brainrtconnectivitydatatreeitem.cpp \
    3DObjects/bem/bemtreeitem.cpp \
    3DObjects/bem/bemsurfacetreeitem.cpp \
    3DObjects/digitizer/digitizertreeitem.cpp \
    helpers/abstracttreeitem.cpp \
    helpers/renderable3Dentity.cpp \
    helpers/custommesh.cpp \
    control/control3dwidget.cpp \
    rt/rtSourceLoc/rtsourcelocdataworker.cpp \
    3DObjects/brain/brainsourcespacetreeitem.cpp \
    materials/shadermaterial.cpp \
    3DObjects/digitizer/digitizersettreeitem.cpp

HEADERS += \
    view3D.h \
    3DObjects/data3Dtreemodel.h \
    3DObjects/data3Dtreedelegate.h \
    3DObjects/subject/subjecttreeitem.h \
    3DObjects/common/metatreeitem.h \
    3DObjects/brain/brainsurfacetreeitem.h \
    3DObjects/brain/brainsurfacesettreeitem.h \
    3DObjects/brain/brainannotationtreeitem.h \
    3DObjects/brain/brainhemispheretreeitem.h \
    3DObjects/brain/brainrtsourcelocdatatreeitem.h \
    3DObjects/brain/brainrtconnectivitydatatreeitem.h \
    3DObjects/bem/bemtreeitem.h \
    3DObjects/bem/bemsurfacetreeitem.h \
    3DObjects/digitizer/digitizertreeitem.h \
    helpers/abstracttreeitem.h \
    helpers/renderable3Dentity.h \
    helpers/custommesh.h \
    helpers/types.h \
    control/control3dwidget.h \
    disp3D_global.h \
    rt/rtSourceLoc/rtsourcelocdataworker.h \
    3DObjects/brain/brainsourcespacetreeitem.h \
    materials/shadermaterial.h \
    3DObjects/digitizer/digitizersettreeitem.h

FORMS += \
    control/control3dwidget.ui \

INCLUDEPATH += $${EIGEN_INCLUDE_DIR}
INCLUDEPATH += $${MNE_INCLUDE_DIR}

# Install headers to include directory
header_files.files = ./*.h
header_files.path = $${MNE_INCLUDE_DIR}/disp3D

INSTALLS += header_files

unix: QMAKE_CXXFLAGS += -isystem $$EIGEN_INCLUDE_DIR

# Deploy Qt Dependencies
win32 {
    isEmpty(TARGET_EXT) {
        TARGET_CUSTOM_EXT = .dll
    } else {
        TARGET_CUSTOM_EXT = $${TARGET_EXT}
    }

    DEPLOY_COMMAND = windeployqt

    DEPLOY_TARGET = $$shell_quote($$shell_path($${MNE_BINARY_DIR}/$${TARGET}$${TARGET_CUSTOM_EXT}))

    #  # Uncomment the following line to help debug the deploy command when running qmake
    #  warning($${DEPLOY_COMMAND} $${DEPLOY_TARGET})
    QMAKE_POST_LINK += $${DEPLOY_COMMAND} $${DEPLOY_TARGET}
}
