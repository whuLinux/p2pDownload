#include "downloadmanager.h"
#include "ui_downloadmanager.h"

#include "httpdownloader.h"

#include <QDebug>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkAccessManager>

DownloadManager::DownloadManager(QWidget *parent)
            : QMainWindow(parent), ui(new Ui::DownloadManager),
              isPartner(false), taskIndex(-1), begin(-1), end(-1), lastStartSecond(0), totalSecond(0),
              isFromStart(true), threadCount(0), finishedThreadCount(0) {
    ui->setupUi(this);
    ui->lineEdit_Path->setText("C:/Users/yujia/Desktop/");
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(0);
}

DownloadManager::DownloadManager(bool isPartner, int taskIndex, QUrl url, qint64 begin, qint64 end, QWidget *parent)
        : QMainWindow(parent), ui(new Ui::DownloadManager),
          isPartner(isPartner), taskIndex(taskIndex),
          url(url), begin(begin), end(end), lastStartSecond(0), totalSecond(0),
          isFromStart(true), threadCount(0), finishedThreadCount(0) {

    ui->setupUi(this);
}

DownloadManager::~DownloadManager() {

    delete ui;
    delete [] bytesRead;
}

qint64 DownloadManager::getFileSize(QUrl url) {
    /* 根据 URL 获取文件大小，返回字节数 */

    /* 发送请求 */
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.head(QNetworkRequest(url));

    /* 检测请求是否结束 */
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    /* 请求错误则输出出错信息并退出 */
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "【HTTP请求错误】【获取文件大小时请求出错】" << reply->errorString();
        return -1;
    }

    /* 获取文件大小 */
    QVariant var = reply->header(QNetworkRequest::ContentLengthHeader);
    reply->deleteLater();

    /* 转换为 qint64 */
    return var.toLongLong();
}

QString DownloadManager::getFileName(QUrl url) {
    /* 处理 URL 格式，获取文件名 */

    return QFileInfo(url.path()).fileName();
}

int DownloadManager::getPartCount(qint64 fileSize, qint64 divideSize, int amount) {
    /* 可用于分配伙伴机数量，也可用于分配线程数量 */

    return (fileSize > divideSize * amount) ? (amount) : (fileSize/divideSize + 1);
}

/**
 * 错误代码：
 *      0 - 正确执行
 *      1 - URL不合法
 *      2 - 起止点不合法
 *      3 - 无法创建下载文件夹
 */
int DownloadManager::startDownload() {

    if (!isFromStart) {
        emit signalContinueDownload();

        ui->pushButton_Start->setText("Continue");
        ui->pushButton_Start->setEnabled(false);
        ui->pushButton_Pause->setEnabled(true);

        return 0;
    }

    /* 初次运行时将 isFromStart 置为 false，以后再调用此函数则为断点续传 */
    isFromStart = false;

    /* 开始计时 */
    lastStartSecond = QDateTime::currentDateTime().toTime_t();

    /* 确认 URL */
    if (url.isEmpty()) {
        qDebug() << "URL为空，尝试从输入框中读取...";
        if ((url = ui->lineEdit_URL->text()).isEmpty()) {
            qDebug() << "输入框为空，无法读取URL，无法开始下载";
            return 1;
        }
    }

    /* 确认文件名 */
    name = getFileName(url);
    if (taskIndex >= 0) {
        name += ".index" + QString::number(taskIndex);
    }

    /* 确认存放路径 */
    path = getFilePath();

    /* 确认起止点及文件大小 */
    if (begin == -1 || end == -1) {
        qDebug() << "未指定起止点，默认下载整个文件";
        begin = 0;
        end = getFileSize(url);
        size = end - begin;
    } else if (end > begin) {
        qDebug() << "设置的终点大于起点，无法分配该任务";
        return 2;
    } else {
        size = end - begin;
    }

    /* 确认分配线程数 */
    threadCount = getPartCount(size);

    /* 创建临时文件夹，用于存放分块文件 */
    QDir dir;
    QString dirName = path + "." + name + "/";
    if (!dir.exists(dirName)) {
        dir.mkpath(dirName);
        if (!dir.exists(dirName)) {
            qDebug() << "临时文件夹创建失败";
            return 3;
        } else {
            qDebug() << "临时文件夹创建成功: " << dirName;
        }
    }

    /* 创建线程 */
    HttpDownloader *downloader;

    if (threadCount == 1) {

        downloader = new HttpDownloader(1, url, name, dirName, begin, end, this);
        downloader->startDownload();

        QObject::connect(downloader, &HttpDownloader::subThreadFinished, downloader, &HttpDownloader::deleteLater);
        QObject::connect(downloader, &HttpDownloader::subThreadFinished, this, &DownloadManager::onSubThreadFinished);
        QObject::connect(this, &DownloadManager::signalPauseDownload, downloader, &HttpDownloader::onSupPauseDownload);
        QObject::connect(this, &DownloadManager::signalContinueDownload, downloader, &HttpDownloader::onSupStartDownload);
        QObject::connect(downloader, &HttpDownloader::subDownloadProgress, this, &DownloadManager::onSubDownloadProgress);

    } else {

        for (int i = 0; i < threadCount; i++) {
            qint64 partBegin = size*i/threadCount + begin;
            qint64 partEnd   = size*(i+1)/threadCount - 1;
            if (i == threadCount-1) { partEnd++; }

            downloader = new HttpDownloader(i+1, url, name+".part"+QString::number(i+1),
                                            dirName, partBegin, partEnd, this);
            downloader->startDownload();

            QObject::connect(downloader, &HttpDownloader::subThreadFinished, downloader, &HttpDownloader::deleteLater);
            QObject::connect(downloader, &HttpDownloader::subThreadFinished, this, &DownloadManager::onSubThreadFinished);
            QObject::connect(this, &DownloadManager::signalPauseDownload, downloader, &HttpDownloader::onSupPauseDownload);
            QObject::connect(this, &DownloadManager::signalContinueDownload, downloader, &HttpDownloader::onSupStartDownload);
            QObject::connect(downloader, &HttpDownloader::subDownloadProgress, this, &DownloadManager::onSubDownloadProgress);
        }
    }

    /* 初始时已下载大小置0 */
    bytesRead = new qint64[threadCount];
    for (int i = 0; i < threadCount; i++) {
        bytesRead[i] = 0;
    }

    qDebug() << "[startDownload]" << "URL: " << url;
    qDebug() << "[startDownload]" << "文件名: " << name;
    qDebug() << "[startDownload]" << "存放路径: " << path;
    qDebug() << "[startDownload]" << "开始处: " << begin;
    qDebug() << "[startDownload]" << "结束处: " << end;
    qDebug() << "[startDownload]" << "总大小: " << size;
    qDebug() << "[startDownload]" << "分配线程数: " << threadCount;

    return 0;
}

