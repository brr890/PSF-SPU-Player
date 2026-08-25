cmake_minimum_required(VERSION 3.16)

project(psf_spu_player VERSION 1.57 LANGUAGES C CXX)

set(PSF_ARCHIVE_RUNTIME_DIR "" CACHE PATH "Folder containing the optional 32-bit bsdtar archive runtime")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MINGW AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    add_compile_options(-msse2 -mfpmath=sse)
endif()

if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    add_compile_options(/arch:SSE2)
endif()

find_package(ZLIB REQUIRED)

add_library(spu2log STATIC
    src/spu2log.c
)
target_include_directories(spu2log PUBLIC include)
target_compile_definitions(spu2log PUBLIC SPU2LOG_STATIC)

add_library(spu2log_spu2_adapter STATIC
    src/spu2log_spu2_adapter.c
)
target_include_directories(spu2log_spu2_adapter PUBLIC include)
target_link_libraries(spu2log_spu2_adapter PUBLIC spu2log)

add_library(spu2log_audacious_hooks STATIC
    src/spu2log_audacious_hooks.c
)
target_include_directories(spu2log_audacious_hooks PUBLIC include)

add_library(psf2_core_bridge STATIC
    src/psf2_core_bridge.c
)
target_include_directories(psf2_core_bridge PUBLIC include)
target_link_libraries(psf2_core_bridge PUBLIC spu2log_spu2_adapter)

set(AUDACIOUS_PSF_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/psf2_core/upstream/audacious-plugins-master/src/psf
)

add_library(psf2_provider_imported STATIC
    src/providers/psf2_provider_audacious.cc
    ${AUDACIOUS_PSF_DIR}/corlett.cc
    ${AUDACIOUS_PSF_DIR}/eng_psf.cc
    ${AUDACIOUS_PSF_DIR}/eng_psf2.cc
    ${AUDACIOUS_PSF_DIR}/psx.cc
    ${AUDACIOUS_PSF_DIR}/psx_hw.cc
    ${AUDACIOUS_PSF_DIR}/peops/spu.cc
    ${AUDACIOUS_PSF_DIR}/peops2/dma.cc
    ${AUDACIOUS_PSF_DIR}/peops2/registers.cc
    ${AUDACIOUS_PSF_DIR}/peops2/spu.cc
)
target_include_directories(psf2_provider_imported
    PUBLIC include
    PRIVATE
        ${AUDACIOUS_PSF_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/audacious_compat/include
)
target_link_libraries(psf2_provider_imported
    PUBLIC psf2_core_bridge
    PRIVATE spu2log_audacious_hooks
)
target_compile_definitions(psf2_provider_imported PRIVATE
    PSF2LOG_IMPORTED_PROVIDER_LICENSE="GPL-2.0-or-later"
)

if(MINGW)
    get_filename_component(PSF2LOG_ZLIB_LIBRARY_DIR "${ZLIB_LIBRARY_RELEASE}" DIRECTORY)
    find_file(PSF2LOG_ZLIB_STATIC_LIBRARY
        NAMES libz.a
        PATHS "${PSF2LOG_ZLIB_LIBRARY_DIR}"
        NO_DEFAULT_PATH
        REQUIRED
    )
    target_link_libraries(psf2_provider_imported PRIVATE "${PSF2LOG_ZLIB_STATIC_LIBRARY}")
else()
    target_link_libraries(psf2_provider_imported PRIVATE ZLIB::ZLIB)
endif()

add_library(psf_file STATIC
    src/psf_file.c
)
target_include_directories(psf_file PUBLIC include)

if(MSVC)
    foreach(target
        spu2log
        spu2log_spu2_adapter
        spu2log_audacious_hooks
        psf2_core_bridge
        psf2_provider_imported
        psf_file
    )
        target_compile_options(${target} PRIVATE /W4)
    endforeach()
else()
    foreach(target
        spu2log
        spu2log_spu2_adapter
        spu2log_audacious_hooks
        psf2_core_bridge
        psf2_provider_imported
        psf_file
    )
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endforeach()
endif()

if(NOT WIN32)
    message(FATAL_ERROR "PSF SPU Player currently supports Windows builds only")
endif()

enable_language(RC)
set_source_files_properties(app/psf2_spu2_player.rc PROPERTIES
    OBJECT_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/psf2_player_version.h"
)
add_executable(psf_spu_player WIN32
    app/psf2_spu2_player.c
    app/psf2_spu2_player.rc
)
target_include_directories(psf_spu_player PRIVATE include)
target_link_libraries(psf_spu_player PRIVATE
    psf2_provider_imported
    psf_file
    winmm
    comdlg32
    comctl32
    shell32
    ole32
)

if(MINGW)
    target_link_options(psf_spu_player PRIVATE -static)
    if(CMAKE_BUILD_TYPE STREQUAL "Release" AND CMAKE_STRIP)
        add_custom_command(TARGET psf_spu_player POST_BUILD
            COMMAND ${CMAKE_STRIP} --strip-all $<TARGET_FILE:psf_spu_player>
            COMMENT "Removing non-runtime debug and symbol data"
            VERBATIM
        )
    endif()

    if(NOT PSF_ARCHIVE_RUNTIME_DIR)
        find_program(PSF_ARCHIVE_BSDTAR_EXECUTABLE
            NAMES bsdtar.exe bsdtar
            HINTS "C:/msys64/mingw32/bin"
        )
        if(PSF_ARCHIVE_BSDTAR_EXECUTABLE)
            get_filename_component(PSF_ARCHIVE_RUNTIME_DIR
                "${PSF_ARCHIVE_BSDTAR_EXECUTABLE}" DIRECTORY)
        endif()
    endif()
    set(PSF_ARCHIVE_RUNTIME_FILES
        bsdtar.exe
        libarchive-13.dll
        libb2-1.dll
        libbz2-1.dll
        libexpat-1.dll
        libgcc_s_dw2-1.dll
        libiconv-2.dll
        liblz4.dll
        liblzma-5.dll
        libpcre2-8-0.dll
        libpcre2-posix-3.dll
        libwinpthread-1.dll
        libzstd.dll
        zlib1.dll
    )
    set(PSF_ARCHIVE_RUNTIME_READY TRUE)
    foreach(PSF_ARCHIVE_RUNTIME_FILE IN LISTS PSF_ARCHIVE_RUNTIME_FILES)
        if(NOT EXISTS "${PSF_ARCHIVE_RUNTIME_DIR}/${PSF_ARCHIVE_RUNTIME_FILE}")
            set(PSF_ARCHIVE_RUNTIME_READY FALSE)
        endif()
    endforeach()
    if(PSF_ARCHIVE_RUNTIME_READY)
        add_custom_command(TARGET psf_spu_player POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:psf_spu_player>/archive"
            COMMENT "Installing the optional archive extraction runtime"
        )
        foreach(PSF_ARCHIVE_RUNTIME_FILE IN LISTS PSF_ARCHIVE_RUNTIME_FILES)
            add_custom_command(TARGET psf_spu_player POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${PSF_ARCHIVE_RUNTIME_DIR}/${PSF_ARCHIVE_RUNTIME_FILE}"
                    "$<TARGET_FILE_DIR:psf_spu_player>/archive/${PSF_ARCHIVE_RUNTIME_FILE}"
                VERBATIM
            )
        endforeach()
    else()
        message(STATUS "Archive runtime not found; installed 7-Zip or Windows tar will be used")
    endif()
endif()
