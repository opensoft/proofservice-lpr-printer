TARGET = proofservice-label-printer
TEMPLATE = app

VERSION = 0.15.4.10

CONFIG += proofhardwarelabelprinter

SOURCES += \
    main.cpp \
    labelprinterrestserver.cpp \
    labelprinterhandler.cpp

HEADERS += \
    proofservice_label_printer_global.h \
    labelprinterrestserver.h \
    labelprinterhandler.h

DISTFILES += \
    CHANGELOG.md \
    UPGRADE.md

include($$(PROOF_PATH)/proof_service.pri)
