#include "geofiletilecache.h"

#include <QtLocation/private/qgeotilespec_p.h>
#include <QtLocation/private/qgeomappingmanager_p.h>

#include <QDir>
#include <QStandardPaths>
#include <QMetaType>
#include <QPixmap>
#include <QDebug>

QT_BEGIN_NAMESPACE

class QGeoCachedTileMemory
{
public:
    ~QGeoCachedTileMemory()
    {
        if (cache)
            cache->evictFromMemoryCache(this);
    }

    QGeoTileSpec spec;
    GeoFileTileCache *cache;
    QByteArray bytes;
    QString format;
};

void Cache3QTileEvictionPolicy::aboutToBeRemoved(const QGeoTileSpec &key, QSharedPointer<GeoCachedTileDisk> obj)
{
    Q_UNUSED(key);
    // set the cache pointer to zero so we can't call evictFromDiskCache
    obj->cache = nullptr;
}

void Cache3QTileEvictionPolicy::aboutToBeEvicted(const QGeoTileSpec &key, QSharedPointer<GeoCachedTileDisk> obj)
{
    Q_UNUSED(key);
    Q_UNUSED(obj);
    // leave the pointer set if it's a real eviction
}

GeoCachedTileDisk::~GeoCachedTileDisk()
{
    if (cache)
        cache->evictFromDiskCache(this);
}

GeoFileTileCache::GeoFileTileCache(const QString &directory, QObject *parent)
    : QAbstractGeoTileCache(parent), directory_(directory)
{
}

void GeoFileTileCache::init()
{
    const QString basePath = baseCacheDirectory() + QLatin1String("QtLocation/");

    // delete old tiles from QtLocation 5.7 or prior
    // Newer version use plugin-specific subdirectories, versioned with qt version so those are not affected.
    // TODO Remove cache cleanup in Qt 6
    QDir baseDir(basePath);
    if (baseDir.exists()) {
        const QStringList oldCacheFiles = baseDir.entryList(QDir::Files);
        for (const QString& file : oldCacheFiles)
            baseDir.remove(file);
        const QStringList oldCacheDirs = { QStringLiteral("osm"), QStringLiteral("mapbox"), QStringLiteral("here") };
        for (const QString& d : oldCacheDirs) {
            QDir oldCacheDir(basePath + QLatin1Char('/') + d);
            if (oldCacheDir.exists())
                oldCacheDir.removeRecursively();
        }
    }

    if (directory_.isEmpty()) {
        directory_ = baseLocationCacheDirectory();
        qWarning() << "Plugin uses uninitialized GeoFileTileCache directory which was deleted during startup";
    }

    const bool directoryCreated = QDir::root().mkpath(directory_);
    if (!directoryCreated)
        qWarning() << "Failed to create cache directory " << directory_;

    // default values
    if (!isDiskCostSet_) { // If setMaxDiskUsage has not been called yet
        if (costStrategyDisk_ == ByteSize)
            setMaxDiskUsage(50 * 1024 * 1024);
        else
            setMaxDiskUsage(1000);
    }

    if (!isMemoryCostSet_) { // If setMaxMemoryUsage has not been called yet
        if (costStrategyMemory_ == ByteSize)
            setMaxMemoryUsage(3 * 1024 * 1024);
        else
            setMaxMemoryUsage(100);
    }

    if (!isTextureCostSet_) { // If setExtraTextureUsage has not been called yet
        if (costStrategyTexture_ == ByteSize)
            setExtraTextureUsage(6 * 1024 * 1024);
        else
            setExtraTextureUsage(30); // byte size of texture is >> compressed image, hence unitary cost should be lower
    }

    loadTiles();
}

void GeoFileTileCache::loadTiles()
{
    QStringList formats;
    formats << QLatin1String("*.*");

    QDir dir(directory_);
    const QStringList files = dir.entryList(formats, QDir::Files);
#if 0 // workaround for QTBUG-60581
    // Method:
    // 1. read each queue file then, if each file exists, deserialize the data into the appropriate
    // cache queue.
    for (int i = 1; i<=4; i++) {
        QString filename = dir.filePath(QString::fromLatin1("queue") + QString::number(i));
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
            continue;
        QList<QSharedPointer<GeoCachedTileDisk> > queue;
        QList<QGeoTileSpec> specs;
        QList<int> costs;
        while (!file.atEnd()) {
            QByteArray line = file.readLine().trimmed();
            QString filename = QString::fromLatin1(line.constData(), line.length());
            if (dir.exists(filename)){
                files.removeOne(filename);
                QGeoTileSpec spec = filenameToTileSpec(filename);
                if (spec.zoom() == -1)
                    continue;
                QSharedPointer<GeoCachedTileDisk> tileDisk(new GeoCachedTileDisk);
                tileDisk->filename = dir.filePath(filename);
                tileDisk->cache = this;
                tileDisk->spec = spec;
                QFileInfo fi(tileDisk->filename);
                specs.append(spec);
                queue.append(tileDisk);
                if (costStrategyDisk_ == ByteSize)
                    costs.append(fi.size());
                else
                    costs.append(1);

            }
        }

        diskCache_.deserializeQueue(i, specs, queue, costs);
        file.close();
    }
#endif
    // 2. remaining tiles that aren't registered in a queue get pushed into cache here
    // this is a backup, in case the queue manifest files get deleted or out of sync due to
    // the application not closing down properly
    for (const auto &file : files) {
        QGeoTileSpec spec = filenameToTileSpec(file);
        if (spec.zoom() == -1)
            continue;
        QString filename = dir.filePath(file);
        addToDiskCache(spec, filename);
    }
}

