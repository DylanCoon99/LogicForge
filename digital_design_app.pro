QT       += core gui widgets svg
CONFIG   += c++17 sdk_no_version_check

QMAKE_LIBS_OPENGL = -framework OpenGL

TARGET = digital_design_app
TEMPLATE = app

INCLUDEPATH += src

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/canvas/circuitscene.cpp \
    src/canvas/circuitview.cpp \
    src/model/pin.cpp \
    src/model/component.cpp \
    src/model/wire.cpp \
    src/model/circuit.cpp \
    src/model/simulationengine.cpp \
    src/components/gatecomponent.cpp \
    src/components/inputswitch.cpp \
    src/components/outputprobe.cpp \
    src/components/clocksource.cpp \
    src/components/flipflop.cpp \
    src/components/latch.cpp \
    src/components/mux.cpp \
    src/components/demux.cpp \
    src/components/registercomponent.cpp \
    src/components/counter.cpp \
    src/components/bussplitter.cpp \
    src/components/busjoiner.cpp \
    src/components/customcomponent.cpp \
    src/dialogs/createcustomdialog.cpp \
    src/graphics/componentgraphicsitem.cpp \
    src/graphics/pingraphicsitem.cpp \
    src/graphics/wiregraphicsitem.cpp \
    src/widgets/truthtabledialog.cpp \
    src/widgets/timingdiagramwidget.cpp

HEADERS += \
    src/mainwindow.h \
    src/canvas/circuitscene.h \
    src/canvas/circuitview.h \
    src/model/pin.h \
    src/model/component.h \
    src/model/wire.h \
    src/model/circuit.h \
    src/model/simulationengine.h \
    src/components/gatecomponent.h \
    src/components/inputswitch.h \
    src/components/outputprobe.h \
    src/components/clocksource.h \
    src/components/flipflop.h \
    src/components/latch.h \
    src/components/mux.h \
    src/components/demux.h \
    src/components/registercomponent.h \
    src/components/counter.h \
    src/components/bussplitter.h \
    src/components/busjoiner.h \
    src/components/customcomponent.h \
    src/dialogs/createcustomdialog.h \
    src/graphics/componentgraphicsitem.h \
    src/graphics/pingraphicsitem.h \
    src/graphics/wiregraphicsitem.h \
    src/widgets/truthtabledialog.h \
    src/widgets/timingdiagramwidget.h
