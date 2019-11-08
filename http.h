#ifndef HTTP_H
#define HTTP_H

#include <QWidget>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QUrl>
#include <QFile>

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

    void startRequest(QUrl url);

private slots:
    void httpReadyRead();
    void httpDownloadProgress(qint64 bytesRead, qint64 totalBytes);
    void httpFinished();

    void on_pushButton_Begin_clicked();

    void on_pushButton_Select_clicked();

private:
    Ui::Http *ui;

    QNetworkAccessManager   *manager;
    QNetworkReply           *reply;
    QFile                   *file;
    QUrl                     url;
    QString                  path;
};

#endif // HTTP_H