GeoFileTileCache::~GeoFileTileCache()
{
#if 0 // workaround for QTBUG-60581
    // write disk cache queues to disk
    QDir dir(directory_);
    for (int i = 1; i<=4; i++) {
        QString filename = dir.filePath(QString::fromLatin1("queue") + QString::number(i));
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly)){
            qWarning() << "Unable to write tile cache file " << filename;
            continue;
        }
        QList<QSharedPointer<GeoCachedTileDisk> > queue;
        diskCache_.serializeQueue(i, queue);
        for (const QSharedPointer<GeoCachedTileDisk> &tile : queue) {
            if (tile.isNull())
                continue;

            // we just want the filename here, not the full path
            int index = tile->filename.lastIndexOf(QLatin1Char('/'));
            QByteArray filename = tile->filename.mid(index + 1).toLatin1() + '\n';
            file.write(filename);
        }
        file.close();
    }
#endif
}

void GeoFileTileCache::printStats()
{
    textureCache_.printStats();
    memoryCache_.printStats();
    diskCache_.printStats();
}

void GeoFileTileCache::setMaxDiskUsage(int diskUsage)
{
    diskCache_.setMaxCost(diskUsage);
    isDiskCostSet_ = true;
}

int GeoFileTileCache::maxDiskUsage() const
{
    return diskCache_.maxCost();
}

int GeoFileTileCache::diskUsage() const
{
    return diskCache_.totalCost();
}

void GeoFileTileCache::setMaxMemoryUsage(int memoryUsage)
{
    memoryCache_.setMaxCost(memoryUsage);
    isMemoryCostSet_ = true;
}

int GeoFileTileCache::maxMemoryUsage() const
{
    return memoryCache_.maxCost();
}

int GeoFileTileCache::memoryUsage() const
{
    return memoryCache_.totalCost();
}

void GeoFileTileCache::setExtraTextureUsage(int textureUsage)
{
    extraTextureUsage_ = textureUsage;
    textureCache_.setMaxCost(minTextureUsage_ + extraTextureUsage_);
    isTextureCostSet_ = true;
}

void GeoFileTileCache::setMinTextureUsage(int textureUsage)
{
    minTextureUsage_ = textureUsage;
    textureCache_.setMaxCost(minTextureUsage_ + extraTextureUsage_);
}

int GeoFileTileCache::maxTextureUsage() const
{
    return textureCache_.maxCost();
}

int GeoFileTileCache::minTextureUsage() const
{
    return minTextureUsage_;
}


int GeoFileTileCache::textureUsage() const
{
    return textureCache_.totalCost();
}

void GeoFileTileCache::clearAll()
{
    textureCache_.clear();
    memoryCache_.clear();
    diskCache_.clear();
    QDir dir(directory_);
    dir.setNameFilters(QStringList() << QLatin1String("*-*-*-*.*"));
    dir.setFilter(QDir::Files);
    for (const QString &dirFile : dir.entryList()) {
        dir.remove(dirFile);
    }
}

void GeoFileTileCache::clearMapId(const int mapId)
{
    for (const QGeoTileSpec &k : diskCache_.keys())
        if (k.mapId() == mapId)
            diskCache_.remove(k, true);
    for (const QGeoTileSpec &k : memoryCache_.keys())
        if (k.mapId() == mapId)
            memoryCache_.remove(k);
    for (const QGeoTileSpec &k : textureCache_.keys())
        if (k.mapId() == mapId)
            textureCache_.remove(k);

    // TODO: It seems the cache leaves residues, like some tiles do not get picked up.
    // After the above calls, files that shouldnt be left behind are still on disk.
    // Do an additional pass and make sure what has to be deleted gets deleted.
    QDir dir(directory_);
    QStringList formats;
    formats << QLatin1String("*.*");
    const QStringList files = dir.entryList(formats, QDir::Files);
    qWarning() << "Old tile data detected. Cache eviction left out "<< files.size() << "tiles";
    for (const QString &tileFileName : files) {
        QGeoTileSpec spec = filenameToTileSpec(tileFileName);
        if (spec.mapId() != mapId)
            continue;
        QFile::remove(dir.filePath(tileFileName));
    }
}

