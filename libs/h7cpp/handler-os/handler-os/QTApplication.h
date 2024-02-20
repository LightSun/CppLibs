#pragma once

#ifdef BUILD_WITH_QT
#include <QApplication>
#include <QObject>
#include <memory>

namespace h7_handler_os {

typedef struct _QTApplication_ctx _QTApplication_ctx;

class QTEventInterceptor{
public:
    virtual bool intercept(QObject *obj, QEvent *event) = 0;
};

class QTApplication: public QApplication
{
    Q_OBJECT
public:
    //must use &
    QTApplication(int& argc, char** argv);
    ~QTApplication();

    static QTApplication* get();
    void postEvent2(QObject *receiver, QEvent *event);

    void setIdleTimeThreshold(int msec);

    void setEventInterceptor(std::shared_ptr<QTEventInterceptor> inter){
        m_interceptor = inter;
    }
    _QTApplication_ctx* getAppCtx(){
        return m_ctx;
    }
protected:
    bool notify(QObject *obj, QEvent *event) override;
    //bool event(QEvent *) override;

private:
    _QTApplication_ctx* m_ctx{nullptr};
    std::shared_ptr<QTEventInterceptor> m_interceptor;
};
}
#endif
