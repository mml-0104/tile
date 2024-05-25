#include "geofiletilecache_tile.h"
#include <QDir>


QT_BEGIN_NAMESPACE

GeoFileTileCacheTile::GeoFileTileCacheTile(const QString &directory, GeoTiledMappingManagerEngineTile *parent)
    : GeoFileTileCache{directory,parent},m_engine(parent)
{
    Q_ASSERT(m_engine);
}

GeoFileTileCacheTile::~GeoFileTileCacheTile()
{
}

QString GeoFileTileCacheTile::tileSpecToFilename(const QGeoTileSpec &spec, const QString &format,
                                                    const QString &directory) const
{
    QString filename;
    QDir dir = QDir(directory);
    int hierarchy = m_engine->cacheHierarchy();
    switch(hierarchy)
    {
    default:
    {
        //prefix contains mapid
        const QString& prefix = m_engine->tilePrefix();
        if(!prefix.isEmpty())
            filename += prefix;
        filename += QString::number(spec.zoom());
        filename += QLatin1String("-");
        filename += QString::number(spec.x());
        filename += QLatin1String("-");
        filename += QString::number(spec.y());
        break;
    }
    case 1:
    {
        dir=QDir(directory+"/"+QString::number(spec.zoom()));
        filename += QString::number(spec.x());
        filename += QLatin1String("-");
        filename += QString::number(spec.y());
        break;
    }
    case 2:
    {
        dir=QDir(directory+"/"+QString::number(spec.zoom())+"/"+QString::number(spec.x()));
        filename += QString::number(spec.y());
        break;
    }
    case 3:
    {
        dir=QDir(directory+"/"+QString::number(spec.zoom())+"/"+QString::number(spec.y()));
        filename += QString::number(spec.x());
    }
    }
    //Append version if real version number to ensure backwards compatibility and eviction of old tiles
    if (spec.version() != -1) {
        filename += QLatin1String("-");
        filename += QString::number(spec.version());
    }

    filename += QLatin1String(".");
    filename += format;

    return dir.filePath(filename);
}

QSharedPointer<QGeoTileTexture> GeoFileTileCacheTile::get(const QGeoTileSpec &spec)
{
    return GeoFileTileCache::get(spec);
}

bool GeoFileTileCacheTile::addToDiskCache(const QGeoTileSpec &spec, const QString &filename, const QByteArray &bytes)
{
    QSharedPointer<GeoCachedTileDisk> td(new GeoCachedTileDisk);
    td->spec = spec;
    td->filename = filename;
    td->cache = this;

    int cost = 1;
    if (costStrategyDisk_ == ByteSize)
        cost = bytes.size();

    if (diskCache_.insert(spec, td, cost)) {
        //Asynchronously load the tile file and send it back here, no need to save the image file
        //Only tiles obtained through the network need to be written to the file
        /*QFile file(filename);
        file.open(QIODevice::WriteOnly);
        file.write(bytes);
        file.close();*/
        return true;
    }
    return false;
}

bool GeoFileTileCacheTile::addToDiskCacheCond(const QGeoTileSpec &spec, const QString &filename)
{
    bool filled = diskCacheFilled();
    if(!filled){
        GeoFileTileCache::addToDiskCache(spec, filename);
    }
    return filled;
}

bool GeoFileTileCacheTile::diskCacheFilled()
{
    int rate = ((uint64_t)diskUsage())*100/maxDiskUsage();
    return rate > 98;
}

void GeoFileTileCacheTile::clearAll()
{
    textureCache_.clear();
    memoryCache_.clear();
    diskCache_.clear();
}

void GeoFileTileCacheTile::clearMapId(const int mapId)
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
}

QSharedPointer<QGeoTileTexture> GeoFileTileCacheTile::getFromHardDisk(const QGeoTileSpec &spec)
{
    QString filepath = tileSpecToFilename(spec,QStringLiteral("*"),directory_);
    int index = filepath.lastIndexOf(QLatin1Char('/'));
    if(index == -1)
        return QSharedPointer<QGeoTileTexture>();

    QDir dir(filepath.left(index));
    QString filename = filepath.right(filepath.size()-index-1);
    QStringList tiles = dir.entryList({filename});
    if(!tiles.size())
        return QSharedPointer<QGeoTileTexture>();

    QFile file(dir.absoluteFilePath(tiles.first()));
    if (!file.open(QIODevice::ReadOnly))
        return QSharedPointer<QGeoTileTexture>();
    QByteArray bytes = file.readAll();
    file.close();

    QImage image;
    if (!image.loadFromData(bytes)) {
        handleError(spec, QLatin1String("Problem with tile image"));
        return QSharedPointer<QGeoTileTexture>();
    }

    addToMemoryCache(spec, bytes, QString());
    return addToTextureCache(spec, image);
}

