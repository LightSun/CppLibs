#include "pub_api.h"
#include <QFileDialog>
#include <QApplication>
#include <QTextCodec>
#include <QDebug>
#include "qt_Handler.h"

static inline std::string _cal_fmts(med_api::SelectFileContext* ctx){
    std::string fmts;
    for(int i = 0 ; i < (int)ctx->fmts.size() ; ++i){
        fmts += ctx->fmts[i];
        if(i != (int)ctx->fmts.size() - 1){
            fmts += " ";
        }
    }
    return fmts;
}

namespace med_api {

ShowUploadHelper::~ShowUploadHelper(){
    if(m_ptr){
        delete (QApplication*)m_ptr;
        m_ptr = nullptr;
    }
}

void ShowUploadHelper::init(){
   // QApplication app(argc, (char**)args);
    if(m_ptr == nullptr){
        const char* args[1];
        args[0]="test_ui.exe";
        int argc = 1;
        m_ptr = new QApplication(argc, (char**)args);
        qDebug() << "ShowUploadHelper::init...";
        //auto codec = QTextCodec::codecForName("utf-8");
        //QTextCodec::setCodecForLocale(codec);


        h7::handler_post([](){
            qDebug() << "qt_handler::init ok";
        });
    }
}
void ShowUploadHelper::showDlg(SelectFileContext* ctx){
    QString fmt = QString::fromStdString(_cal_fmts(ctx));
    if(ctx->multi){
        QStringList _list = QFileDialog::getOpenFileNames(nullptr,
                                                                  "请选择文件", "C:/Users",
                  "生信相关文件("+ fmt + ");;所有文件（*.*);;"
                                                                  );
        for (int i = 0; i < _list.size(); i++){
            QString& str_path = _list[i];
            QByteArray ba = str_path.toUtf8();
            //ctx->files.push_back(str_path.toLatin1());
            ctx->files.push_back(std::string(ba.constData(), ba.length()));
        }
    }else{
        QString _file = QFileDialog::getOpenFileName(nullptr,
                                                                  "请选择文件", "C:/Users",
                  "生信相关文件("+ fmt + ");;所有文件（*.*);;"
                                                                  );
        QByteArray ba = _file.toUtf8();
        ctx->files.push_back(std::string(ba.constData(), ba.length()));
    }
}
void ShowUploadHelper::showGtkDlg(SelectFileContext* ctx){
    //empty
    showDlg(ctx);
}

int ShowUploadHelper::exec(){
    if(m_ptr){
        return ((QApplication*)m_ptr)->exec();
    }
    return 0;
}

void qt_handler_post(std::function<void()> func, int delayMs){
    h7::handler_post(func, delayMs);
}
void qt_handler_postAsync(std::function<void()> func, int delayMs){
    h7::handler_postAsync(func, delayMs);
}


}
