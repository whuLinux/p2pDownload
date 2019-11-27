#ifndef RECQTIMER_H
#define RECQTIMER_H
#include<QTimer>

/**
 * @brief The RecQTimer class
 * 接收Qtimer timeout 的信号，将之附上token后转 发
 */
class RecQTimer:public QTimer
{
    Q_OBJECT

private:
    qint32 token;
public:
    RecQTimer();
    inline void setToken(qint32 token);

public slots:
    /**
     * @brief acceptTimeOut
     * 接收父类QTimer的timeout信号，转发
     */
    void acceptTimeOut();

signals:
    /**
     * @brief recordTimeOut
     * 转发token给主控模块
     * @param token 异常主机任务的token
     */
    //TODO:待连接信号槽
    void recordTimeOut(qint32 token);
};

void RecQTimer::setToken(qint32 token)
{
    this->token=token;
}

#endif // RECQTIMER_H
