#ifndef PUBAPI_H
#define PUBAPI_H

#include <string>
#include <vector>
#include <functional>

namespace med_api {
    struct SelectFileContext{
        std::vector<std::string> fmts {{"*.*"}};
        bool multi {false};
        bool init {true};
        std::vector<std::string> files;
    };

    class ShowUploadHelper{
    public:
        ~ShowUploadHelper();
        void init();
        void showDlg(SelectFileContext* ctx);
        void showGtkDlg(SelectFileContext* ctx);

        int exec();
    private:
        void* m_ptr {nullptr};
    };

    void qt_handler_post(std::function<void()> func, int delayMs = 0);
    void qt_handler_postAsync(std::function<void()> func, int delayMs = 0);
}

#endif // PUBAPI_H
