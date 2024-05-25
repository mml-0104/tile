// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOSERVICEPROVIDER_TIANDITU_H
#define QGEOSERVICEPROVIDER_TIANDITU_H

#include <qgeoserviceproviderfactory.h>
#include <QObject>

QT_BEGIN_NAMESPACE

class GeoServiceProviderFactoryTile : public QObject, public QGeoServiceProviderFactory
{
    Q_OBJECT
    Q_INTERFACES(QGeoServiceProviderFactory)
    Q_PLUGIN_METADATA(IID "org.qt-project.qt.geoservice.serviceproviderfactory/6.0"
                      FILE "tile_plugin.json")

public:
    QGeoMappingManagerEngine *createMappingManagerEngine(const QVariantMap &parameters,
                                                         QGeoServiceProvider::Error *error,
                                                         QString *errorString) const override;
};

QT_END_NAMESPACE

#endif
