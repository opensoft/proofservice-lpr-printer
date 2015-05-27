TARGET = proofservice-label-printer
TEMPLATE = app

VERSION = 0.15.4.10

QT += core network
QT -= gui

CONFIG += c++11 proof proofcore proofnetwork proofhardwarelabelprinter
CONFIG -= app_bundle

SOURCES += \
    main.cpp \
    labelprinterrestserver.cpp \
    labelprinterhandler.cpp

HEADERS += \
    proofservice_label_printer_global.h \
    labelprinterrestserver.h \
    labelprinterhandler.h

target.path = $$PREFIX/opt/Opensoft/proof/bin/
rename_target.path = $$PREFIX/opt/Opensoft/proof/bin/
rename_target.files = $$PREFIX/opt/Opensoft/proof/bin/$$TARGET
rename_target.extra = mv $$PREFIX/opt/Opensoft/proof/bin/$$TARGET $$PREFIX/opt/Opensoft/proof/bin/$$TARGET-bin
target_link.path = $$PREFIX/opt/Opensoft/proof/bin/
target_link.commands = cd $$PREFIX/opt/Opensoft/proof/bin/ && ln -s ./proof-wrapper $$TARGET

# Need for proof-restarter script from proof package
target_spawn.path = $$PREFIX/opt/Opensoft/proof/bin/
target_spawn.commands = mkdir -p $$PREFIX/opt/Opensoft/proof/bin/spawn/$$TARGET && echo \"$$LITERAL_HASH!/bin/bash\n\`dirname \\\$$0\`/../../$$TARGET\" > $$PREFIX/opt/Opensoft/proof/bin/spawn/$$TARGET/run
 
INSTALLS += target
INSTALLS += rename_target
INSTALLS += target_link
INSTALLS += target_spawn
