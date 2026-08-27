cmake_minimum_required(VERSION 3.16)

project(Parking_Sensor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Multimedia)
find_package(Qt6 REQUIRED COMPONENTS Quick SerialPort)
find_package(Qt6 REQUIRED COMPONENTS SerialPort)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appParking_Sensor
    main.cpp
)

qt_add_qml_module(appParking_Sensor
    URI Parking_Sensor
    VERSION 1.0
    QML_FILES
        Main.qml
        QML_FILES
        QML_FILES
        RESOURCES
        QML_FILES CarWithSensors.qml
        SOURCES serialcontroller.h serialcontroller.cpp
        QML_FILES
        SOURCES SensorSurCar.h SensorSurCar.cpp
        QML_FILES Mybutton.qml
        QML_FILES CameraView.qml
        QML_FILES DashboardView.qml
        RESOURCES
        RESOURCES img/img.qrc
        QML_FILES LeftPanel.qml
        QML_FILES
        QML_FILES
        QML_FILES GearStatus.qml
        QML_FILES PanelHeader.qml
        QML_FILES
        QML_FILES
        QML_FILES ParkingSensorPanel.qml
        QML_FILES SonarArcs.qml
        QML_FILES VehicleInfo.qml
        QML_FILES VehicleState.qml
        QML_FILES DistanceInfo.qml
        QML_FILES DashboardTile.qml
        QML_FILES InfoRow.qml
        QML_FILES TPMS.qml
        QML_FILES TpmsValue.qml
        QML_FILES WeatherPanel.qml
        QML_FILES WeatherMetric.qml
        QML_FILES MusicPlayer.qml
        QML_FILES SonarWave.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appParking_Sensor PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appParking_Sensor
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appParking_Sensor
    PRIVATE Qt6::Quick Qt6::Multimedia Qt6::SerialPort
)


include(GNUInstallDirs)
install(TARGETS appParking_Sensor
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
