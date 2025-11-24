# Copyright (c) Prophesee S.A. - All Rights Reserved
#
# Subject to Prophesee Metavision Licensing Terms and Conditions ("License T&C's").
# You may not use this file except in compliance with these License T&C's.
# A copy of these License T&C's is located in the "licensing" folder accompanying this file.

cmake_minimum_required(VERSION 3.5)
project(calibratio)

set(CMAKE_C_COMPILER /usr/bin/gcc-12)
set(CMAKE_CXX_COMPILER /usr/bin/g++-12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(MetavisionSDK COMPONENTS core driver cv calibration ui REQUIRED)
find_package(Boost COMPONENTS filesystem program_options REQUIRED)
find_package(OpenCV COMPONENTS core highgui imgproc calib3d REQUIRED)

set (sample calib_cam)

add_executable(${sample} 
               ${sample}.cpp
               blinking_pattern_focus.cpp
               calibration_recording.cpp
               blinking_chessboard_detector.cpp 
               blinking_dots_grid_detector.cpp)

target_include_directories(${sample} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${sample}
    MetavisionSDK::calibration
    MetavisionSDK::ui
    MetavisionSDK::cv
    MetavisionSDK::driver
    MetavisionSDK::core
    MetavisionSDK::base    
    Boost::program_options
    Boost::filesystem
    ${OpenCV_LIBS})

#target_link_libraries(${sample}
#   MetavisionSDK::core
#    MetavisionSDK::base    
#    MetavisionSDK::driver
#    MetavisionSDK::cv
#    MetavisionSDK::calibration
#    MetavisionSDK::ui
#    Boost::program_options
#    Boost::filesystem
#    ${OpenCV_LIBS})

