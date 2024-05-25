#ifndef GEOTILEDMAPREPLYTILE_H
#define GEOTILEDMAPREPLYTILE_H

#include <QtLocation/private/qgeotiledmapreply_p.h>
#include <QtConcurrent>
#include <QFutureWatcher>

class GeoTiledMapReplyTile : public QGeoTiledMapReply
{
    Q_OBJECT
public:
    explicit GeoTiledMapReplyTile(const QString& filepath, const QGeoTileSpec &spec, QObject *parent = nullptr);
    ~GeoTiledMapReplyTile();

private Q_SLOTS:
    void startHandle();
    void replyFinished();

private:
    QString m_filepath;
    QFutureWatcher<QByteArray> m_watcher;
};

#endif // GEOTILEDMAPREPLYTILE_H
