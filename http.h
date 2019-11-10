#ifndef HTTP_H
#define HTTP_H

#include <QWidget>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QUrl>
#include <QFile>
#include <QTime>

QT_BEGIN_NAMESPACE
    namespace Ui {
        class Http;
    }
QT_END_NAMESPACE

class Http : public QWidget {

    Q_OBJECT

public:
    Http(QWidget *parent = nullptr);
    ~Http();

    void startRequest(QUrl url, qint64 bytesRead);

    QUrl getUrl() { return url; }

private slots:
    void httpReadyRead();
    void httpDownloadProgress(qint64 bytesRead, qint64 totalBytes);
    void httpFinished();
    void httpError(QNetworkReply::NetworkError error);

    void on_pushButton_Begin_clicked();

    void on_pushButton_Select_clicked();

    void on_pushButton_Pause_clicked();

private:
    Ui::Http                *ui;

    QNetworkAccessManager   *manager;
    QNetworkReply           *reply;
    QUrl                     url;

    QFile                   *file;
    QString                  fileName;
    QString                  filePath;

    qint64                   totalBytes;        // 本次下载过程总共需要下载的字节数
    qint64                   bytesRead;         // 本次下载过程中已下载的字节数
    qint64                   bytesLastRead;     // 上一次断点处已下载的字节数
};

#endif // HTTP_H
