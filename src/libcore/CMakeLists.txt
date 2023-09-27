set(INC_DIR "../../include")

add_library(psdr-core-obj OBJECT
    logger.cpp
    math_func.cpp
    sampler.cpp
    stats.cpp
    utils.cpp
    miniz.cpp
    statistics.cpp     
    properties.cpp 
    stream.cpp
    fstream.cpp
    cube_distrb.cpp
    bitmap.cpp
)

target_link_libraries(psdr-core-obj
    PUBLIC  spdlog)
target_compile_options(psdr-core-obj PUBLIC -flto)
add_library(psdr-core SHARED $<TARGET_OBJECTS:psdr-core-obj>)
# embed bitcode during linking. https://reviews.llvm.org/D68213?id=233652
target_link_options(psdr-core PUBLIC -flto)