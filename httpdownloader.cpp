#include "httpdownloader.h"

#include <QDir>
#include <QNetworkReply>

HttpDownloader::HttpDownloader(int index, QUrl url, qint64 begin, qint64 end,
                               QString path, QString name, QObject *parent)
        : QObject(parent), index(index), url(url),
          begin(begin), end(end), path(path), name(name), bytesRead(0)
{
    manager = new QNetworkAccessManager(this);

    /* 检测临时文件夹是否已被创建 */
    QDir dir(path);
    if (!dir.exists()) {
        qDebug() << "[线程" << index << "] 文件夹未创建，正在尝试创建...";
        if (!dir.mkdir(path)) {
            qDebug() << "[线程" << index << "] 文件夹创建失败：" << path;
        } else {
            qDebug() << "[线程" << index << "] 文件夹创建成功：" << path;
        }
    }

    /* 创建或打开临时文件 */
    QFileInfo info(path+name);
    file = new QFile(path+name);
    if (info.exists()) {
        bytesRead = file->size();
        qDebug() << "[线程" << index << "] 发现上一次未下载完的临时文件，尝试从"
                 << bytesRead+begin << "处继续下载";
    }

    if (!file->open(QIODevice::WriteOnly | QIODevice::Append)) {
        qDebug() << "[线程" << index << "] 打开文件失败";
        delete file;
        file = nullptr;
        return;
    }
}

void HttpDownloader::start() {

    if (bytesRead+begin >= end) {
        onFinished();
        return;
    }

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Range", QString("bytes=%1-%2")
                         .arg(begin+bytesRead).arg(end)
                         .toLatin1());

    reply = manager->get(request);

    QObject::connect(reply, &QNetworkReply::readyRead,
                     this,  &HttpDownloader::onReadyRead);
    QObject::connect(reply, &QNetworkReply::finished,
                     this,  &HttpDownloader::onFinished);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this,  SLOT(onError(QNetworkReply::NetworkError)));
    QObject::connect(reply, &QNetworkReply::downloadProgress,
                     this,  &HttpDownloader::onDownloadProgress);

    qDebug() << "[线程" << index << "] 开始下载区间："
             << begin+bytesRead << " - " << end;
}

void HttpDownloader::onPause() {

    if (reply) {
        QObject::disconnect(reply, &QNetworkReply::readyRead,
                            this,  &HttpDownloader::onReadyRead);
        QObject::disconnect(reply, &QNetworkReply::finished,
                            this,  &HttpDownloader::onFinished);
        disconnect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                   this,  SLOT(onError(QNetworkReply::NetworkError)));
        QObject::disconnect(reply, &QNetworkReply::downloadProgress,
                            this,  &HttpDownloader::onDownloadProgress);

        reply->abort();
        reply->deleteLater();
        reply = nullptr;
    }

    qDebug() << "[线程" << index << "] 暂停下载";
}

void HttpDownloader::onContinue() {

    bytesRead = file->size();
    start();
}

void HttpDownloader::onAbort() {

    this->deleteLater();
}

void HttpDownloader::onReadyRead() {

    if (file) {
        file->write(reply->readAll());
    }
}

void HttpDownloader::onFinished() {

    qDebug() << "[线程" << index << "] 下载完毕，下载文件大小：" << file->size();
    if (file->size() < end - begin + 1)
        qDebug() << "[线程" << index << "]" << "未能下载完整";

    delete file;
    file = nullptr;

    emit finished();
}

void HttpDownloader::onError(QNetworkReply::NetworkError e) {

    qDebug() << "[线程" << index << "] 下载出错：" << e;
}

void HttpDownloader::onDownloadProgress() {

    emit downloadProgress(index, file->size());
}
