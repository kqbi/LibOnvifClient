aux_source_directory(. onvifTest_src_list)

add_executable(onvifTest ${onvifTest_src_list})

if(ANDROID)
    target_link_libraries(onvifTest ${CMAKE_PROJECT_NAME}_static ${LINK_LIB_LIST})
elseif(WIN32)
    target_link_libraries(onvifTest ${CMAKE_PROJECT_NAME}_static ${LINK_LIB_LIST})
else()
    target_link_libraries(onvifTest ${CMAKE_PROJECT_NAME}_static ${LINK_LIB_LIST} ssl crypto pthread)
endif()
