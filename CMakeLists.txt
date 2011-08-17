PROJECT(SensEngine)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${SensEngine_SOURCE_DIR}/cmake)
IF(CMAKE_COMPILER_IS_GNUCXX)
  # on GCC, build to target a second-gen pentium 4, but run as well as
  # possible on a core 2.
  IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET(SENSE_C_FLAGS -march=nocona)
  ELSE(CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET(SENSE_C_FLAGS -march=prescott)
  ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 8)

  SET(SENSE_C_FLAGS "${SENSE_C_FLAGS} -Wall -Wno-deprecated -Werror -mtune=core2")
  SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} ${SENSE_C_FLAGS})
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x ${SENSE_C_FLAGS}")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

IF(MSVC)
  # MSVC doesn't give us as precise control over optimization. Just
  # enable SSE2
  ADD_DEFINITIONS("-D_CRT_SECURE_NO_WARNINGS")
  ADD_DEFINITIONS("-DNOMINMAX")
  ADD_DEFINITIONS("-D_SECURE_SCL=0")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
ENDIF(MSVC)

IF(WIN32)
  SET(PLATFORM "w32")
  SET(PIPELINE "ogl")
  SET(SENSE_platform_srcs 3rdparty/glew/glew.c)
ELSEIF(APPLE)
  SET(PLATFORM "mac")
  SET(PIPELINE "ogl")
  SET(SENSE_platform_srcs 3rdparty/glew/glew.c)
ELSE()
  SET(PLATFORM "x11")
  SET(PIPELINE "ogl")
  FIND_PACKAGE(X11 REQUIRED)
  SET(PLATFORM_LIBRARIES ${X11_LIBRARIES})
  SET(SENSE_platform_srcs 3rdparty/glew/glew.c)
ENDIF()

FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(Boost COMPONENTS filesystem system thread REQUIRED)
FIND_PACKAGE(Python 3 REQUIRED)
FIND_PACKAGE(PNG REQUIRED)
FIND_PACKAGE(ZLIB REQUIRED)

ADD_DEFINITIONS(-DGLEW_STATIC)
ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)
ADD_DEFINITIONS(-D BOOST_FILESYSTEM_VERSION=3)
ADD_DEFINITIONS(${PNG_DEFINITIONS})
INCLUDE_DIRECTORIES("3rdparty/glew")
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

INCLUDE(SenseConfig)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/client/Client_config.in.cpp
               ${CMAKE_CURRENT_BINARY_DIR}/client/Client_config.cpp)

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
)

SET(SENSE_pipeline_hdrs
  pipeline/interface.hpp
  pipeline/DefinitionTypes.hpp
  pipeline/Drawable.hpp
  pipeline/Image.hpp
  pipeline/Material.hpp
)

SET(SENSE_python_srcs
  python/module.cpp
)

SET(SENSE_python_pipeline_srcs
  python/pipeline/MaterialDefinition.cpp
)

SET(SENSE_python_pipeline_hdrs
  python/pipeline/PyMaterialDef.hpp
)

SET(SENSE_python_entity_srcs
  python/entity/Component.cpp
  python/entity/CoordinateComponent.cpp
  python/entity/DrawableComponent.cpp
  python/entity/Entity.cpp
  python/entity/EntityFactory.cpp
)

SET(SENSE_python_entity_hdrs
  python/entity/PyEntity.hpp
)

SET(SENSE_python_world_srcs
  python/world/DataManager.cpp
)

SET(SENSE_python_world_hdrs
  python/world/PyDataManager.hpp
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
  entity/CoordinateComponent.cpp
  entity/DrawableComponent.cpp
  entity/Entity.cpp
  entity/message/vtables.cpp
)

set(SENSE_entity_hdrs
  entity/Component.hpp
  entity/CoordinateComponent.hpp
  entity/DrawableComponent.hpp
  entity/Entity.hpp
  entity/message/DrawMessage.hpp
  entity/message/LoadMessage.hpp
)

