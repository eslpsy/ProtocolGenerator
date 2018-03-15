#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QFileDialog>
#include <QStringListModel>

#include "protocolfilereader.h"
#include "cppgenerator.h"
#include "luagenerator.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_select_xml_path_btn_clicked();
    void on_select_cpp_path_btn_clicked();
    void on_select_lua_path_btn_clicked();
    void on_generate_one_btn_clicked();

    void OnXmlDirSelected(const QString &dir_path_str);
    void OnCppDirSelected(const QString &dir_path_str);
    void OnLuaDirSelected(const QString &dir_path_str);

    void on_generate_all_btn_clicked();

protected:
    void LoadPathFromFile();
    void SavePathToFile();
    void ShowXmlList();
    void GenerateSelectXml();
    void GenerateOneAll();
    bool DoGenerate(const QString &xml_file_name);

private:
    Ui::MainWindow *ui;

    QFileDialog xml_path_dialog_;
    QFileDialog cpp_path_dialog_;
    QFileDialog lua_path_dialog_;

    QString xml_path_;
    QString cpp_path_;
    QString lua_path_;

    QStringListModel xml_list_model_;

    ProtocolFileReader reader_;
    CppGenerator cpp_generator_;
    LuaGenerator lua_generator_;
};

#endif // MAINWINDOW_H
