
#include "handler-os/QTApplication.h"
#include "handler-os/Looper.h"

using namespace h7_handler_os;

#ifdef BUILD_WITH_QT
#include <QWidget>
#include <QPushButton>

#define CONN_0(src, src_name, dst, dst_name) \
QObject::connect(src, SIGNAL(src_name()), dst, SLOT(dst_name()))


class TestReceiver: public QObject{
    Q_OBJECT
public slots:
    void onClicked(){
        printf("TestReceiver >> onClicked...\n");
        //Looper::myLooper()
    }
};
#endif

int test_qt_handler(int argc, char* argv[]){

    setbuf(stdout, NULL);
#ifdef BUILD_WITH_QT
    QTApplication app(argc, argv);
    //QApplication app(argc, argv);
    TestReceiver* re = new TestReceiver();

    QWidget window;
    window.setWindowTitle("QPushButton Example");
    window.resize(300, 200);

    QPushButton *button = new QPushButton(&window);
    button->setText("Click Me!");
    button->setGeometry(100, 50, 100, 50);

    CONN_0(button, released, re, onClicked);
    window.show();

    return app.exec();
#endif
}

#include "test_qt_handler.moc"
