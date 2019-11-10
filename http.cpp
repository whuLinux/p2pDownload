#include "http.h"
#include "ui_http.h"

#include <QNetworkRequest>

#include <QDebug>
#include <QFileInfo>
#include <QFileDialog>

Http::Http(QWidget *parent)
        : QWidget(parent), ui(new Ui::Http),
          totalBytes(0), bytesRead(0), bytesLastRead(0) {

    ui->setupUi(this);
    ui->progressBar->hide();

    manager = new QNetworkAccessManager(this);
}

Http::~Http() {

    delete ui;

    delete manager;
    delete reply;

    delete file;
}

void Http::startRequest(QUrl url, qint64 bytesLastRead) {

    /* 从bytesRead 处开始继续传输 */
    qDebug() << "bytesLastRead = " << bytesLastRead;
    QString range = QString("bytes=%1-").arg(bytesLastRead);

    /* 创建请求 */
    QNetworkRequest header;
    header.setUrl(url);
    header.setRawHeader("Range", range.toLatin1());

    /* 获取响应 */
    reply = manager->get(QNetworkRequest(header));

    /* 关联槽函数：开始下载、更新下载进程、结束下载、下载错误 */
    connect(reply, SIGNAL(readyRead()),
            this,  SLOT  (httpReadyRead()));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
            this,  SLOT  (httpDownloadProgress(qint64, qint64)));
    connect(reply, SIGNAL(finished()),
            this,  SLOT  (httpFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this,  SLOT  (httpError(QNetworkReply::NetworkError)));
}

void Http::httpReadyRead() {
    if (file) {
        file->write(reply->readAll());
    }
}

void Http::httpDownloadProgress(qint64 bytesRead, qint64 totalBytes) {

    /* 更新数据 */
    this->bytesRead = bytesRead;
    this->totalBytes = totalBytes;

    /* 更新进度条 */
    ui->progressBar->setMaximum(totalBytes + bytesLastRead);
    ui->progressBar->setValue(bytesRead + bytesLastRead);
}

void Http::httpFinished() {
    /* 响应结束后销毁相关对象 */
    qDebug() << "下载结束";

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "statusCode = " << statusCode;

    if (statusCode >= 200 && statusCode < 400) {
        qDebug() << "下载成功";
        ui->pushButton_Begin->setEnabled(true);
        ui->pushButton_Pause->setEnabled(false);
    } else {
        qDebug() << "下载失败";
    }

    file->flush();
    file->close();
    delete file;
    file = nullptr;

    reply->deleteLater();
    reply = nullptr;

    totalBytes    = 0;
    bytesRead     = 0;
    bytesLastRead = 0;

    ui->progressBar->hide();
}

void Http::httpError(QNetworkReply::NetworkError error) {

    qDebug() << "Error: " << error;
}

void Http::on_pushButton_Begin_clicked() {
    /* 点击开始下载按钮后执行请求 */
    ui->pushButton_Begin->setEnabled(false);

    /* 首先读取 URL 并解析出文件名 */
    url = ui->lineEdit_Url->text();
    qDebug() << "url = " << url;
    QFileInfo info(url.path());
    qDebug() << "url.path = " << url.path();
    fileName = QString(info.fileName());
    qDebug() << "fileName = " << fileName;

    if (fileName.isEmpty()) {
        fileName = "index.html";
        qDebug() << "文件名为空，缺省为 index.html";
    }

    /* 若文件存在且已下载字节数为 0 则说明下载未开始或重新下载 */
    /* 此时需要把之前存在的临时文件删除 */
//    if (bytesLastRead <= 0) {
//        QFileInfo fileInfo(fileName);
//        if (fileInfo.exists()) {
//            QFile::remove(fileName);
//        }
//    }

    /* 关闭上一次打开的文件 */
    if (file) { delete file; }

    /* 打开文件 */
    file = new QFile(filePath + fileName);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Append)) {
        qDebug() << "打开文件失败";
        delete file;
        file = nullptr;
        return;
    }

    qDebug() << "开始下载";
    bytesRead = 0;
    totalBytes = 0;

    startRequest(url, bytesLastRead);

    ui->pushButton_Pause->setEnabled(true);
    ui->progressBar->setValue(bytesLastRead);
    ui->progressBar->show();
}

void Http::on_pushButton_Select_clicked() {

    filePath = QFileDialog::getExistingDirectory(this, "选择下载路径", "/");

    if (filePath.isEmpty()) {
        qDebug() << "路径名为空，缺省为可执行文件所在路径";
    } else {
        filePath += "/";
        qDebug() << "path = " << filePath;
    }

    ui->lineEdit_Path->setText(filePath);
}

void Http::on_pushButton_Pause_clicked() {

    ui->pushButton_Pause->setEnabled(false);

    bytesLastRead = file->size();

    if (reply) {
        /* 取消槽函数关联 */
        disconnect(reply, SIGNAL(readyRead()),
                   this,  SLOT  (httpReadyRead()));
        disconnect(reply, SIGNAL(downloadProgress(qint64, qint64)),
                   this,  SLOT  (httpDownloadProgress(qint64, qint64)));
        disconnect(reply, SIGNAL(finished()),
                   this,  SLOT  (httpFinished()));
        disconnect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                   this,  SLOT  (httpError(QNetworkReply::NetworkError)));

        reply->abort();
        reply->deleteLater();
        reply = nullptr;
    }

    ui->pushButton_Begin->setEnabled(true);
    ui->progressBar->setValue(bytesLastRead);
    ui->progressBar->show();
}
