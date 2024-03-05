# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)
#you may not need to set the python path, if you want to use it change it to your path
set(PYTHON "C:/Tools/Espressif/python_env/idf5.1_py3.11_env/Scripts/python")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(main)