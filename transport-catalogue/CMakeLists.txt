cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)
add_library(proto-objects OBJECT "${CMAKE_CURRENT_LIST_DIR}/transport_catalogue.proto")
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto)
#protobuf_generate(TARGET proto-objects
 #                 LANGUAGE cpp
  #                PROTOC_OUT_DIR "${CMAKE_CURRENT_LIST_DIR}"
   #               PROTOC_OPTIONS --experimental_allow_proto3_optional=true
    #              )

set(TR_FILES domain.h domain.cpp serialization.cpp transport_catalogue.h main.cpp serialization.h transport_catalogue.proto json_builder.cpp map_renderer.cpp transport_router.cpp domain.cpp json_builder.h map_renderer.h transport_router.h domain.h json.cpp ranges.h svg.cpp geo.cpp json.h request_handler.cpp svg.h geo.h json_reader.cpp request_handler.h transport_catalogue.cpp graph.h json_reader.h router.h)
#set(PROTO_FILES transport_catalogue.pb.cc transport_catalogue.pb.h)
add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${TR_FILES})
#add_executable(transport_catalogue ${PROTO_FILES} ${TR_FILES})
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)