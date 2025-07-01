#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "bluetoothdevicemanager.h"
#include "mediaengine.h"
#include "bluetootha2dpmanager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    BluetoothDeviceManager btManager;
    MediaEngine mEngine;
    BluetoothA2DPManager a2dpManager;
    a2dpManager.initialize();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("btManager", &btManager);
    engine.rootContext()->setContextProperty("mEngine", &mEngine);
    engine.rootContext()->setContextProperty("a2dpManager",&a2dpManager);
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("SmartMultimediaPlayer", "Main");
    QObject *rootObject = engine.rootObjects().first();
    QObject *videoOutput = rootObject->findChild<QObject*>("videoOutputId");
    if (videoOutput) {
        videoOutput->setProperty("videoSink", QVariant::fromValue(mEngine.videoSink()));
    }
    return app.exec();
}
