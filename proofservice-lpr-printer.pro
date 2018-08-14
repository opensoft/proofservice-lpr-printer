TARGET = proofservice-lpr-printer
TEMPLATE = app

VERSION = 0.17.12.25

CONFIG += proofutils

SOURCES += \
    main.cpp \
    lprprinterrestserver.cpp \
    lprprinterhandler.cpp

HEADERS += \
    lprprinterrestserver.h \
    lprprinterhandler.h \
    proofservice_lpr_printer_global.h

DISTFILES += \
    CHANGELOG.md \
    UPGRADE.md \
    README.md

include($$(PROOF_PATH)/proof_service.pri)
