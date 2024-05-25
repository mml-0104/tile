// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "geoserviceproviderplugin_tile.h"
#include "geotiledmappingmanagerengine_tile.h"

QT_BEGIN_NAMESPACE

QGeoMappingManagerEngine *GeoServiceProviderFactoryTile::createMappingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new GeoTiledMappingManagerEngineTile(parameters, error, errorString);
}


QT_END_NAMESPACE
