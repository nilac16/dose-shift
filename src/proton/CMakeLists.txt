add_library(proton proton-dose.c dcmload.cc)

add_subdirectory(mcc)

target_link_libraries(proton
    PUBLIC mcc
    PRIVATE ${DCMTK_LIBRARIES})