SET(SENSE_link_libraries
  SenseCore
  ${OPENGL_gl_LIBRARY}
  ${Boost_LIBRARIES}
  ${PYTHON_LIBRARY}
  ${PLATFORM_LIBRARIES}
  ${BERKELIUM_LIBRARIES}
  ${PNG_LIBRARIES}
  ${ZLIB_LIBRARY}
)

SET(SENSE_client_libraries
  SensePipe 
)

SET(SENSE_world_srcs
  world/Builtins.cpp
  world/DataManager.cpp
)

SET(SENSE_world_hdrs
  world/Builtins.hpp
  world/DataManager.hpp
)

ADD_LIBRARY(SenseCore
            ${SENSE_platform_srcs}
            ${SENSE_entity_srcs} ${SENSE_entity_hdrs}
            ${SENSE_pipeline_srcs} ${SENSE_pipeline_hdrs}
            ${SENSE_world_srcs} ${SENSE_world_hdrs}
            ${SENSE_python_srcs} ${SENSE_python_hdrs}
            ${SENSE_python_entity_srcs} ${SENSE_python_entity_hdrs}
            ${SENSE_python_pipeline_srcs} ${SENSE_python_pipeline_hdrs}
            ${SENSE_python_world_srcs} ${SENSE_python_world_hdrs}
            ${SENSE_util_hdrs}
)

ADD_LIBRARY(SenseDummyPipe
            pipeline/dummy.cpp
)

IF(CMAKE_COMPILER_IS_GNUCXX)
  SET_TARGET_PROPERTIES(SenseCore PROPERTIES
                        COMPILE_FLAGS "-fPIC"
                        LINK_FLAGS "-fPIC"
  )
  SET_TARGET_PROPERTIES(SenseDummyPipe PROPERTIES
                        COMPILE_FLAGS "-fPIC"
                        LINK_FLAGS "-fPIC"
  )
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

ADD_EXECUTABLE(SensEngine WIN32 main.cpp
               ${SENSE_client_srcs} ${SENSE_client_hdrs}
               ${SENSE_client_python_srcs} ${SENSE_client_python_hdrs}
)
IF(MSVC)
  SET_TARGET_PROPERTIES(SensEngine PROPERTIES
                        LINK_FLAGS "/ENTRY:mainCRTStartup"
  )
ENDIF(MSVC)
TARGET_LINK_LIBRARIES(SensEngine
                      ${SENSE_client_libraries}
                      ${SENSE_link_libraries}
)

ADD_LIBRARY(PySensEngine SHARED pysense.cpp)
TARGET_LINK_LIBRARIES(PySensEngine
                      ${SENSE_link_libraries}
                      SenseDummyPipe
)
SET_TARGET_PROPERTIES(PySensEngine PROPERTIES
                      PREFIX ""
                      OUTPUT_NAME "SensEngine"
)

IF(WIN32)
  SET_TARGET_PROPERTIES(PySensEngine PROPERTIES
                        SUFFIX ".pyd"
  )
ENDIF(WIN32)

SOURCE_GROUP("pipeline" FILES ${SENSE_pipeline_srcs} ${SENSE_pipeline_hdrs})
SOURCE_GROUP("python" FILES ${SENSE_python_srcs} ${SENSE_python_hdrs})
SOURCE_GROUP("python\\pipeline" FILES ${SENSE_python_pipeline_srcs} ${SENSE_python_pipeline_hdrs})
SOURCE_GROUP("python\\entity" FILES ${SENSE_python_entity_srcs} ${SENSE_python_entity_hdrs})
SOURCE_GROUP("python\\world" FILES ${SENSE_python_world_srcs} ${SENSE_python_world_hdrs})
SOURCE_GROUP("util" FILES ${SENSE_util_hdrs})
SOURCE_GROUP("entity" FILES ${SENSE_entity_srcs} ${SENSE_entity_hdrs})
SOURCE_GROUP("world" FILES ${SENSE_world_srcs} ${SENSE_world_hdrs})
