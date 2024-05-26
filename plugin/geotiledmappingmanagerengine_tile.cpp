#include "geotiledmappingmanagerengine_tile.h"
#include "geofiletilecache_tile.h"
#include "geotilefetcher_tile.h"

#include <QtLocation/private/qgeotiledmappingmanagerengine_p_p.h>
#include <QtLocation/private/qgeocameracapabilities_p.h>
#include <QtLocation/private/qgeomaptype_p.h>
#include <QtLocation/private/qgeotiledmap_p.h>
#include <QtLocation/private/qgeocameradata_p.h>


QT_BEGIN_NAMESPACE

GeoTiledMappingManagerEngineTile::GeoTiledMappingManagerEngineTile(const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString):
    QGeoTiledMappingManagerEngine()
{
    QGeoCameraCapabilities cameraCaps;
    cameraCaps.setMinimumZoomLevel(0.0);
    cameraCaps.setMaximumZoomLevel(19.0);
    cameraCaps.setSupportsBearing(true);
    cameraCaps.setSupportsTilting(true);
    cameraCaps.setMinimumTilt(0);
    cameraCaps.setMaximumTilt(80);
    cameraCaps.setMinimumFieldOfView(20.0);
    cameraCaps.setMaximumFieldOfView(120.0);
    cameraCaps.setOverzoomEnabled(true);
    setCameraCapabilities(cameraCaps);

    setTileSize(QSize(256, 256));

    if (parameters.contains(QStringLiteral("tile.mapping.copyright")))
        m_customCopyright = parameters.value(QStringLiteral("tile.mapping.copyright")).toString();

    if (parameters.contains(QStringLiteral("tile.mapping.name")))
        m_mapName = parameters.value(QStringLiteral("tile.mapping.name")).toString();

    if(m_mapName.isEmpty())
        m_mapName = QStringLiteral("CustomTileMap");

    if (parameters.contains(QStringLiteral("tile.mapping.tile_prefix")))
        m_tilePrefix = parameters.value(QStringLiteral("tile.mapping.tile_prefix")).toString();

    int minimumZoomLevel = 0.0;
    int maximumZoomLevel = 19.0;

    if (parameters.contains(QStringLiteral("tile.mapping.minzoomlevel")))
        minimumZoomLevel = parameters.value(QStringLiteral("tile.mapping.minzoomlevel")).toInt();

    if (parameters.contains(QStringLiteral("tile.mapping.maxzoomlevel")))
        maximumZoomLevel = parameters.value(QStringLiteral("tile.mapping.maxzoomlevel")).toInt();

    if (parameters.contains(QStringLiteral("tile.mapping.precachezoomlevel")))
        m_preCacheZoomLevel = parameters.value(QStringLiteral("tile.mapping.precachezoomlevel")).toInt();
    else
        m_preCacheZoomLevel = 0;

    m_preCacheZoomLevel = qMax(qMax(m_preCacheZoomLevel,minimumZoomLevel),0);

    if(minimumZoomLevel >= 0 && minimumZoomLevel <= 25
        && maximumZoomLevel >= 0 && maximumZoomLevel <=25
        && minimumZoomLevel < maximumZoomLevel)
    {
        cameraCaps.setMinimumZoomLevel(minimumZoomLevel);
        cameraCaps.setMaximumZoomLevel(maximumZoomLevel);
    }

    QList<QGeoMapType> mapTypes;

    const QByteArray pluginName = "tile";

    mapTypes << QGeoMapType(
        QGeoMapType::CustomMap,
        m_mapName,
        "Custom offline tile map",
        false,
        false,
        1,
        pluginName,
        cameraCaps);

    setSupportedMapTypes(mapTypes);

    if (parameters.contains(QStringLiteral("tile.mapping.cache.directory"))) {
        m_cacheDirectory = parameters.value(QStringLiteral("tile.mapping.cache.directory")).toString();
    } else {
        // managerName() is not yet set, we have to hardcode the plugin name below
        m_cacheDirectory = QAbstractGeoTileCache::baseLocationCacheDirectory() + QLatin1String("tile");
    }

    if (parameters.contains(QStringLiteral("tile.mapping.cache.hierarchy")))
        m_cacheHierarchy = parameters.value(QStringLiteral("tile.mapping.cache.hierarchy")).toInt();
    else
        m_cacheHierarchy = 0;

    QAbstractGeoTileCache *tileCache = new GeoFileTileCacheTile(m_cacheDirectory,this);

    /*
     * Disk cache setup -- defaults to ByteSize (old behavior)
     */
    if (parameters.contains(QStringLiteral("tile.mapping.cache.disk.cost_strategy"))) {
        QString cacheStrategy = parameters.value(QStringLiteral("tile.mapping.cache.disk.cost_strategy")).toString().toLower();
        if (cacheStrategy == QLatin1String("bytesize"))
            tileCache->setCostStrategyDisk(QAbstractGeoTileCache::ByteSize);
        else
            tileCache->setCostStrategyDisk(QAbstractGeoTileCache::Unitary);
    } else {
        tileCache->setCostStrategyDisk(QAbstractGeoTileCache::ByteSize);
    }
    if (parameters.contains(QStringLiteral("tile.mapping.cache.disk.size"))) {
        bool ok = false;
        int cacheSize = parameters.value(QStringLiteral("tile.mapping.cache.disk.size")).toString().toInt(&ok);
        if (ok)
            tileCache->setMaxDiskUsage(cacheSize);
    }

    /*
     * Memory cache setup -- defaults to ByteSize (old behavior)
     */
    if (parameters.contains(QStringLiteral("tile.mapping.cache.memory.cost_strategy"))) {
        QString cacheStrategy = parameters.value(QStringLiteral("tile.mapping.cache.memory.cost_strategy")).toString().toLower();
        if (cacheStrategy == QLatin1String("bytesize"))
            tileCache->setCostStrategyMemory(QAbstractGeoTileCache::ByteSize);
        else
            tileCache->setCostStrategyMemory(QAbstractGeoTileCache::Unitary);
    } else {
        tileCache->setCostStrategyMemory(QAbstractGeoTileCache::ByteSize);
    }
    if (parameters.contains(QStringLiteral("tile.mapping.cache.memory.size"))) {
        bool ok = false;
        int cacheSize = parameters.value(QStringLiteral("tile.mapping.cache.memory.size")).toString().toInt(&ok);
        if (ok)
            tileCache->setMaxMemoryUsage(cacheSize);
    }

    /*
     * Texture cache setup -- defaults to ByteSize (old behavior)
     */
    if (parameters.contains(QStringLiteral("tile.mapping.cache.texture.cost_strategy"))) {
        QString cacheStrategy = parameters.value(QStringLiteral("tile.mapping.cache.texture.cost_strategy")).toString().toLower();
        if (cacheStrategy == QLatin1String("bytesize"))
            tileCache->setCostStrategyTexture(QAbstractGeoTileCache::ByteSize);
        else
            tileCache->setCostStrategyTexture(QAbstractGeoTileCache::Unitary);
    } else {
        tileCache->setCostStrategyTexture(QAbstractGeoTileCache::ByteSize);
    }
    if (parameters.contains(QStringLiteral("tile.mapping.cache.texture.size"))) {
        bool ok = false;
        int cacheSize = parameters.value(QStringLiteral("tile.mapping.cache.texture.size")).toString().toInt(&ok);
        if (ok)
            tileCache->setExtraTextureUsage(cacheSize);
    }

    //managerName() is not yet set, call later setTileCache in createMap, refer to GeoFileTileCacheTile::init
    //setTileCache(tileCache);
    m_tileCache=tileCache;

     /* TILE FETCHER */
    GeoTileFetcherTile *tileFetcher = new GeoTileFetcherTile(this);

    setTileFetcher(tileFetcher);

    /* PREFETCHING */
    if (parameters.contains(QStringLiteral("tile.mapping.prefetching_style"))) {
        const QString prefetchingMode = parameters.value(QStringLiteral("tile.mapping.prefetching_style")).toString();
        if (prefetchingMode == QStringLiteral("TwoNeighbourLayers"))
            m_prefetchStyle = QGeoTiledMap::PrefetchTwoNeighbourLayers;
        else if (prefetchingMode == QStringLiteral("OneNeighbourLayer"))
            m_prefetchStyle = QGeoTiledMap::PrefetchNeighbourLayer;
        else if (prefetchingMode == QStringLiteral("NoPrefetching"))
            m_prefetchStyle = QGeoTiledMap::NoPrefetching;
    }

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

GeoTiledMappingManagerEngineTile::~GeoTiledMappingManagerEngineTile()
{
}

QGeoMap *GeoTiledMappingManagerEngineTile::createMap()
{
    setTileCache(m_tileCache);
    QGeoTiledMap *map = new QGeoTiledMap(this,nullptr);
    map->setPrefetchStyle(m_prefetchStyle);
    return map;
}

void GeoTiledMappingManagerEngineTile::updateTileRequests(QGeoTiledMap *map,
                        const QSet<QGeoTileSpec> &tilesAdded,
                        const QSet<QGeoTileSpec> &tilesRemoved)
{
    QGeoTiledMappingManagerEngine::updateTileRequests(map,tilesAdded,tilesRemoved);
}

void GeoTiledMappingManagerEngineTile::engineTileFinished(const QGeoTileSpec &spec, const QByteArray &bytes, const QString &format)
{
    QGeoTiledMappingManagerEngine::engineTileFinished(spec,bytes,format);
}

void GeoTiledMappingManagerEngineTile::engineTileError(const QGeoTileSpec &spec, const QString &errorString)
{
    //QGeoTiledMappingManagerEngine::engineTileError(spec,errorString);

    Q_D(QGeoTiledMappingManagerEngine);

    QSet<QGeoTiledMap *> maps = d->tileHash_.value(spec);
    typedef QSet<QGeoTiledMap *>::const_iterator map_iter;
    map_iter map = maps.constBegin();
    map_iter mapEnd = maps.constEnd();
    for (; map != mapEnd; ++map) {
        QSet<QGeoTileSpec> tileSet = d->mapHash_.value(*map);

        tileSet.remove(spec);
        if (tileSet.isEmpty())
            d->mapHash_.remove(*map);
        else
            d->mapHash_.insert(*map, tileSet);
    }
    d->tileHash_.remove(spec);

    /* Do not notify the manager to retry
    for (map = maps.constBegin(); map != mapEnd; ++map) {
        //(*map)->requestManager()->tileError(spec, errorString);
    }
    */

    emit tileError(spec, errorString);
}
QT_END_NAMESPACE
