cmake_minimum_required(VERSION 3.12.0)
find_package(Proof REQUIRED)
proof_project(proofservice-lpr-printer VERSION 0.19.4.2)

proof_add_target_sources(proofservice-lpr-printer
    main.cpp
    lprprinterrestserver.cpp
)

proof_add_target_headers(proofservice-lpr-printer
    lprprinterrestserver.h
    proofservice_lpr_printer_global.h
)

proof_add_service(proofservice-lpr-printer
    PROOF_LIBS Utils
)
