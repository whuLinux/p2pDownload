#include "downloadmanager.h"
#include "httpdownloader.h"

#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QFileInfo>
#include <QEventLoop>
#include <QNetworkReply>
#include <QNetworkAccessManager>

DownloadManager::DownloadManager
        (QUrl url, qint64 begin, qint64 end, QObject *parent)
        : QObject(parent), url(url), begin(begin), end(end),
          size(0), threadCount(0), finishedThreadCount(0),
          isFromStart(true), lastTimePartBytesRead(0), speed(0),
          startTime(0), totalTime(0), progress(0) {

    timer = new QTimer(this);
    QObject::connect(timer, &QTimer::timeout,
                     this,  &DownloadManager::updateSpeed);
}

void DownloadManager::start() {

    /* 开始计时 */
    timer->start(UPDATE_TIME);

    /* 记录下载时间 */
    startTime = QDateTime::currentDateTime().toTime_t();

    if (!isFromStart) {
        emit continueDownload();
    }

    /* 初次运行时将 isFromStart 置为 false，以后再调用此函数则为断点续传 */
    isFromStart = false;

    /* 确认起止点及文件大小 */
    if ((size = end-begin) <= 0 || begin < 0 || end < 0) {
        qDebug() << "[Manager] size <= 0，将缺省为下载整个文件";
        size  = getFileSize(url);
        begin = 0;
        end   = size;
    }

    /* 确认文件名 */
    if (name.isEmpty()) {
        name = getFileName(url);
    }

    /* 确认路径 */
    if (path.isEmpty()) {
        path = "";
    }

    /* 确认分配线程数 */
    threadCount = (size > PARTITION_SIZE * MAX_THREAD_COUNT) ?
                  (MAX_THREAD_COUNT) : (size/PARTITION_SIZE + 1);

    /* 创建临时文件夹 */
    QDir dir;
    QString dirName = path + name + "/";
    if (!dir.exists(dirName)) {
        dir.mkpath(dirName);
        if (!dir.exists(dirName)) {
            qDebug() << "[Manager] 已创建临时文件夹：" << dirName;
        }
    }

    qDebug() << "[Manager] 开始下载，文件总大小: " << size;

    /* 创建线程 */
    HttpDownloader *downloader;

    if (threadCount == 1) {

        downloader = new HttpDownloader(1, url, begin, end,
                                        dirName, name, this);
        downloader->start();

        QObject::connect(downloader, &HttpDownloader::finished,
                         downloader, &HttpDownloader::deleteLater);
        QObject::connect(downloader, &HttpDownloader::finished,
                         this,       &DownloadManager::onFinished);
        QObject::connect(this,       &DownloadManager::pauseDownload,
                         downloader, &HttpDownloader::onPause);
        QObject::connect(this,       &DownloadManager::continueDownload,
                         downloader, &HttpDownloader::onContinue);
        QObject::connect(this,       &DownloadManager::abortDownload,
                         downloader, &HttpDownloader::onAbort);
        QObject::connect(downloader, &HttpDownloader::downloadProgress,
                         this,       &DownloadManager::onDownloadProgress);

    } else {

        for (int i = 0; i < threadCount; i++) {
            qint64 partBegin = size*i/threadCount + begin;
            qint64 partEnd   = size*(i+1)/threadCount - 1;
            if (i == threadCount-1) { partEnd++; }

            downloader = new HttpDownloader(i+1, url, partBegin, partEnd,
                             dirName, name+".part"+QString::number(i+1), this);
            downloader->start();

            QObject::connect(downloader, &HttpDownloader::finished,
                             downloader, &HttpDownloader::deleteLater);
            QObject::connect(downloader, &HttpDownloader::finished,
                             this,       &DownloadManager::onFinished);
            QObject::connect(this,       &DownloadManager::pauseDownload,
                             downloader, &HttpDownloader::onPause);
            QObject::connect(this,       &DownloadManager::continueDownload,
                             downloader, &HttpDownloader::onContinue);
            QObject::connect(this,       &DownloadManager::abortDownload,
                             downloader, &HttpDownloader::onAbort);
            QObject::connect(downloader, &HttpDownloader::downloadProgress,
                             this,       &DownloadManager::onDownloadProgress);
        }
    }

    totalBytesRead = new qint64[threadCount];
    for (int i = 0; i < threadCount; i++) {
        totalBytesRead[i] = 0;
    }
}

