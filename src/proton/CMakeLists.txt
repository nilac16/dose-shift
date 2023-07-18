add_library(proton proton-dose.c dcmload.cc mcc-data.c)

if (NOT WIN32)
    set(DCMTK::DCMTK ${DCMTK_LIBRARIES})
endif ()

target_link_libraries(proton
    PUBLIC DCMTK::DCMTK)
