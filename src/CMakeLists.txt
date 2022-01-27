
project (AT)

add_subdirectory (TeakLibW)
add_subdirectory (SBLib)
add_subdirectory (SSE)

include_directories(${PROJECT_SOURCE_DIR})

if(MSVC)
    add_compile_options("/MP")
    add_compile_options("-m")
else()
    add_compile_options("-Wall")
    add_compile_options("-Wextra")
    add_compile_options("-Wno-invalid-source-encoding")
    add_compile_options("-Wno-sign-compare")
    add_compile_options("-Wno-type-limits")
endif()

# Add source to this project's executable.
add_executable (AT
    Buero.cpp
    City.cpp
    DataTabl.cpp
    Route.cpp
    Tips.cpp
    Abend.cpp
    AirportView.cpp
    ArabAir.cpp
    Askbrick.cpp
    AtNet.cpp
    Aufsicht.cpp
    Auftrag.cpp
    Bank.cpp
    Block.cpp
    Brick.cpp
    Checkup.cpp
    ColorFx.cpp
    Credits.cpp
    DeltaVid.cpp
    Designer.cpp
    Dialog.cpp
    DutyFree.cpp
    Editor.cpp
    Fracht.cpp
    GameFrame.cpp
    Global.cpp
    Globe.cpp
    HalldiskMenu.cpp
    HLine.cpp
    Init.cpp
    Insel.cpp
    Intro.cpp
    Kiosk.cpp
    Klacker.cpp
    Laptop.cpp
    Lastmin.cpp
    Mainfrm.cpp
    Makler.cpp
    Misc.cpp
    Museum.cpp
    Nasa.cpp
    NewGamePopup.cpp
    Options.cpp
    Outro.cpp
    Person.cpp
    Personal.cpp
    Planer.cpp
    Planetyp.cpp
    PlanProp.cpp
    Player.cpp
    PNet.cpp
    Protect.cpp
    Reise.cpp
    Ricks.cpp
    RouteBox.cpp
    Rushmore.cpp
    Sabotage.cpp
    Sbbm.cpp
    Schedule.cpp
    Security.cpp
    SecurityManager.cpp
    Sim.cpp
    SmackPrs.cpp
    Sound.cpp
    Statistk.cpp
    StdAfx.cpp
    StdRaum.cpp
    Synthese.cpp
    Tafel.cpp
    Takeoff.cpp
    Talker.cpp
    TitlePopup.cpp
    WeltAll.cpp
    Werbung.cpp
    Werkstat.cpp
    World.cpp
    Res/Script2.rc
)

set_target_properties(AT PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/../BUILD/"
)

set_target_properties(
    AT PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/../BUILD/")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    target_compile_features(AT PUBLIC cxx_std_17)
    target_compile_options(AT PUBLIC "/Zc:__cplusplus")
    target_compile_options(AT PUBLIC "/std:c++17")
endif()

if(NOT ${CMAKE_VERSION} VERSION_LESS "3.16.0")
    target_precompile_headers(AT PUBLIC StdAfx.h)
endif()

target_link_libraries(AT
    TeakLibW
    SSE
    SBLib
    libsmacker
    enet
    SDL2::SDL2_ttf
    SDL2::SDL2_image
    SDL2::SDL2_mixer
    SDL2::SDL2
    SDL2::SDL2main
)

if (WIN32)
  target_link_libraries(AT ws2_32 Winmm)
else()
  target_link_libraries(AT jansson)
endif()

add_custom_command(TARGET AT POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:AT>"
     "BUILD/$<TARGET_FILE_NAME:AT>" 
  COMMENT "Copying to output directory")