void DownloadManager::pause() {

    timer->stop();
    totalTime += QDateTime::currentDateTime().toTime_t() - startTime;

    emit pauseDownload();
}

bool DownloadManager::abort() {

    qDebug() << "[Manager] 终止任务中...";

    pause();
    emit abortDownload();

    qDebug() << "\t[Manager] 删除分块文件中...";
    if (threadCount == 1) {
        QFile::remove(path+name+"/"+name);
    } else if (threadCount > 1) {
        for (int i = 0; i < threadCount; i++) {
            QFile::remove(path+name+"/"+name+".part"+QString::number(i+1));
        }
    }

    qDebug() << "\t[Manager] 删除临时文件夹中...";
    QDir dir;
    if (!dir.rmdir(path+"."+name+"/")) {
        qDebug() << "\t[Manager] 未能删除临时文件夹";
        qDebug() << "[Manager] 任务终止失败";
        return false;
    }

    qDebug() << "[Manager] 任务已终止";
    this->deleteLater();

    return true;
}

qint64 DownloadManager::getFileSize(QUrl url) {

    /* 发送请求 */
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.head(QNetworkRequest(url));

    /* 检测请求是否结束 */
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished,
                     &loop, &QEventLoop::quit);
    loop.exec();

    /* 请求错误则输出出错信息并退出 */
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "[getFileSize][error] " << reply->errorString();
        return -1;
    }

    /* 获取文件大小 */
    QVariant var = reply->header(QNetworkRequest::ContentLengthHeader);
    reply->deleteLater();

    /* 转换为 qint64 */
    return var.toLongLong();
}

QString DownloadManager::getFileName(QUrl url) {

    return QFileInfo(url.path()).fileName();
}

void DownloadManager::onFinished() {

    finishedThreadCount++;

    qDebug() << "已完成进程数：" << finishedThreadCount
             << "\t总进程数：" << threadCount;

    if (finishedThreadCount >= threadCount) {
        finished();
    }
}

void DownloadManager::onDownloadProgress(int index, qint64 bytesRead) {

    totalBytesRead[index] = bytesRead;
    lastTimePartBytesRead += bytesRead;

    qint64 sum = 0;
    for (int i = 1; i < threadCount+1; i++) {
        sum += totalBytesRead[i];
    }

    progress = (double)sum / size;
    qint64 currentTime = totalTime - startTime
                         + QDateTime::currentDateTime().toTime_t();

    emit updateData(currentTime, speed, progress);
}

void DownloadManager::updateSpeed() {

    speed = (double)lastTimePartBytesRead / UPDATE_TIME;
    lastTimePartBytesRead = 0;
}

void DownloadManager::finished() {

    totalTime += QDateTime::currentDateTime().toTime_t() - startTime;
    speed = 0;
    progress = 1;

    if (threadCount != 1) {
        QFile outFile(path+name);
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);

        QFile *partFile;
        for (int i = 0; i < threadCount; i++) {
            qDebug() << "[Manager] 正在合并分块文件" << i+1;

            partFile = new QFile(path+name+"/"+name+
                                 ".part"+QString::number(i+1));
            partFile->open(QIODevice::ReadWrite);

            outFile.write(partFile->readAll());

            partFile->close();
            delete partFile;

            QFile::remove(path+name+"/"+name+
                          ".part"+QString::number(i+1));
        }
    } else {
        QFile outFile(path+name);
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QFile *partFile = new QFile(path+name+"/"+name);
        partFile->open(QIODevice::ReadWrite);
        outFile.write(partFile->readAll());
        partFile->close();
        delete partFile;
        QFile::remove(path+name+"/"+name);
    }

    QDir dir;
    if (!dir.rmdir(path+"."+name+"/")) {
        qDebug() << "[Manager] 未能删除空文件夹";
    }

    emit taskFinished();
}
