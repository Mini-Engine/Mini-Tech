
cmake_minimum_required(VERSION 3.18)
project(MiniGaffer)

# tod verify GAFFER_ROOT

option( USE_GAFFER_DEPENDENCIES "Turn this off you're building against your own dependencies (eg boost, python, tbb, exr, cortex) rather than the versions included with Gaffer" ON )

#set(CMAKE_VERBOSE_MAKEFILE ON)

set( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1 )

if(USE_GAFFER_DEPENDENCIES)
    set(BOOST_INCLUDE_PATH ${GAFFER_ROOT}/include)
    set(PYTHON_ROOT ${GAFFER_ROOT})
    set(TBB_ROOT ${GAFFER_ROOT})
    set(OPENEXR_ROOT ${GAFFER_ROOT})
    set(CORTEX_ROOT ${GAFFER_ROOT})
endif()

set(DEPENDENCY_INCLUDE_PATHS
        ${3RDPARTY_ROOT}/include
        ${3RDPARTY_ROOT}/include/bullet
        ${BOOST_INCLUDE_PATH}
        ${TBB_ROOT}/include
        ${OPENEXR_ROOT}/include
        ${OPENEXR_ROOT}/include/OpenEXR
        ${CORTEX_ROOT}/include
        ${GAFFER_ROOT}/include
        ${GAFFER_ROOT}/include/python3.10
        ${GAFFER_ROOT}/include/Imath
)

# todo lib on linux and libs & Lib on windows?
link_directories( MiniGaffer
        ${3RDPARTY_ROOT}/lib
        ${GAFFER_ROOT}/lib
        ${GAFFER_ROOT}/libs
        ${GAFFER_ROOT}/Lib)

if (WIN32)
    set(BOOST_INCLUDEDIR ${BOOST_INCLUDE_PATH})
    find_package( Boost REQUIRED COMPONENTS python${PYTHON_VERSION})
endif()

