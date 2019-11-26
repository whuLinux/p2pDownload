#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QUrl>
#include <QTimer>
#include <QObject>

const int MAX_THREAD_COUNT = 16;                // 最大线程数
const int PARTITION_SIZE   = 1024 * 1024 * 5;   // 按 5M 对文件分块
const int UPDATE_TIME      = 1000 * 3;          // 每过 3s 更新下载速度

/**
 * 接收一个下载任务并分配资源进行下载
 *
 * 参数：
 *  url, begin, end         指定下载任务的区间为 [begin, end]
 */
class DownloadManager : public QObject {

    Q_OBJECT

public:
    explicit DownloadManager(QObject *parent = nullptr) : QObject(parent) {}
    DownloadManager(QUrl url, qint64 begin = -1, qint64 end = -1,
                QObject *parent = nullptr);
    ~DownloadManager() { delete [] totalBytesRead; }

public:
    void start();
    void pause();

public:
    static qint64  getFileSize(QUrl url);
    static QString getFileName(QUrl url);

public:
    void setUrl  (const QUrl    &value) { url   = value; }
    void setBegin(const qint64  &value) { begin = value; }
    void setEnd  (const qint64  &value) { end   = value; }
    void setPath (const QString &value) { path  = value; }
    void setName (const QString &value) { name  = value; }

    QUrl    getUrl  () const { return url;   }
    qint64  getBegin() const { return begin; }
    qint64  getEnd  () const { return end;   }
    QString getPath () const { return path;  }
    QString getName () const { return name;  }

    double  getSpeed() const { return speed; }
    qint64  getTime () const { return totalTime; }
    double  getProgress() const { return progress; }

signals:
    void continueDownload();
    void pauseDownload();

    void updateData(qint64 time, double speed, double progress);

public slots:
    void onFinished();
    void onDownloadProgress(int index, qint64 bytesRead);

private slots:
    void updateSpeed();

private:
    void finished();

private:
    QUrl    url;
    qint64  begin;
    qint64  end;

    qint64  size;
    QString path;
    QString name;

    int     threadCount;
    int     finishedThreadCount;

    bool    isFromStart;    // 是否是第一次从头开始下载

    QTimer  *timer;                     // 用于计算下载速度
    qint64  *totalBytesRead;            // 总下载字节数
    qint64  lastTimePartBytesRead;      // 上一个时间片内的总下载字节数
    double  speed;                      // 上一个时间片内的平均下载速度
    qint64  startTime;                  // 自上一次开始时已下载时间
    qint64  totalTime;                  // 已下载时间
    double  progress;                   // 已下载百分比
};

#endif // DOWNLOADMANAGER_H
