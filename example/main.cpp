// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
#if 1 //0: osm test  1:tianditu test
    engine.load(QUrl(QStringLiteral("qrc:/tianditu.qml")));
#else
    engine.load(QUrl(QStringLiteral("qrc:/osm.qml")));
#endif
    return app.exec();
}

