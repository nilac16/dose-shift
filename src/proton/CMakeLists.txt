add_library(proton proton-dose.c dcmload.cc mcc-data.c)

add_subdirectory(delaunay)

target_link_libraries(proton
    PUBLIC delaunay
    PRIVATE ${DCMTK_LIBRARIES})
