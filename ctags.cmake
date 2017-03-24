# Add a top-level "tags" target which includes all files in both
# the build and source versions of src/*.
set_source_files_properties(tags PROPERTIES GENERATED true)
add_custom_target(tags
    COMMAND ctags -R --c++-kinds=+p --fields=+iaS --extra=+q 
        ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/tags ${CMAKE_SOURCE_DIR}
    get_target_property(FOO usbpptest SOURCES)
#    COMMAND echo ${FOO}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# ...but only make it a dependency of the project if the ctags program
# is available, else it will fail to build on Windows.
find_program(CTAGS_PATH ctags)
if(CTAGS_PATH)
    message(STATUS "Found ctags: ${CTAGS_PATH}")
    add_dependencies(usbpptest tags)
endif(CTAGS_PATH)

