# the pipeline library (OpenGL 3.2 implementation)

ADD_LIBRARY(SensePipe
            implementation.hpp
            Pipeline.cpp
            Loader.cpp
            Webview.cpp Webview.hpp
            
)
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET_TARGET_PROPERTIES(SensePipe PROPERTIES COMPILE_FLAGS "-fPIC" LINK_FLAGS "-fPIC")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)