void GeoFileTileCache::setCostStrategyDisk(QAbstractGeoTileCache::CostStrategy costStrategy)
{
    costStrategyDisk_ = costStrategy;
}

QAbstractGeoTileCache::CostStrategy GeoFileTileCache::costStrategyDisk() const
{
    return costStrategyDisk_;
}

void GeoFileTileCache::setCostStrategyMemory(QAbstractGeoTileCache::CostStrategy costStrategy)
{
    costStrategyMemory_ = costStrategy;
}

QAbstractGeoTileCache::CostStrategy GeoFileTileCache::costStrategyMemory() const
{
    return costStrategyMemory_;
}

void GeoFileTileCache::setCostStrategyTexture(QAbstractGeoTileCache::CostStrategy costStrategy)
{
    costStrategyTexture_ = costStrategy;
}

QAbstractGeoTileCache::CostStrategy GeoFileTileCache::costStrategyTexture() const
{
    return costStrategyTexture_;
}

QSharedPointer<QGeoTileTexture> GeoFileTileCache::get(const QGeoTileSpec &spec)
{
    QSharedPointer<QGeoTileTexture> tt = getFromMemory(spec);
    if (tt)
        return tt;
    return getFromDisk(spec);
}

void GeoFileTileCache::insert(const QGeoTileSpec &spec,
                               const QByteArray &bytes,
                               const QString &format,
                               QAbstractGeoTileCache::CacheAreas areas)
{
    if (bytes.isEmpty())
        return;

    if (areas & QAbstractGeoTileCache::DiskCache) {
        QString filename = tileSpecToFilename(spec, format, directory_);
        addToDiskCache(spec, filename, bytes);
    }

    if (areas & QAbstractGeoTileCache::MemoryCache) {
        addToMemoryCache(spec, bytes, format);
    }

    /* inserts do not hit the texture cache -- this actually reduces overall
     * cache hit rates because many tiles come too late to be useful
     * and act as a poison */
}

QString GeoFileTileCache::tileSpecToFilenameDefault(const QGeoTileSpec &spec, const QString &format, const QString &directory)
{
    QString filename = spec.plugin();
    filename += QLatin1String("-");
    filename += QString::number(spec.mapId());
    filename += QLatin1String("-");
    filename += QString::number(spec.zoom());
    filename += QLatin1String("-");
    filename += QString::number(spec.x());
    filename += QLatin1String("-");
    filename += QString::number(spec.y());

    //Append version if real version number to ensure backwards compatibility and eviction of old tiles
    if (spec.version() != -1) {
        filename += QLatin1String("-");
        filename += QString::number(spec.version());
    }

    filename += QLatin1String(".");
    filename += format;

    QDir dir = QDir(directory);

    return dir.filePath(filename);
}

QGeoTileSpec GeoFileTileCache::filenameToTileSpecDefault(const QString &filename)
{
    QGeoTileSpec emptySpec;

    const QStringList parts = filename.split(QLatin1Char('.'));

    if (parts.length() != 2)
        return emptySpec;

    const QString name = parts.at(0);
    const QStringList fields = name.split(QLatin1Char('-'));

    qsizetype length = fields.length();
    if (length != 5 && length != 6)
        return emptySpec;

    QList<int> numbers;

    bool ok = false;
    for (qsizetype i = 1; i < length; ++i) {
        ok = false;
        int value = fields.at(i).toInt(&ok);
        if (!ok)
            return emptySpec;
        numbers.append(value);
    }

    //File name without version, append default
    if (numbers.length() < 5)
        numbers.append(-1);

    return QGeoTileSpec(fields.at(0),
                        numbers.at(0),
                        numbers.at(1),
                        numbers.at(2),
                        numbers.at(3),
                        numbers.at(4));
}

void GeoFileTileCache::evictFromDiskCache(GeoCachedTileDisk *td)
{
    Q_UNUSED(td);
    //QFile::remove(td->filename);  //Holy shit, Tiling files will be deleted by gefiletilecache
}

void GeoFileTileCache::evictFromMemoryCache(QGeoCachedTileMemory * /* tm  */)
{
}

QSharedPointer<GeoCachedTileDisk> GeoFileTileCache::addToDiskCache(const QGeoTileSpec &spec, const QString &filename)
{
    QSharedPointer<GeoCachedTileDisk> td(new GeoCachedTileDisk);
    td->spec = spec;
    td->filename = filename;
    td->cache = this;

    int cost = 1;
    if (costStrategyDisk_ == ByteSize) {
        QFileInfo fi(filename);
        cost = fi.size();
    }
    diskCache_.insert(spec, td, cost);
    return td;
}

