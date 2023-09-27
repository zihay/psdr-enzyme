# ----------------------------------------------------------
#  Compile Intel Embree
# ----------------------------------------------------------

set(EMBREE_ISPC_SUPPORT              OFF CACHE BOOL " " FORCE)
set(EMBREE_TUTORIALS                 OFF CACHE BOOL " " FORCE)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(-Wno-unused-variable)
endif()

# Briefly remove -march=native and let Embree do it's own ISA selection
unset(CMAKE_CXX_VISIBILITY_PRESET)
add_subdirectory(embree)
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
set(EMBREE_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/embree/include" PARENT_SCOPE)


# ----------------------------------------------------------
#  Compile Intel Enzyme
# ----------------------------------------------------------

add_subdirectory(Enzyme/enzyme)

add_subdirectory(googletest)
