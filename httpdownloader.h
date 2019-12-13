#ifndef HTTPDOWNLOADER_H
#define HTTPDOWNLOADER_H

#include <QUrl>
#include <QFile>
#include <QObject>
#include <QNetworkReply>
#include <QNetworkAccessManager>

/**
 * 一个 HTTP 下载器，它由 DownloadManager 调用生成
 *
 * 参数：
 *  index               指定此下载器的编号，便于分辨各个线程
 *  url, begin, end     指定下载任务的区间为 [begin, end]
 *  path, name          指定此下载器将区间 [begin, end] 写入临时路径 path 下的临时文件 name
 */
class HttpDownloader : public QObject {

    Q_OBJECT

public:
    explicit HttpDownloader(QObject *parent = nullptr) : QObject(parent) {}
    HttpDownloader(int, QUrl, qint64, qint64,
                   QString, QString, QObject *parent = nullptr);
    ~HttpDownloader();

public:
    void start();

signals:
    void finished(int index);                               // 下载完毕，发出通知
    void downloadProgress(int index, qint64 bytesRead);     // 更新下载进度

public slots:
    void onPause();             // 响应 暂停下载 的请求
    void onContinue();          // 响应 继续下载 的请求
    void onAbort();             // 响应 终止下载 的请求

private slots:
    void onReadyRead();
    void onFinished();
    void onError(QNetworkReply::NetworkError);
    void onDownloadProgress();

private:
    int         index;
    QUrl        url;
    qint64      begin;
    qint64      end;
    QString     path;
    QString     name;

    QNetworkAccessManager *manager;
    QNetworkReply         *reply;
    QFile                 *file;

    qint64      bytesRead;      // 用于断点续传，记录自上一次开始下载时已下载了多少
                                // 将它与 begin 相加可确定下一次下载时从何处开始
};

#endif // HTTPDOWNLOADER_H
