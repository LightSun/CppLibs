#include "test_qt_handler.h"

int test_qt_handler(int argc, char* argv[]){
#ifdef BUILD_WITH_QT
    h7_handler_os::QTApplication app(argc, argv);
#endif
#ifdef BUILD_WITH_QT
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
    return 0;
}

//#include "test_qt_handler.moc"
