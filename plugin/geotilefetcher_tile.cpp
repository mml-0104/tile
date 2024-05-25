#include "geotilefetcher_tile.h"
#include "geofiletilecache_tile.h"
#include "geotiledmapreply_tile.h"

#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkRequest>

#include <QtLocation/private/qgeotilespec_p.h>

QT_BEGIN_NAMESPACE

GeoTileFetcherTile::GeoTileFetcherTile(GeoTiledMappingManagerEngineTile *parent) :
    QGeoTileFetcher(parent),m_engine(parent)
{
    m_missCache.setMaxCost(1000);
}

QGeoTiledMapReply *GeoTileFetcherTile::getTileImage(const QGeoTileSpec &spec)
{
    if(m_missCache.contains(spec))
        return new QGeoTiledMapReply(QGeoTiledMapReply::UnknownError,QStringLiteral("Tile not found"));

    const QString directory = m_engine->cacheDirectory();
    GeoFileTileCacheTile *tileCache = qobject_cast<GeoFileTileCacheTile*>(m_engine->tileCache());
    QString filepath=tileCache->tileSpecToFilename(spec,QStringLiteral("*"),directory);
    GeoTiledMapReplyTile *reply = new GeoTiledMapReplyTile(filepath,spec);
    QMetaObject::invokeMethod(reply,"startHandle",Qt::QueuedConnection);
    return reply;
}

void GeoTileFetcherTile::handleReply(QGeoTiledMapReply *reply, const QGeoTileSpec &spec)
{
    if (reply->error() == QGeoTiledMapReply::NoError) {
        emit tileFinished(spec, reply->mapImageData(), reply->mapImageFormat());
    } else {
        m_missCache.insert(spec,0);
        emit tileError(spec, reply->errorString());
    }

    reply->deleteLater();
}
QT_END_NAMESPACE
