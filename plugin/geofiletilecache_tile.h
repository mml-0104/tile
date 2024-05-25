#ifndef GEOFILETILECACHE_TILE_H
#define GEOFILETILECACHE_TILE_H

#include "geofiletilecache.h"
//#include <QtLocation/private/qgeofiletilecache_p.h>
#include <QtCore/QSharedPointer>
#include <QSharedDataPointer>
#include "geotiledmappingmanagerengine_tile.h"

QT_BEGIN_NAMESPACE

class GeoCachedTileDisk;

class GeoFileTileCacheTile : public GeoFileTileCache
{
    Q_OBJECT
public:
    explicit GeoFileTileCacheTile(const QString &directory = QString(), GeoTiledMappingManagerEngineTile *parent = nullptr);
    ~GeoFileTileCacheTile();

    void clearAll() override;
    void clearMapId(const int mapId);

    QSharedPointer<QGeoTileTexture> get(const QGeoTileSpec &spec) override;

protected:
    bool addToDiskCache(const QGeoTileSpec &spec, const QString &filename, const QByteArray &bytes);
    bool addToDiskCacheCond(const QGeoTileSpec &spec, const QString &filename);
    QSharedPointer<QGeoTileTexture> getFromHardDisk(const QGeoTileSpec &spec);
public:
    QString tileSpecToFilename(const QGeoTileSpec &spec, const QString &format, const QString &directory) const override;
protected:
    QGeoTileSpec filenameToTileSpec(const QString &filename) const override;
    QGeoTileSpec filenameToTileSpec(int z, const QString &filename) const;
    QGeoTileSpec filenameToTileSpec(int z, int x, const QString &filename) const;
    QGeoTileSpec filenameToTileSpec(int z, int y, const QString &filename, int) const;
    void init() override;

    void loadTiles();

    void loadTilesDefault(); //cache/{prefix}{z}-{x}-{y}.{image}
    void loadTilesStyle1();  //cache/{z}/{x}-{y}.{image}
    void loadTilesStyle2();  //cache/{z}/{x}/{y}.{image}
    void loadTilesStyle3();  //cache/{z}/{y}/{x}.{image}

    bool diskCacheFilled();
private:
    GeoTiledMappingManagerEngineTile *m_engine;
    QString m_pluginString;
};
QT_END_NAMESPACE

#endif // GEOFILETILECACHE_TILE_H
