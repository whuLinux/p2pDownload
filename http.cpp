#include "http.h"
#include "ui_http.h"

#include <QNetworkRequest>

#include <QDebug>
#include <QFileInfo>
#include <QFileDialog>

Http::Http(QWidget *parent) : QWidget(parent), ui(new Ui::Http) {

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

void Http::startRequest(QUrl url) {

    /* 获取请求的响应 */
    reply = manager->get(QNetworkRequest(url));

    /* 关联槽函数：开始下载、更新下载进程、结束下载 */
    connect(reply, SIGNAL(readyRead()),
            this,  SLOT  (httpReadyRead()));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
            this,  SLOT  (httpDownloadProgress(qint64, qint64)));
    connect(reply, SIGNAL(finished()),
            this,  SLOT  (httpFinished()));
}

void Http::httpReadyRead() {

    if (file) {
        /* 将请求到的响应全部写入文件 */
        qDebug() << "开始下载";
        file->write(reply->readAll());
    } else {
        qDebug() << "打开文件失败";
    }
}

void Http::httpDownloadProgress(qint64 bytesRead, qint64 totalBytes) {

    ui->progressBar->setMaximum(totalBytes);
    ui->progressBar->setValue(bytesRead);
}

void Http::httpFinished() {
    /* 响应结束后销毁相关对象 */
    qDebug() << "下载完毕";
    file->flush();
    file->close();

    reply->deleteLater();
    reply = nullptr;

    delete file;
    file = nullptr;

    ui->progressBar->hide();
}

void Http::on_pushButton_Begin_clicked() {
    /* 点击开始下载按钮后执行请求 */
    /* 首先读取 URL 并解析 */
    url = ui->lineEdit_Url->text();
    qDebug() << "\nurl = " << url;
    QFileInfo info(url.path());
    qDebug() << "url.path = " << url.path();
    QString fileName(info.fileName());
    qDebug() << "fileName = " << fileName;

    if (fileName.isEmpty()) {
        fileName = "index.html";
        qDebug() << "文件名为空，缺省为 index.html";
    }

    /* 新建或打开文件，默认存放于可执行文件所在的路径 */
    file = new QFile(path + fileName);
    if (!file->open(QIODevice::WriteOnly)) {
        qDebug() << "打开文件失败";
        delete file;
        file = nullptr;
        return;
    }

    qDebug() << "开始请求";
    startRequest(url);

    ui->progressBar->setValue(0);
    ui->progressBar->show();
}

void Http::on_pushButton_Select_clicked() {

    path = QFileDialog::getExistingDirectory(this, "选择下载路径", "/");

    if (path.isEmpty()) {
        qDebug() << "路径名为空，缺省为可执行文件所在路径";
    } else {
        path += "/";
        qDebug() << "path = " << path;
    }

    ui->lineEdit_Path->setText(path);
}
