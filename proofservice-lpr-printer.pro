TARGET = proofservice-lpr-printer
TEMPLATE = app

VERSION = 0.18.9.24

CONFIG += proofutils

SOURCES += \
    main.cpp \
    lprprinterrestserver.cpp

HEADERS += \
    lprprinterrestserver.h \
    proofservice_lpr_printer_global.h

DISTFILES += \
    CHANGELOG.md \
    UPGRADE.md \
    README.md

include($$(PROOF_PATH)/proof_service.pri)