void GeoFileTileCacheTile::init()
{
    if (directory_.isEmpty()) {
        directory_ = baseLocationCacheDirectory();
        qWarning() << "Plugin uses uninitialized QGeoFileTileCache directory which was deleted during startup";
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

    if(m_engine->managerVersion() != -1)
        m_pluginString = m_engine->managerName() + QLatin1Char('_') + QString::number(m_engine->managerVersion());
    else
        m_pluginString = "tile_100";
    loadTiles();
    //printStats();
    if(!diskUsage())
        qWarning() << "No offline tile data is available in directory " << directory_;
}

void GeoFileTileCacheTile::loadTiles()
{
    if(maxDiskUsage() < 100*1024) {
        qWarning() << "The maximum disk cache capacity is insufficient ";
    }

    int hierarchy = m_engine->cacheHierarchy();
    switch(hierarchy)
    {
    case 1: loadTilesStyle1();break;
    case 2: loadTilesStyle2();break;
    case 3: loadTilesStyle3();break;
    default:loadTilesDefault();break;
    }
}

//default cache/{prefix}{z}-{x}-{y}.{image}
void GeoFileTileCacheTile::loadTilesDefault()
{
    QStringList formats;
    formats << QLatin1String("*.*");

    QDir dir(directory_);

    const QStringList files = dir.entryList(formats, QDir::Files);
    for (const auto &file : files) {
        QGeoTileSpec spec = filenameToTileSpec(file);
        if (spec.zoom() == -1)
            continue;
        QString filename = dir.filePath(file);
        if(addToDiskCacheCond(spec, filename))
            return;
    }
}
QGeoTileSpec GeoFileTileCacheTile::filenameToTileSpec(const QString &filename) const
{
    QGeoTileSpec emptySpec;

    const QStringList parts = filename.split(QLatin1Char('.'));

    if (parts.length() != 2)
        return emptySpec;

    QString name = parts.at(0);
    //prefix contains mapid
    const QString& prefix = m_engine->tilePrefix();
    if(!prefix.isEmpty()){
        if(name.startsWith(prefix))
            name = name.remove(0,prefix.length());
        else
            return emptySpec;
    }
    //name: z-x-y or z-x-y-v
    const QStringList fields = name.split(QLatin1Char('-'));
    const int length = fields.size();
    if (length != 3 && length != 4)
        return emptySpec;

    /*if(fields.at(0).toInt() < m_engine->preCacheZoomLevel())
        return emptySpec;*/

    QList<int> numbers;

    bool ok = false;
    for (int i = 0; i < length; ++i) {
        ok = false;
        int value = fields.at(i).toInt(&ok);
        if (!ok)
            return emptySpec;
        numbers.append(value);
    }

    //File name without version, append default
    if (numbers.length() < 4)
        numbers.append(-1);

    return QGeoTileSpec(m_pluginString, //Consistent with QGeoTiledMapPrivate.cpp line 163
                        1,              //fixed map id
                        numbers.at(0),
                        numbers.at(1),
                        numbers.at(2),
                        numbers.at(3));
}

//cache/{z}/{x}-{y}.{image}
void GeoFileTileCacheTile::loadTilesStyle1()
{
    QDir dir(directory_);

    QMap<int,bool> zmap;
    const QStringList& zdris = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for(const auto& zs : zdris){
        bool ok=false;
        int z = zs.toInt(&ok);
        if(!ok || z<m_engine->preCacheZoomLevel())
            continue;
        zmap.insert(z,false);
    }

    for(int z : zmap){
        QDir zdir(dir.absolutePath()+"/"+QString::number(z));//cache/{z}
        const QStringList& files = zdir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        for (const auto &file : files) {
            QGeoTileSpec spec = filenameToTileSpec(z,file);
            if (spec.zoom() == -1)
                continue;
            QString filename = zdir.filePath(file);
            if(addToDiskCacheCond(spec, filename))
                return;
        }
    }
}

QGeoTileSpec GeoFileTileCacheTile::filenameToTileSpec(int z, const QString &filename) const
{
    QGeoTileSpec emptySpec;

    const QStringList parts = filename.split(QLatin1Char('.'));

    if (parts.length() != 2)
        return emptySpec;

    QString name = parts.at(0);

    //name: {x}-{y}.{image} or {x}-{y}-{v}
    const QStringList fields = name.split(QLatin1Char('-'));
    const int length = fields.size();
    if (length != 2 && length != 3)
        return emptySpec;

    QList<int> numbers;
    numbers << z;

    bool ok = false;
    for (int i = 0; i < length; ++i) {
        ok = false;
        int value = fields.at(i).toInt(&ok);
        if (!ok)
            return emptySpec;
        numbers.append(value);
    }

    //File name without version, append default
    if (numbers.length() < 4)
        numbers.append(-1);

    return QGeoTileSpec(m_pluginString, //Consistent with QGeoTiledMapPrivate.cpp line 163
                        1,              //fixed map id
                        numbers.at(0),
                        numbers.at(1),
                        numbers.at(2),
                        numbers.at(3));
}

//cache/{z}/{x}/{y}.{image}
void GeoFileTileCacheTile::loadTilesStyle2()
{
    QDir dir(directory_);
    QMap<int,bool> zmap;
    //shit zdris content order: 1,2,10,11,12,13,14,15,16,17,18,3,4,5,6,7,8,9
    const QStringList& zdris = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for(const auto& zs : zdris){
        bool ok=false;
        int z = zs.toInt(&ok);
        if(!ok || z<m_engine->preCacheZoomLevel())
            continue;
        zmap.insert(z,false);
    }

    for(auto z: zmap.keys()){
        QDir zdir(dir.absolutePath()+"/"+QString::number(z)+"/");//cache/{z}
        QStringList xdirs = zdir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for(const auto& xs : xdirs)
        {
            bool ok=false;
            int x = xs.toInt(&ok);
            if(!ok || x<0)
                continue;
            QDir xdir(zdir.absolutePath()+"/"+xs);//cache/{z}/{x}
            QStringList files = xdir.entryList(QDir::Files | QDir::NoDotAndDotDot);
            for (const auto &file : files) {
                QGeoTileSpec spec = filenameToTileSpec(z,x,file);
                if (spec.zoom() == -1)
                    continue;
                QString filename = xdir.filePath(file);
                if(addToDiskCacheCond(spec, filename))
                    return;
            }
        }
    }
}

QGeoTileSpec GeoFileTileCacheTile::filenameToTileSpec(int z, int x, const QString &filename) const
{
    QGeoTileSpec emptySpec;

    const QStringList parts = filename.split(QLatin1Char('.'));

    if (parts.length() != 2)
        return emptySpec;

    //cache/{z}/{x}/{y}.{image} or {y}-{v}.{image}
    QString name = parts.at(0);
    const QStringList fields = name.split(QLatin1Char('-'));
    const int length = fields.size();
    if (length != 1 && length != 2)
        return emptySpec;

    QList<int> numbers;
    numbers << z << x;

    bool ok = false;
    for (int i = 0; i < length; ++i) {
        ok = false;
        int value = fields.at(i).toInt(&ok);
        if (!ok)
            return emptySpec;
        numbers.append(value); //y
    }

    //File name without version, append default
    if (numbers.length() < 4)
        numbers.append(-1);

    return QGeoTileSpec(m_pluginString, //Consistent with QGeoTiledMapPrivate.cpp line 163
                        1,              //fixed map id
                        numbers.at(0),
                        numbers.at(1),
                        numbers.at(2),
                        numbers.at(3));
}

//cache/{z}/{y}/{x}.{image}
void GeoFileTileCacheTile::loadTilesStyle3()
{
    QDir dir(directory_);

    QMap<int,bool> zmap;
    const QStringList& zdris = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for(const auto& zs : zdris){
        bool ok=false;
        int z = zs.toInt(&ok);
        if(!ok || z<m_engine->preCacheZoomLevel())
            continue;
        zmap.insert(z,false);
    }

    for(auto z: zmap.keys()){
        QDir zdir(dir.absolutePath()+"/"+QString::number(z)+"/");//cache/{z}
        QStringList ydirs = zdir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for(const auto& ys : ydirs)
        {
            bool ok=false;
            int y = ys.toInt(&ok);
            if(!ok || y<0)
                continue;
            QDir ydir(zdir.absolutePath()+"/"+ys);//cache/{z}/{y}
            QStringList files = ydir.entryList(QDir::Files | QDir::NoDotAndDotDot);
            for (const auto &file : files) {
                QGeoTileSpec spec = filenameToTileSpec(z,y,file,1);
                if (spec.zoom() == -1)
                    continue;
                QString filename = ydir.filePath(file);
                if(addToDiskCacheCond(spec, filename))
                    return;
            }
        }
    }
}

QGeoTileSpec GeoFileTileCacheTile::filenameToTileSpec(int z, int y, const QString &filename, int) const
{
    QGeoTileSpec emptySpec;

    const QStringList parts = filename.split(QLatin1Char('.'));

    if (parts.length() != 2)
        return emptySpec;

    //cache/{z}/{y}/{x}.{image} or {x}-{v}.{image}
    QString name = parts.at(0);
    const QStringList fields = name.split(QLatin1Char('-'));
    const int length = fields.size();
    if (length != 1 && length != 2)
        return emptySpec;

    QList<int> numbers;
    numbers << z ;

    bool ok = false;
    for (int i = 0; i < length; ++i) {
        ok = false;
        int value = fields.at(i).toInt(&ok);
        if (!ok)
            return emptySpec;
        numbers.append(value); //x
        if(i==0)
            numbers << y;
    }

    //File name without version, append default
    if (numbers.length() < 4)
        numbers.append(-1);

    return QGeoTileSpec(m_pluginString, //Consistent with QGeoTiledMapPrivate.cpp line 163
                        1,              //fixed map id
                        numbers.at(0),
                        numbers.at(1),
                        numbers.at(2),
                        numbers.at(3));
}

QT_END_NAMESPACE
