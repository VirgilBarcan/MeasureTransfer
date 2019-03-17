cmake_minimum_required(VERSION 3.13)
project(MeasureTransfer)

set(CMAKE_CXX_STANDARD 17)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.64.0 COMPONENTS system thread)

include_directories(common)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})

    # Make the Server
    add_executable(Server server/main.cpp server/server.h server/session.h server/communicator.h)
    target_link_libraries(Server ${Boost_LIBRARIES})

    # Make the Client
    add_executable(Client client/main.cpp common/messages.h common/types.h common/utils.h client/client.h)
    target_link_libraries(Client ${Boost_LIBRARIES} pthread)
endif()
