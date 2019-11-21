#include "httpdownloader.h"

#include <QDir>
#include <QNetworkReply>

HttpDownloader::HttpDownloader(QObject *parent)
            : QObject(parent) {

}

HttpDownloader::HttpDownloader(int index, QUrl url, QString name, QString path, qint64 begin, qint64 end, QObject *parent)
            : QObject(parent), index(index), url(url), name(name), path(path), begin(begin), end(end) {

    manager = new QNetworkAccessManager(this);

    QDir dir(path);
    if (!dir.exists()) {
        if (!dir.mkdir(path)) {
            qDebug() << "创建文件夹失败：" << path;
        } else {
            qDebug() << "创建文件夹成功：" << path;
        }
    }

    qDebug() << "打开文件：" << path + name;
    QFileInfo info(path+name);
    file = new QFile(path+name);
    if (info.exists()) {
        lastTimeBytesRead = file->size();
        qDebug() << "发现未完成文件，尝试从" << lastTimeBytesRead+begin << "继续下载";
    }
    if (!file->open(QIODevice::WriteOnly | QIODevice::Append)) {
        qDebug() << "打开文件时失败";
        delete file;
        file = Q_NULLPTR;
        return;
    }
}

HttpDownloader::~HttpDownloader() {

    delete manager;
    delete reply;

    if(file) {
        file->close();
        delete file;
    }
}

void HttpDownloader::startDownload() {

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Range", QString("bytes=%1-%2").arg(begin+lastTimeBytesRead).arg(end).toLatin1());

    reply = manager->get(request);

    QObject::connect(reply, &QNetworkReply::readyRead, this, &HttpDownloader::onReadyRead);
    QObject::connect(reply, &QNetworkReply::finished, this, &HttpDownloader::onFinished);
    QObject::connect(reply, &QNetworkReply::downloadProgress, this, &HttpDownloader::onDownloadProgress);
}

void HttpDownloader::onSupPauseDownload() {

    qDebug() << "线程" << index << "暂停下载";

    if (reply) {
        QObject::disconnect(reply, &QNetworkReply::readyRead, this, &HttpDownloader::onReadyRead);
        QObject::disconnect(reply, &QNetworkReply::finished, this, &HttpDownloader::onFinished);
        QObject::disconnect(reply, &QNetworkReply::downloadProgress, this, &HttpDownloader::onDownloadProgress);

        reply->abort();
        reply->deleteLater();
        reply = Q_NULLPTR;
    }

    lastTimeBytesRead = file->size();
}

void HttpDownloader::onSupStartDownload() {

    qDebug() << "线程" << index << "继续下载，从" << begin+lastTimeBytesRead << "到" << end;
    startDownload();
}

void HttpDownloader::onReadyRead() {

    if (file) {
        file->write(reply->readAll());
    }
}

void HttpDownloader::onFinished() {

    qDebug() << "线程" << index << "下载完毕\t下载大小：" << file->size();

    file->close();

    emit subThreadFinished();
}

void HttpDownloader::onDownloadProgress() {

    emit subDownloadProgress(index, file->size());
}
