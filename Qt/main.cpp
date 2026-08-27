#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "serialcontroller.h"
#include "SensorSurCar.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);


    qmlRegisterType<SensorSurCar>("SensorUI", 1, 0, "SensorSurCar");

    SerialController serialController;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(
        "serialController", &serialController);

    engine.loadFromModule("Parking_Sensor", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
