add_library(proton proton-dose.c proton-image.c dcmload.cc)

target_link_libraries(proton PRIVATE ${DCMTK_LIBRARIES})
