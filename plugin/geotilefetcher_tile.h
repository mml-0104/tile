#ifndef GEOTILEFETCHERTILE_H
#define GEOTILEFETCHERTILE_H

#include "geotiledmappingmanagerengine_tile.h"

#include <QtLocation/private/qgeotilefetcher_p.h>

QT_BEGIN_NAMESPACE

class QGeoTiledMappingManagerEngine;
class QNetworkAccessManager;

class GeoTileFetcherTile : public QGeoTileFetcher
{
    Q_OBJECT
public:
    GeoTileFetcherTile(GeoTiledMappingManagerEngineTile *parent);

protected:
    QGeoTiledMapReply *getTileImage(const QGeoTileSpec &spec) override;
    void handleReply(QGeoTiledMapReply *reply, const QGeoTileSpec &spec) override;

    GeoTiledMappingManagerEngineTile *m_engine;
    QCache<QGeoTileSpec,bool> m_missCache;
};

#endif // GEOTILEFETCHERTILE_H
