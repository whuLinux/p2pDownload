#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QUrl>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
    class DownloadManager;
}
QT_END_NAMESPACE

const int MAX_THREAD_COUNT = 16;                // 最大线程数量
const int PART_SIZE = 1024 * 1024 * 20;         // 文件分块的大小，默认为 20Mb

/*
    进行下载任务的分配调度、提供 GUI 界面
*/
class DownloadManager : public QMainWindow {

    Q_OBJECT

public:
    DownloadManager(QWidget *parent = Q_NULLPTR);
    DownloadManager(bool isPartner, int taskIndex, QUrl url, qint64 begin, qint64 end, QWidget *parent = Q_NULLPTR);
    ~DownloadManager();

    static qint64  getFileSize(QUrl url);                       // 根据 URL 获取文件大小
    static QString getFileName(QUrl url);                       // 根据 URL 获取文件名
    static int     getPartCount(qint64 fileSize, qint64 divideSize = PART_SIZE, int amount = MAX_THREAD_COUNT);
                                                                // 根据文件大小计算分块数目
    int  startDownload();                                       // 开始下载
    void pauseDownload();                                       // 暂停下载

private:
    QString getFilePath();                                      // 指定下载路径

    void downloadFinished();

signals:
    void signalContinueDownload();
    void signalPauseDownload();

public slots:
    void onSubThreadFinished();                                 // 子线程下载完毕后检测下载任务是否结束
    void onSubDownloadProgress(int index, qint64 bytesRead);    // 子线程要求更新下载进度

private slots:
    void on_pushButton_Start_clicked();
    void on_pushButton_Path_clicked();
    void on_pushButton_Pause_clicked();

private:
    Ui::DownloadManager *ui;

    bool    isPartner;
    int     taskIndex;

    QUrl    url;
    qint64  begin;
    qint64  end;

    QString path;
    QString name;
    qint64  size;

    qint64  lastStartSecond;        // 记录上一次开始下载的时间
    qint64  totalSecond;            // 下载总用时（暂停时不算在内）
    bool    isFromStart;            // 用于区分按下开始按钮时是初次开始下载还是暂停后继续下载

    int     threadCount;            // 划分的线程数
    int     finishedThreadCount;    // 已完成的线程数
    qint64  *bytesRead;             // 记录各线程已下载的进度
};

#endif // DOWNLOADMANAGER_H
