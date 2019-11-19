#ifndef HTTPDOWNLOADER_H
#define HTTPDOWNLOADER_H

#include <QUrl>
#include <QFile>
#include <QObject>
#include <QNetworkAccessManager>

/*
    一个下载器，按照给定的URL，从给定的开始及截至处下载文件，并存放在指定的路径下
    这些参数由调用它的 DownloadManager 指定
*/
class HttpDownloader : public QObject {

    Q_OBJECT

public:
    explicit HttpDownloader(QObject *parent = nullptr);
    HttpDownloader(int index, QUrl url, QString name, QString path, qint64 begin, qint64 end, QObject *parent);

    void startDownload();

signals:
    void threadFinished();  // 子线程下载结束时通知 manager

public slots:
    void onSupPauseDownload();      // manager 要求暂停瞎咋
    void onSupStartDownload();      // manager 要求继续下载

private slots:
    void onReadyRead();
    void onFinished();
    void onDownloadProgress();

private:
    QNetworkAccessManager *manager;
    QNetworkReply         *reply;
    QFile                 *file;

    int         index;      // 线程号，用于确定该线程划分的是哪一部份
    QUrl        url;
    QString     name;
    QString     path;
    qint64      begin;
    qint64      end;

    qint64      lastTimeBytesRead;      // 用于断点续传，记录上一次暂停下载时已下载了多少
                                        // 将它与 begin 相加可确定下一次继续下载时从何处开始下载
};

#endif // HTTPDOWNLOADER_H
