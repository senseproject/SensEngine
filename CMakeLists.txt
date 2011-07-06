PROJECT(SensEngine)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${SensEngine_SOURCE_DIR}/cmake)
IF(CMAKE_COMPILER_IS_GNUCXX)
    IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=nocona")
    ELSE(CMAKE_SIZEOF_VOID_P EQUAL 8)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=pentium4")
    ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wno-deprecated -Werror -mtune=core2 -msse2")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)
IF(MSVC)
    ADD_DEFINITIONS("-D_CRT_SECURE_NO_WARNINGS")
    ADD_DEFINITIONS("-DNOMINMAX")
	ADD_DEFINITIONS("-D_SECURE_SCL=0")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
ENDIF(MSVC)

IF(WIN32)
  SET(PLATFORM "w32")
  SET(PIPELINE "ogl")
  SET(SENSE_platform_srcs 3rdparty/glew/glew.c)
ELSE(WIN32)
  SET(PLATFORM "x11")
  SET(PIPELINE "ogl")
  FIND_PACKAGE(X11 REQUIRED)
  SET(PLATFORM_LIBRARIES ${X11_LIBRARIES})
  SET(SENSE_platform_srcs 3rdparty/glew/glew.c)
ENDIF(WIN32)

FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(Boost COMPONENTS filesystem system thread REQUIRED)
FIND_PACKAGE(Python 3 REQUIRED)
# Disabled until Berkelium can be made to work correctly
# FIND_PACKAGE(Berkelium REQUIRED)

ADD_DEFINITIONS(-DGLEW_STATIC)
ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)
ADD_DEFINITIONS(-D BOOST_FILESYSTEM_VERSION=3)
INCLUDE_DIRECTORIES("3rdparty/glew")
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})
# INCLUDE_DIRECTORIES(${BERKELIUM_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

INCLUDE(SenseConfig)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/client/Client_config.in.cpp ${CMAKE_CURRENT_BINARY_DIR}/client/Client_config.cpp)

ADD_SUBDIRECTORY(pipeline/${PIPELINE})

SET(SENSE_client_srcs
  client/Client.cpp
  client/Client_${PLATFORM}_${PIPELINE}.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/client/Client_config.cpp
)

SET(SENSE_client_hdrs
  client/Client.hpp
)

SET(SENSE_pipeline_srcs
  pipeline/Builtins.cpp
)

SET(SENSE_pipeline_hdrs
  pipeline/interface.hpp
  pipeline/Drawable.hpp
  pipeline/Builtins.hpp
)

SET(SENSE_python_srcs
  python/module.cpp
)

SET(SENSE_python_pipeline_srcs
  python/pipeline/Loader.cpp
  python/pipeline/MaterialDefinition.cpp
)

SET(SENSE_python_pipeline_hdrs
  python/pipeline/PyMaterialDef.hpp
)

SET(SENSE_python_hdrs
  python/module.hpp
  python/pywarnings.hpp
)

SET(SENSE_util_hdrs
  util/atomic.hpp
  util/queue.hpp
  util/util.hpp
)

set(SENSE_entity_srcs
  entity/stubs.cpp
)

set(SENSE_entity_hdrs
  entity/Component.hpp
  entity/Entity.hpp
)

SET(SENSE_link_libraries
  SenseCore
  SensePipe
  SenseCore
  ${OPENGL_gl_LIBRARY}
  ${Boost_LIBRARIES}
  ${PYTHON_LIBRARY}
  ${PLATFORM_LIBRARIES}
  ${BERKELIUM_LIBRARIES}
)

ADD_LIBRARY(SenseCore
            ${SENSE_platform_srcs}
            ${SENSE_entity_srcs} ${SENSE_entity_hdrs}
            ${SENSE_pipeline_srcs} ${SENSE_pipeline_hdrs}
            ${SENSE_python_srcs} ${SENSE_python_hdrs}
            ${SENSE_python_pipeline_srcs} ${SENSE_python_pipeline_hdrs}
            ${SENSE_util_hdrs}
)
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET_TARGET_PROPERTIES(SenseCore PROPERTIES COMPILE_FLAGS "-fPIC" LINK_FLAGS "-fPIC")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

ADD_EXECUTABLE(SensEngine main.cpp ${SENSE_client_srcs} ${SENSE_client_hdrs})
TARGET_LINK_LIBRARIES(SensEngine ${SENSE_link_libraries})

ADD_LIBRARY(PySensEngine SHARED pysense.cpp)
TARGET_LINK_LIBRARIES(PySensEngine ${SENSE_link_libraries})
SET_TARGET_PROPERTIES(PySensEngine PROPERTIES PREFIX "" OUTPUT_NAME "SensEngine")
IF(WIN32)
  SET_TARGET_PROPERTIES(PySensEngine PROPERTIES SUFFIX ".pyd")
ENDIF(WIN32)

SOURCE_GROUP("pipeline" FILES ${SENSE_pipeline_srcs} ${SENSE_pipeline_hdrs})
SOURCE_GROUP("python" FILES ${SENSE_python_srcs} ${SENSE_python_hdrs})
SOURCE_GROUP("python\\pipeline" FILES ${SENSE_python_pipeline_srcs} ${SENSE_python_pipeline_hdrs})
SOURCE_GROUP("util" FILES ${SENSE_util_hdrs})
SOURCE_GROUP("entity" FILES ${SENSE_entity_srcs} ${SENSE_entity_hdrs})
SOURCE_GROUP("client" FILES ${SENSE_client_srcs} ${SENSE_client_hdrs})
