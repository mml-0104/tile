#ifndef GEOTILEDMAPPINGMANAGERENGINE_TILE_H
#define GEOTILEDMAPPINGMANAGERENGINE_TILE_H

#include <QtCore/QCache>
#include <QtLocation/QGeoServiceProvider>
#include <QtLocation/private/qgeotiledmappingmanagerengine_p.h>
#include <QtLocation/private/qgeofiletilecache_p.h>

QT_BEGIN_NAMESPACE

class QGeoTiledMappingManagerEnginePrivate;

class GeoTiledMappingManagerEngineTile : public QGeoTiledMappingManagerEngine
{
    Q_OBJECT
public:
    GeoTiledMappingManagerEngineTile(const QVariantMap &parameters,
                                     QGeoServiceProvider::Error *error, QString *errorString);
    ~GeoTiledMappingManagerEngineTile();

    QGeoMap *createMap() override;

    void updateTileRequests(QGeoTiledMap *map,
                                    const QSet<QGeoTileSpec> &tilesAdded,
                                    const QSet<QGeoTileSpec> &tilesRemoved) override;
protected Q_SLOTS:
    void engineTileFinished(const QGeoTileSpec &spec, const QByteArray &bytes, const QString &format) override;
    void engineTileError(const QGeoTileSpec &spec, const QString &errorString) override;

public:
    inline QString mapName() const;
    inline QString tilePrefix() const;
    inline QString cacheDirectory() const;
    inline int cacheHierarchy() const;
    inline int preCacheZoomLevel() const;
private:
    QAbstractGeoTileCache *m_tileCache;
    QString m_mapName;
    int m_preCacheZoomLevel;
    QString m_tilePrefix;
    QString m_customCopyright;
    QString m_cacheDirectory;
    int m_cacheHierarchy;
};

inline QString GeoTiledMappingManagerEngineTile::mapName() const
{
    return m_mapName;
}

inline QString GeoTiledMappingManagerEngineTile::tilePrefix() const
{
    return m_tilePrefix;
}

inline QString GeoTiledMappingManagerEngineTile::cacheDirectory() const
{
    return m_cacheDirectory;
}

inline int GeoTiledMappingManagerEngineTile::cacheHierarchy() const
{
    return m_cacheHierarchy;
}

inline int GeoTiledMappingManagerEngineTile::preCacheZoomLevel() const
{
    return m_preCacheZoomLevel;
}

QT_END_NAMESPACE

#endif // GEOTILEDMAPPINGMANAGERENGINE_TILE_H
