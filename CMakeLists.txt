# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(Jino LANGUAGES CXX)

# --- Configuration ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# --- Set Output Directory (for local build) ---
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)


# --- Find Packages ---
find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui)

# --- QtAwesome Integration ---
set(QTAWESOME_SUBMODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/QtAwesome)
if(EXISTS ${QTAWESOME_SUBMODULE_DIR}/QtAwesome/QtAwesome.h)
    set(QTAWESOME_SRC_DIR ${QTAWESOME_SUBMODULE_DIR}/QtAwesome)
    set(QTAWESOME_SOURCES
        ${QTAWESOME_SRC_DIR}/QtAwesome.cpp
        ${QTAWESOME_SRC_DIR}/QtAwesomeAnim.cpp
    )
    set(QTAWESOME_RESOURCE_FILE
        ${QTAWESOME_SRC_DIR}/QtAwesomeFree.qrc
    )
    message(STATUS "Using QtAwesome submodule from vendor/ directory")
else()
    message(WARNING "QtAwesome submodule not found at ${QTAWESOME_SUBMODULE_DIR}. Build might fail.")
    set(QTAWESOME_SOURCES "")
    set(QTAWESOME_RESOURCE_FILE "")
    set(QTAWESOME_SRC_DIR "") # Ensure this is also empty if not found
endif()


# --- Project Sources ---
# Define source file variables (optional, but can keep clarity)
set(APP_SOURCES
    src/app/jino_editor.cpp
    src/app/status_bar_manager.cpp
    src/app/menu_manager.cpp
)
set(EDITOR_SOURCES
    src/editor/editor_widget.cpp
    src/editor/line_number_widget.cpp
    src/editor/vim/vim_handler.cpp
    src/editor/org_syntax_highlighter.cpp
    src/editor/markdown_syntax_highlighter.cpp
)
set(RESOURCE_FILES
    resources.qrc
    ${QTAWESOME_RESOURCE_FILE}
)

# --- Executable ---
add_executable(jino
    # --- List ONLY source (.cpp) and resource (.qrc) files ---
    src/main.cpp
    ${APP_SOURCES}
    ${EDITOR_SOURCES}
    ${QTAWESOME_SOURCES} # Add QtAwesome sources if found
    ${RESOURCE_FILES}
    # *** DO NOT LIST HEADER FILES (.hpp) HERE ***
)

# --- Linking ---
target_link_libraries(jino PRIVATE
    Qt5::Widgets
)

# --- Include Directories ---
target_include_directories(jino PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src # For finding project headers
    ${CMAKE_CURRENT_BINARY_DIR}    # *** ADDED: For MOC generated files ***
    ${QTAWESOME_SRC_DIR}           # For QtAwesome headers (if found)
)

# --- Installation ---
# (Installation rules remain the same)
install(TARGETS jino RUNTIME DESTINATION bin)
install(FILES data/jino.desktop DESTINATION share/applications)
install(FILES assets/icons/jino_icon_48x48.png DESTINATION share/icons/hicolor/48x48/apps RENAME jino.png)
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Executable will be installed to: ${CMAKE_INSTALL_PREFIX}/bin")
message(STATUS "Desktop file will be installed to: ${CMAKE_INSTALL_PREFIX}/share/applications")
message(STATUS "Icon file will be installed to: ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/48x48/apps")

# --- Uninstall Target Setup ---
# (Uninstall target setup remains the same)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cmake)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake"
    IMMEDIATE @ONLY
)
add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake"
    COMMENT "Uninstalling ${PROJECT_NAME} from ${CMAKE_INSTALL_PREFIX}..."
)
message(STATUS "Uninstall target added (run 'make uninstall' or similar in build dir)")

# Add check for QtAwesome submodule presence
if(NOT QTAWESOME_SOURCES)
    message(WARNING "QtAwesome sources not included. Ensure it's available or disable its usage.")
endif()
