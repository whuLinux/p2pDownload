#include "downloadmanager.h"
#include "ui_downloadmanager.h"

#include "httpdownloader.h"

#include <QDebug>
#include <QDateTime>
#include <QFileDialog>
#include <QNetworkReply>
#include <QNetworkAccessManager>

DownloadManager::DownloadManager(QWidget *parent)
            : QMainWindow(parent) , ui(new Ui::DownloadManager),
              startSecond(0), isFromStart(true), threadCount(0), finishedThreadCount(0) {
    ui->setupUi(this);
    ui->lineEdit_Path->setText("C:/Users/yujia/Desktop/");
}

DownloadManager::~DownloadManager() {

    delete ui;
}

qint64 DownloadManager::getFileSize(QUrl url) {
    /* 根据 URL 得到文件的大小 */

    /* 发送请求 */
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.head(QNetworkRequest(url));

    /* 检测请求是否结束 */
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    /* 输出出错信息 */
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "获取文件大小时发生请求出错：" << reply->errorString();
        return 0;
    }

    /* 获取文件大小 */
    QVariant var = reply->header(QNetworkRequest::ContentLengthHeader);
    reply->deleteLater();

    /* 转换为 qint64 */
    qint64 size = var.toLongLong();
    return size;
}

QString DownloadManager::getFileName(QUrl url) {

    return QFileInfo(url.path()).fileName();
}

QString DownloadManager::getFilePath() {

    return ui->lineEdit_Path->text();
}

int DownloadManager::getThreadCount(qint64 size) {

    if (size >= MAX_THREAD_COUNT * BLOCK_SIZE) {
        return MAX_THREAD_COUNT;
    } else {
        return size / BLOCK_SIZE + 1;
    }
}

void DownloadManager::onThreadFinished() {

    finishedThreadCount++;

    qDebug() << "已完成进程数：" << finishedThreadCount << "\t总进程数：" << threadCount;

    if (finishedThreadCount >= threadCount) {
        qint64 endSecond = QDateTime::currentDateTime().toTime_t();

        qDebug() << "所有线程下载完毕，用时：" << endSecond - startSecond << "秒";
        qDebug() << "开始合并";

        if (threadCount != 1) {
            QFile outFile(path+name);
            outFile.open(QIODevice::WriteOnly | QIODevice::Append);

            QFile *partFile;
            for (int i = 0; i < threadCount; i++) {
                partFile = new QFile(path+name+".download/"+name+".part"+QString::number(i+1));
                partFile->open(QIODevice::ReadWrite);
                qDebug() << "打开文件：" << partFile->fileName();
                outFile.write(partFile->readAll());
                partFile->close();
                delete partFile;
                partFile = Q_NULLPTR;
                QFile::remove(path+name+".download/"+name+".part"+QString::number(i+1));
            }

            QDir dir;
            dir.rmdir(path+name+".download/");
        }

        qDebug() << "合并完毕";
    }
}

void DownloadManager::on_pushButton_Start_clicked() {

    if (!isFromStart) {
        emit supStartDownload();
        return;
    }

    isFromStart = false;

    url  = ui->lineEdit_URL->text();
    path = getFilePath();
    name = getFileName(url);
    size = getFileSize(url);
    threadCount = getThreadCount(size);

    qDebug() << url << path << name << size << threadCount;

    ui->label_Name->setText("File Name: " + name);
    QString lableSize = (size < 1024) ? (QString::number(size) + " bit") :
                        (size < 1024*1024) ? (QString::number(size/1024.0) + " Kbit") :
                        (size < 1024*1024*1024) ? (QString::number(size/1024.0/1024.0) + " Mbit") :
                                                  (QString::number(size/1024.0/1024.0/1024.0) + " Gbit");
    ui->label_Size->setText("File Size: " + lableSize);

    /* 分配线程资源 */
    /* 创建临时文件夹 */
//    path += name + ".download/";
    QDir dir;
    if (!dir.exists(path)) {
        dir.mkpath(path);
    }

    HttpDownloader *downloader;

    startSecond = QDateTime::currentDateTime().toTime_t();
    if (threadCount == 1) {

        downloader = new HttpDownloader(1, url, name, path, 0, size, this);
        qDebug() << 1 << name << path << 0 << size << size / 1024.0 << "Kb";
        downloader->startDownload();

        QObject::connect(downloader, &HttpDownloader::threadFinished, downloader, &HttpDownloader::deleteLater);
        QObject::connect(downloader, &HttpDownloader::threadFinished, this, &DownloadManager::onThreadFinished);

    } else {

        for (int i = 0; i < threadCount; i++) {
            qint64 begin = size * i / threadCount;
            qint64 end   = size * (i+1) / threadCount - 1;
            if (i == threadCount - 1) { end++; }

            downloader = new HttpDownloader(i+1, url, name+".part"+QString::number(i+1), path+name+".download/", begin, end, this);
            qDebug() << i+1 << name+".part"+QString::number(i+1) << path+name+".download/" << begin << end << (end - begin) / 1024.0 << "Kb";
            downloader->startDownload();

            QObject::connect(downloader, &HttpDownloader::threadFinished, downloader, &HttpDownloader::deleteLater);
            QObject::connect(downloader, &HttpDownloader::threadFinished, this, &DownloadManager::onThreadFinished);
            QObject::connect(this, &DownloadManager::supPauseDownload, downloader, &HttpDownloader::onSupPauseDownload);
            QObject::connect(this, &DownloadManager::supStartDownload, downloader, &HttpDownloader::onSupStartDownload);
        }
    }
}

void DownloadManager::on_pushButton_Path_clicked() {
    /* 按下 Path 按钮后弹出资源管理器，并读取当前路径，写在 lineEditor 上 */

    QString path = QFileDialog::getExistingDirectory(this, "Choose Download Path", "/");

    /* 若此次未选择路径，则不应将之前的路径覆盖掉 */
    /* 只有在此次选择了路径时，才覆盖之前的路径，并在尾部添加斜杠 */
    if (!path.isEmpty()) {
        this->path = path + "/";
    }

    ui->lineEdit_Path->setText(this->path);
}

void DownloadManager::on_pushButton_Pause_clicked() {
    /* 通知各线程暂停下载 */
    emit supPauseDownload();
}
