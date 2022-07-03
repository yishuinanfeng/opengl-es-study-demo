# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.
cmake_minimum_required(VERSION 3.4.1)


####################################  Darren教程
#设置变量
#SET(SRC_LIST native-lib.cpp)

#给工程取名字
#PROJECT(YUV)
#打印源码路径
#MESSAGE(STATUS "yuvPlayer BINARY DIR" ${PROJECT_SOURCE_DIR})

#让SRC_LIST指向当前makeList所在目录的src下的所有源文件
#AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src SRC_LIST)
#MESSAGE(STATUS "SRC_LIST:" ${SRC_LIST})
#这条命令会将后面匹配到的所有文件 src/*.cpp 交给 GLOB 子命令，由后者生成一个文件列表，
#并将列表赋给 SOURCES 变量。由于这条命令可以帮助我们自动引用所有的源文件，而 set 命令需要我们一个一个地添加文件，所以这里使用 FILE 更加省事
#FILE(GLOB CPP_LIST "${PROJECT_SOURCE_DIR}/src/*cpp")
#FILE(GLOB C_LIST "${PROJECT_SOURCE_DIR}/src/*c")

#INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)
#生成动态库.so(SHARED)，自动在名字前面加lib
#ADD_LIBRARY(native-lib SHARED ${CPP_LIST} ${C_LIST})
#指定需要链接的so库的路径
#LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib)
#添加源文件
#ADD_EXECUTABLE(native-lib ${SRC_LIST})
#将native-lib.so和其他so库进行链接
#TARGET_LINK_LIBRARIES(native-lib GLESv2 EGL)

#############################################  Darren教程


# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             native-lib.cpp )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-lib
                       GLESv2
                       EGL
                       android
                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )