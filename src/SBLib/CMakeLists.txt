cmake_minimum_required (VERSION 3.8)

include_directories(${PROJECT_SOURCE_DIR})

# Add source to this project's library.
add_library (SBLib
    source/CBitmap.cpp
    source/CCursor.cpp
    source/CFont.cpp
    source/GfxLib.cpp
    source/Network.cpp
    source/Tool2.cpp
    source/NetworkTypes/RAKNetNetwork.cpp
    source/NetworkTypes/ENetNetwork.cpp
    libs/rooms-plugin/IntervalTimer.cpp
    libs/rooms-plugin/ProfanityFilter.cpp
    libs/rooms-plugin/RoomsContainer.cpp
    libs/rooms-plugin/RoomsErrorCodes.cpp
    libs/rooms-plugin/RoomsPlugin.cpp
    libs/rooms-plugin/RoomTypes.cpp
    )

target_include_directories(SBLib PUBLIC include)
target_include_directories(SBLib PUBLIC libs)

target_link_libraries(SBLib
    raknet
    enet
    SDL2::SDL2_ttf
    SDL2::SDL2_image
    SDL2::SDL2_mixer
    SDL2::SDL2)
