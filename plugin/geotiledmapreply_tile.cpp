#include "geotiledmapreply_tile.h"

static const unsigned char pngSignature[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00};
static const unsigned char jpegSignature[] = {0xFF, 0xD8, 0xFF, 0x00};
static const unsigned char gifSignature[] = {0x47, 0x49, 0x46, 0x38, 0x00};

GeoTiledMapReplyTile::GeoTiledMapReplyTile(const QString& filepath, const QGeoTileSpec &spec,QObject *parent)
    : QGeoTiledMapReply{spec,parent},m_filepath(filepath)
{
    connect(&m_watcher, &QFutureWatcher<int>::finished, this, &GeoTiledMapReplyTile::replyFinished);
}

GeoTiledMapReplyTile::~GeoTiledMapReplyTile()
{
}

void GeoTiledMapReplyTile::startHandle()
{
    m_watcher.setFuture(QtConcurrent::run(QThreadPool::globalInstance(),[this]()-> QByteArray {
        int index = m_filepath.lastIndexOf(QLatin1Char('/'));
        if(index == -1)
            return QByteArray();

        QDir dir(m_filepath.left(index));
        QString filename = m_filepath.right(m_filepath.size()-index-1);
        QStringList tiles = dir.entryList({filename});
        if(!tiles.size())
            return QByteArray();

        QFile file(dir.absoluteFilePath(tiles.first()));
        if (!file.open(QIODevice::ReadOnly))
            return QByteArray();

        QByteArray imageData = file.readAll();
        file.close();

        bool validFormat = true;
        if (imageData.startsWith(reinterpret_cast<const char*>(pngSignature)))
            setMapImageFormat(QStringLiteral("png"));
        else if (imageData.startsWith(reinterpret_cast<const char*>(jpegSignature)))
            setMapImageFormat(QStringLiteral("jpg"));
        else if (imageData.startsWith(reinterpret_cast<const char*>(gifSignature)))
            setMapImageFormat(QStringLiteral("gif"));
        else
            validFormat = false;

        if (validFormat)
            return imageData;

        return QByteArray();
    }));
}

void GeoTiledMapReplyTile::replyFinished() {
    QFuture<QByteArray> future = m_watcher.future();
    if (future.isFinished()) {
        QByteArray imageData=future.result();
        if(imageData.size())
            setMapImageData(imageData);
        else
            setError(QGeoTiledMapReply::UnknownError,QStringLiteral("Tile not found"));
        setFinished(true);
    }
}



