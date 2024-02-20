#include "pub_api.h"
#include "gtk/gtk.h"

static inline std::string _get_filename(GtkWidget *file)
{
    gchar *filename;
    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file));
    gtk_widget_destroy(file);
    return filename;
}
static inline void print_filename(GtkWidget *file)
{
    gchar *filename;
    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file));
    gtk_widget_destroy(file);
    GtkWidget *dialog;
    dialog=gtk_message_dialog_new(NULL,GTK_DIALOG_MODAL,
                                  GTK_MESSAGE_INFO,
                                  GTK_BUTTONS_OK,
                                  filename);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void on_response(GtkWidget *widget , GdkEvent* event, gpointer data){
    printf("on_response\n");
}

static void select_file(GtkWidget *widget,gpointer data)
{
    med_api::SelectFileContext* ctx = (med_api::SelectFileContext*)data;
    GtkWidget *file;
    //GtkFileChooser *chooser;
    //
    file = gtk_file_chooser_dialog_new("SelectFile", NULL,
                                     GTK_FILE_CHOOSER_ACTION_OPEN,
                                     GTK_STOCK_CANCEL,
                                     GTK_RESPONSE_CANCEL,
                                     GTK_STOCK_OK,
                                     GTK_RESPONSE_ACCEPT,
                                       NULL);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file), ctx->multi);

    GtkFileFilter* filter = gtk_file_filter_new();
    //gtk_file_filter_set_name (filter, "*.All");
    if(!ctx->fmts.empty()){
        for(auto& fmt: ctx->fmts){
            gtk_file_filter_add_pattern(filter, fmt.data());
        }
    }
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (file), filter);
    gtk_window_set_focus(nullptr, file);
    //
    //chooser = GTK_FILE_CHOOSER (file);
   // gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);

    if(gtk_dialog_run(GTK_DIALOG(file))==GTK_RESPONSE_ACCEPT){
        // print_filename(file);
        GSList * list = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(file));
        //int c = g_slist_length(list);
        int nIndex;
        GSList *node;
        for (nIndex = 0; node = g_slist_nth (list, nIndex); nIndex++) {
           std::string str = (char*)node->data;
           ctx->files.push_back(std::move(str));
        }
        g_slist_free(list);
    }
    //gtk_widget_hide_on_delete(file);
    gtk_widget_destroy(file);
}

namespace med_api {

static void show_upload_dlg_gtk(SelectFileContext* ctx){

    int argc = 1;
    char** arr = (char**)malloc(sizeof(void*));
    arr[0] = "test.gtk";
    gtk_init(&argc, (char***)&arr);
    free(arr);

    //
    select_file(NULL, ctx);

//    GtkWidget *win;
//    GtkWidget *button;
//    win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
//    gtk_window_set_title(GTK_WINDOW(win), "FileDialog");
//    gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
//    g_signal_connect(G_OBJECT(win),"destroy", G_CALLBACK(gtk_main_quit),NULL);
//    button = gtk_button_new_with_label("Click Me");
//    gtk_container_add(GTK_CONTAINER(win), button);
//    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(select_file),ctx);
//    gtk_widget_show_all(win);
    //gtk_main();
}

ShowUploadHelper::~ShowUploadHelper(){
}

void ShowUploadHelper::init(){
    std::string str = "test.gtk";
    int argc = 1;
    char** arr = (char**)malloc(sizeof(void*));
    arr[0] = (char*)str.data();
    gtk_init(&argc, (char***)&arr);
    free(arr);
}
void ShowUploadHelper::showDlg(SelectFileContext* ctx){
    select_file(NULL, ctx);
}
void ShowUploadHelper::showGtkDlg(SelectFileContext* ctx){
    //empty
    select_file(NULL, ctx);
}
}