bool GeoFileTileCache::addToDiskCache(const QGeoTileSpec &spec, const QString &filename, const QByteArray &bytes)
{
    QSharedPointer<GeoCachedTileDisk> td(new GeoCachedTileDisk);
    td->spec = spec;
    td->filename = filename;
    td->cache = this;

    int cost = 1;
    if (costStrategyDisk_ == ByteSize)
        cost = bytes.size();

    if (diskCache_.insert(spec, td, cost)) {
        QFile file(filename);
        file.open(QIODevice::WriteOnly);
        file.write(bytes);
        file.close();
        return true;
    }
    return false;
}

void GeoFileTileCache::addToMemoryCache(const QGeoTileSpec &spec, const QByteArray &bytes, const QString &format)
{
    if (isTileBogus(bytes))
        return;

    QSharedPointer<QGeoCachedTileMemory> tm(new QGeoCachedTileMemory);
    tm->spec = spec;
    tm->cache = this;
    tm->bytes = bytes;
    tm->format = format;

    int cost = 1;
    if (costStrategyMemory_ == ByteSize)
        cost = bytes.size();
    memoryCache_.insert(spec, tm, cost);
}

QSharedPointer<QGeoTileTexture> GeoFileTileCache::addToTextureCache(const QGeoTileSpec &spec, const QImage &image)
{
    QSharedPointer<QGeoTileTexture> tt(new QGeoTileTexture);
    tt->spec = spec;
    tt->image = image;

    int cost = 1;
    if (costStrategyTexture_ == ByteSize)
        cost = image.width() * image.height() * image.depth() / 8;
    textureCache_.insert(spec, tt, cost);

    return tt;
}

QSharedPointer<QGeoTileTexture> GeoFileTileCache::getFromMemory(const QGeoTileSpec &spec)
{
    QSharedPointer<QGeoTileTexture> tt = textureCache_.object(spec);
    if (tt)
        return tt;

    QSharedPointer<QGeoCachedTileMemory> tm = memoryCache_.object(spec);
    if (tm) {
        QImage image;
        if (!image.loadFromData(tm->bytes)) {
            handleError(spec, QLatin1String("Problem with tile image"));
            return QSharedPointer<QGeoTileTexture>();
        }
        QSharedPointer<QGeoTileTexture> tt = addToTextureCache(spec, image);
        if (tt)
            return tt;
    }
    return QSharedPointer<QGeoTileTexture>();
}

QSharedPointer<QGeoTileTexture> GeoFileTileCache::getFromDisk(const QGeoTileSpec &spec)
{
    QSharedPointer<GeoCachedTileDisk> td = diskCache_.object(spec);
    if (td) {
        const QString format = QFileInfo(td->filename).suffix();
        QFile file(td->filename);
        file.open(QIODevice::ReadOnly);
        QByteArray bytes = file.readAll();
        file.close();

        QImage image;
        // Some tiles from the servers could be valid images but the tile fetcher
        // might be able to recognize them as tiles that should not be shown.
        // If that's the case, the tile fetcher should write "NoRetry" inside the file.
        if (isTileBogus(bytes)) {
            QSharedPointer<QGeoTileTexture> tt(new QGeoTileTexture);
            tt->spec = spec;
            tt->image = image;
            return tt;
        }

        // This is a truly invalid image. The fetcher should try again.
        if (!image.loadFromData(bytes)) {
            handleError(spec, QLatin1String("Problem with tile image"));
            return QSharedPointer<QGeoTileTexture>();
        }

        // Converting it here, instead of in each QSGTexture::bind()
        if (image.format() != QImage::Format_RGB32 && image.format() != QImage::Format_ARGB32_Premultiplied)
            image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);

        addToMemoryCache(spec, bytes, format);
        QSharedPointer<QGeoTileTexture> tt = addToTextureCache(td->spec, image);
        if (tt)
            return tt;
    }

    return QSharedPointer<QGeoTileTexture>();
}

bool GeoFileTileCache::isTileBogus(const QByteArray &bytes) const
{
    if (bytes.size() == 7 && bytes == QByteArrayLiteral("NoRetry"))
        return true;
    return false;
}

QString GeoFileTileCache::tileSpecToFilename(const QGeoTileSpec &spec, const QString &format, const QString &directory) const
{
    return tileSpecToFilenameDefault(spec, format, directory);
}

QGeoTileSpec GeoFileTileCache::filenameToTileSpec(const QString &filename) const
{
    return filenameToTileSpecDefault(filename);
}

QString GeoFileTileCache::directory() const
{
    return directory_;
}

QT_END_NAMESPACE