void DownloadManager::pauseDownload() {

    ui->pushButton_Start->setEnabled(true);
    ui->pushButton_Pause->setEnabled(false);

    emit signalPauseDownload();
}

QString DownloadManager::getFilePath() {
    /* 获取下载路径，若无用户指定，则默认存放于 exe 同级路径下 */

    if (isPartner) {
        return "";
    } else {
        return ui->lineEdit_Path->text();
    }
}

void DownloadManager::downloadFinished() {

    totalSecond += QDateTime::currentDateTime().toTime_t() - lastStartSecond;

    ui->label_Time->setText("Time Usage: " + QString::number(totalSecond) + " s");
    ui->pushButton_Start->setEnabled(false);
    ui->pushButton_Pause->setEnabled(false);

    if (threadCount != 1) {
        QFile outFile(path+name);
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);

        QFile *partFile;
        for (int i = 0; i < threadCount; i++) {
            partFile = new QFile(path+"."+name+"/"+name+".part"+QString::number(i+1));
            partFile->open(QIODevice::ReadWrite);

            outFile.write(partFile->readAll());

            partFile->close();
            delete partFile;
            partFile = Q_NULLPTR;

            QFile::remove(path+"."+name+"/"+name+".part"+QString::number(i+1));
        }
    }

    QDir dir;
    if (!dir.rmdir(path+"."+name+"/")) {
        qDebug() << "未能删除空文件夹";
    }

    QMessageBox::information(this, "文件下载完毕", "文件名： "+name+"\n已下载至 "+path);
}

void DownloadManager::onSubThreadFinished() {

    finishedThreadCount++;

    qDebug() << "已完成进程数：" << finishedThreadCount << "\t总进程数：" << threadCount;

    if (finishedThreadCount >= threadCount) {
        downloadFinished();
    }
}

void DownloadManager::onSubDownloadProgress(int index, qint64 bytesRead) {

    this->bytesRead[index] = bytesRead;

    qint64 sum = 0;
    for (int i = 1; i < threadCount+1; i++) {
        sum += this->bytesRead[i];
    }

    ui->progressBar->setValue(sum);
    ui->progressBar->setMaximum(size);

    qint64 currentTotalSecond = totalSecond + QDateTime::currentDateTime().toTime_t() - lastStartSecond;
    ui->label_Time->setText("Time Usage: " + QString::number(currentTotalSecond) + " s");
    ui->label_Speed->setText("Average Speed: " + QString::number(sum/1024.0/1024.0/currentTotalSecond) + " Mbit/s");
}

void DownloadManager::on_pushButton_Start_clicked() {

    startDownload();
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

    pauseDownload();
}
