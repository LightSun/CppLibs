#pragma once

#ifdef BUILD_WITH_QT
#include <QApplication>
#include <QObject>

namespace h7_handler_os {

typedef struct _QTApplication_ctx _QTApplication_ctx;

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

    h7_handler_os::_QTApplication_ctx* getAppCtx(){
        return m_ctx;
    }
protected:
    virtual bool notify(QObject *obj, QEvent *event) override;

private:
    h7_handler_os::_QTApplication_ctx* m_ctx{nullptr};
};
}
#endif
