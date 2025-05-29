#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "bluetoothdevicemanager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    BluetoothDeviceManager btManager;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("btManager", &btManager);
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("SmartMultimediaPlayer", "Main");

    return app.exec();
}