# build the library
file( GLOB MiniGafferSrc src/MiniGaffer/*.cpp src/MiniGaffer/BulletPhysics/*.cpp )
add_library( MiniGaffer SHARED ${MiniGafferSrc} )

file( GLOB MiniGafferVDBSrc src/MiniGafferVDB/*.cpp)
add_library( MiniGafferVDB SHARED ${MiniGafferVDBSrc} )

if (WIN32)
    target_compile_definitions( MiniGaffer PRIVATE BOOST_SIGNALS_NO_DEPRECATION_WARNING=1  _GLIBCXX_USE_CXX11_ABI=0)
    target_compile_definitions( MiniGafferVDB PRIVATE BOOST_SIGNALS_NO_DEPRECATION_WARNING=1  _GLIBCXX_USE_CXX11_ABI=0)
else()
    target_compile_definitions( MiniGaffer PRIVATE BOOST_SIGNALS_NO_DEPRECATION_WARNING=1 LINUX=1 _GLIBCXX_USE_CXX11_ABI=0)
    target_compile_definitions( MiniGafferVDB PRIVATE BOOST_SIGNALS_NO_DEPRECATION_WARNING=1 LINUX=1 _GLIBCXX_USE_CXX11_ABI=0)
endif()

target_include_directories( MiniGaffer SYSTEM PRIVATE ${DEPENDENCY_INCLUDE_PATHS} )
target_include_directories( MiniGaffer PRIVATE include )
target_link_libraries( MiniGaffer Gaffer GafferScene GafferImage IECore IECoreScene GafferUI
            fmt osdCPU
            BulletDynamics BulletCollision LinearMath)
install( TARGETS MiniGaffer DESTINATION lib )

target_include_directories( MiniGafferVDB SYSTEM PRIVATE ${DEPENDENCY_INCLUDE_PATHS} )
target_include_directories( MiniGafferVDB PRIVATE include )
target_link_libraries( MiniGafferVDB Gaffer GafferVDB GafferScene GafferImage IECore IECoreScene IECoreVDB GafferUI
        fmt openvdb)
install( TARGETS MiniGafferVDB DESTINATION lib )

# build the python bindings
file( GLOB MiniGafferModuleSrc src/MiniGafferModule/*.cpp )
add_library( MiniGafferModule SHARED ${MiniGafferModuleSrc} )
set_target_properties( MiniGafferModule PROPERTIES PREFIX "" OUTPUT_NAME "_MiniGaffer" )
if (WIN32)
    target_compile_definitions( MiniGafferModule PRIVATE BOOST_SIGNALS_NO_DEPRECATION_WARNING=1  _GLIBCXX_USE_CXX11_ABI=0 BOOST_ALL_NO_LIB=1)
else()
    target_compile_definitions( MiniGafferModule PRIVATE BOOST_SIGNALS_NO_DEPRECATION_WARNING=1 LINUX=1 _GLIBCXX_USE_CXX11_ABI=0)
endif()

target_include_directories( MiniGafferModule SYSTEM PRIVATE include ${DEPENDENCY_INCLUDE_PATHS} ${PYTHON_ROOT}/include/python${PYTHON_VERSION} )

if (WIN32)
    target_link_libraries( MiniGafferModule GafferBindings MiniGaffer IECorePython Boost::python${PYTHON_VERSION} python${PYTHON_VERSION} fmt)
    set_target_properties(MiniGafferModule PROPERTIES SUFFIX ".pyd")
else ()
    target_link_libraries( MiniGafferModule GafferBindings MiniGaffer IECorePython  python${PYTHON_VERSION} fmt)
endif()

# build the python bindings
file( GLOB MiniGafferVDBModuleSrc src/MiniGafferVDBModule/*.cpp )
add_library( MiniGafferVDBModule SHARED ${MiniGafferVDBModuleSrc} )
set_target_properties( MiniGafferVDBModule PROPERTIES PREFIX "" OUTPUT_NAME "_MiniGafferVDB" )
if (WIN32)
    target_compile_definitions( MiniGafferVDBModule PRIVATE BOOST_SIGNALS_NO_DEPRECATION_WARNING=1  _GLIBCXX_USE_CXX11_ABI=0 BOOST_ALL_NO_LIB=1)
else()
    target_compile_definitions( MiniGafferVDBModule PRIVATE BOOST_SIGNALS_NO_DEPRECATION_WARNING=1 LINUX=1 _GLIBCXX_USE_CXX11_ABI=0)
endif()

target_include_directories( MiniGafferVDBModule SYSTEM PRIVATE include ${DEPENDENCY_INCLUDE_PATHS} ${PYTHON_ROOT}/include/python${PYTHON_VERSION} )

if (WIN32)
    target_link_libraries( MiniGafferVDBModule GafferBindings MiniGafferVDB IECorePython Boost::python${PYTHON_VERSION} python${PYTHON_VERSION} fmt)
    set_target_properties( MiniGafferVDBModule PROPERTIES SUFFIX ".pyd")
else ()
    target_link_libraries( MiniGafferVDBModule GafferBindings MiniGafferVDB IECorePython  python${PYTHON_VERSION} fmt)
endif()


# build OSL shaders

set(CMAKE_MSVCIDE_RUN_PATH ${CMAKE_MSVCIDE_RUN_PATH};${GAFFER_ROOT}/lib)

file( GLOB OSL_SHADER_SRCS ${CMAKE_SOURCE_DIR}/shaders/*.osl )
foreach(OSL_SHADER_SRC ${OSL_SHADER_SRCS})
    get_filename_component(OSL_SHADER_SRC_NAME ${OSL_SHADER_SRC} NAME_WE)
    set(OSO_PATH ${CMAKE_BINARY_DIR}/${OSL_SHADER_SRC_NAME}.oso)

if (WIN32)
    add_custom_command(
            OUTPUT ${OSO_PATH}
            COMMAND ${GAFFER_ROOT}/bin/oslc -v ${OSL_SHADER_SRC} -o ${OSO_PATH}
            DEPENDS ${OSL_SHADER_SRC}
    )
else()
    add_custom_command(
            OUTPUT ${OSO_PATH}
            COMMAND export LD_LIBRARY_PATH=${GAFFER_ROOT}/lib:${LD_LIBRARY_PATH} && ${GAFFER_ROOT}/bin/oslc -v ${OSL_SHADER_SRC} -o ${OSO_PATH}
            DEPENDS ${OSL_SHADER_SRC}
    )
endif() # WIN32

    list(APPEND OSO_FILES ${OSO_PATH})
endforeach()

add_custom_target(OslShaders ALL
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS ${OSO_FILES}
)


install( TARGETS MiniGafferModule DESTINATION python/MiniGaffer )
install( TARGETS MiniGafferVDBModule DESTINATION python/MiniGafferVDB )

# build the python modules
install( DIRECTORY python DESTINATION . FILES_MATCHING PATTERN "*.py" )

# build the startup configs
install( DIRECTORY startup DESTINATION . FILES_MATCHING PATTERN "*.py" )

# build the startup configs
install( DIRECTORY nodes DESTINATION . FILES_MATCHING PATTERN "*.grf" )

# install osl headers
install( DIRECTORY shaders DESTINATION . FILES_MATCHING PATTERN "*.osl.h")

install( DIRECTORY graphics DESTINATION . )

# install examples
install( DIRECTORY docs DESTINATION . FILES_MATCHING PATTERN "*.gfr")

install( DIRECTORY examples DESTINATION . PATTERN "examples/.gafferBackups" EXCLUDE)

# install compiled osl shaders
install( FILES ${OSO_FILES} DESTINATION shaders)

# build the graphics
# install( FILES ${ATOMS_ICONS_PATH}/logo_new.png DESTINATION graphics RENAME atoms_logo.png )

# build the examples
# file( GLOB MiniGafferExampleScripts examples/scripts/*.gfr )
# install( FILES ${MiniGafferExampleScripts} DESTINATION examples/scripts )
# install( DIRECTORY examples/assets DESTINATION examples )