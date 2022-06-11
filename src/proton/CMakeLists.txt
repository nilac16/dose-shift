add_library(proton proton-dose.c dcmload.cc mcc-data.c)

target_link_libraries(proton
    PRIVATE ${DCMTK_LIBRARIES})
