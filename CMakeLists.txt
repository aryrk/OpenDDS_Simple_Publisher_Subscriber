cmake_minimum_required(VERSION 3.10)
project(DDS_PubSub)

find_package(OpenDDS REQUIRED)

# Use the modern OpenDDS CMake approach
add_library(messenger_idl)
opendds_target_sources(messenger_idl PUBLIC "Messager.idl" OPENDDS_IDL_OPTIONS -Lc++11 -Gxtypes-complete)
target_link_libraries(messenger_idl PUBLIC OpenDDS::Dcps)

add_executable(publisher
    publisher.cpp
    MessageReaderListener.cpp
)

target_link_libraries(publisher PRIVATE messenger_idl OpenDDS::Dcps)

add_executable(subscriber
    subscriber.cpp
    MessageReaderListener.cpp
)

target_link_libraries(subscriber PRIVATE messenger_idl OpenDDS::Dcps)