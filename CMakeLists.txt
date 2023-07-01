cmake_minimum_required(VERSION 3.10)
project(transport_catalogue)

set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)


protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport-catalogue/svg.proto
        transport-catalogue/map_renderer.proto
        transport-catalogue/transport_catalogue.proto
        transport-catalogue/graph.proto
        transport-catalogue/transport_router.proto)

set(TRANSPORT_FILES transport-catalogue/main.cpp
        transport-catalogue/domain.cpp
        transport-catalogue/geo.cpp
        transport-catalogue/json.cpp
        transport-catalogue/json_reader.cpp
        transport-catalogue/map_renderer.cpp
        transport-catalogue/request_handler.cpp
        transport-catalogue/svg.cpp
        transport-catalogue/transport_catalogue.cpp
        transport-catalogue/json_builder.cpp
        transport-catalogue/transport_router.cpp
        transport-catalogue/serialization.cpp
        transport-catalogue/svg.proto
        transport-catalogue/map_renderer.proto
        transport-catalogue/transport_catalogue.proto
        transport-catalogue/graph.proto
        transport-catalogue/transport_router.proto)


add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${TRANSPORT_FILES})

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads -static)