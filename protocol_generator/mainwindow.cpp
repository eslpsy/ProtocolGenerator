#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QListView>
#include <QTextStream>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), xml_path_dialog_(this), cpp_path_dialog_(this), lua_path_dialog_(this),
    xml_path_("Empty"), cpp_path_("Empty"), lua_path_("Empty")
{
    ui->setupUi(this);

    xml_path_dialog_.setFileMode(QFileDialog::Directory);
    xml_path_dialog_.setAcceptMode(QFileDialog::AcceptSave);
    connect(&xml_path_dialog_,SIGNAL(fileSelected ( const QString & )),this,SLOT(OnXmlDirSelected(const QString & )));

    cpp_path_dialog_.setFileMode(QFileDialog::Directory);
    cpp_path_dialog_.setAcceptMode(QFileDialog::AcceptSave);
    connect(&cpp_path_dialog_,SIGNAL(fileSelected ( const QString & )),this,SLOT(OnCppDirSelected(const QString & )));

    lua_path_dialog_.setFileMode(QFileDialog::Directory);
    lua_path_dialog_.setAcceptMode(QFileDialog::AcceptSave);
    connect(&lua_path_dialog_,SIGNAL(fileSelected ( const QString & )),this,SLOT(OnLuaDirSelected(const QString & )));

    this->LoadPathFromFile();
}

MainWindow::~MainWindow()
{
    disconnect(&xml_path_dialog_,SIGNAL(fileSelected ( const QString & )),this,SLOT(OnXmlDirSelected(const QString & )));
    disconnect(&cpp_path_dialog_,SIGNAL(fileSelected ( const QString & )),this,SLOT(OnCppDirSelected(const QString & )));
    disconnect(&lua_path_dialog_,SIGNAL(fileSelected ( const QString & )),this,SLOT(OnLuaDirSelected(const QString & )));

    delete ui;
}

void MainWindow::on_select_xml_path_btn_clicked()
{
    xml_path_dialog_.show();
}

void MainWindow::on_select_cpp_path_btn_clicked()
{
    cpp_path_dialog_.show();
}

void MainWindow::on_select_lua_path_btn_clicked()
{
    lua_path_dialog_.show();
}

void MainWindow::on_generate_one_btn_clicked()
{
    this->GenerateSelectXml();
}

void MainWindow::on_generate_all_btn_clicked()
{
    this->GenerateOneAll();
}

void MainWindow::OnXmlDirSelected(const QString &dir_path_str)
{
    ui->select_xml_path_txt->setText(dir_path_str);
    xml_path_ = dir_path_str;

    this->SavePathToFile();
    this->ShowXmlList();
}

void MainWindow::OnCppDirSelected(const QString &dir_path_str)
{
    ui->select_cpp_path_txt->setText(dir_path_str);
    cpp_path_ = dir_path_str;

    this->SavePathToFile();
}

void MainWindow::OnLuaDirSelected(const QString &dir_path_str)
{
    ui->select_lua_path_txt->setText(dir_path_str);
    lua_path_ = dir_path_str;

    this->SavePathToFile();
}

void MainWindow::LoadPathFromFile()
{
    QFile path_file("PATH_BACKUP");
    if (path_file.open(QFile::ReadOnly))
    {
        QTextStream in(&path_file);
        in >> xml_path_ >> cpp_path_ >> lua_path_;

        ui->select_xml_path_txt->setText(xml_path_);
        ui->select_cpp_path_txt->setText(cpp_path_);
        ui->select_lua_path_txt->setText(lua_path_);

        this->ShowXmlList();
    }
}

void MainWindow::SavePathToFile()
{
    QFile path_file("PATH_BACKUP");
    if (path_file.open(QFile::WriteOnly))
    {
        QTextStream out(&path_file);
        out << xml_path_ << "\n" << cpp_path_ << "\n" << lua_path_;
    }
}

void MainWindow::ShowXmlList()
{
    QDir dir(xml_path_);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Size | QDir::Reversed);

    QStringList str_list;
    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        str_list << fileInfo.fileName();
    }
    xml_list_model_.setStringList(str_list);
    ui->xml_list_view->setModel(&xml_list_model_);
}

void MainWindow::GenerateSelectXml()
{
    QModelIndex cur_index = ui->xml_list_view->currentIndex();
    if (true)
    {
        QString file_name = cur_index.data().toString();
        ui->generate_log_txt->clear();
        ui->generate_log_txt->append("开始生成[" + file_name + "] ：");
        if (this->DoGenerate(file_name))
        {

        }
    }
}

void MainWindow::GenerateOneAll()
{
    QDir dir(xml_path_);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Size | QDir::Reversed);

    QFileInfoList list = dir.entryInfoList();
    ui->generate_log_txt->clear();
    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);

        QString file_name = fileInfo.fileName();
        ui->generate_log_txt->append("generating  [" + file_name + "] :");
        if (this->DoGenerate(file_name))
        {
        }
    }
}

bool MainWindow::DoGenerate(const QString &xml_file_name)
{
    if (!reader_.Reload(xml_path_ + "/" + xml_file_name))
    {
        ui->statusBar->showMessage("文件（" + xml_file_name + "）读取出错 : " + reader_.GetLastErrorStr());
        return false;
    }

    ui->statusBar->showMessage("正在生成" + xml_file_name + "....");

    QString proto_name = xml_file_name.mid(0, xml_file_name.lastIndexOf("."));
    QString cpp_file_name = proto_name + ".cpp";
    if (cpp_generator_.Generate(reader_, cpp_path_ + "/" + proto_name))
        ui->generate_log_txt->append("  " + proto_name + ".xpp    [Succ]");
    else
        ui->generate_log_txt->append("  " + proto_name + ".xpp    [Fail]");

    QString lua_file_name = proto_name + ".lua";
    if (lua_generator_.Generate(reader_, lua_path_ + "/" + lua_file_name))
        ui->generate_log_txt->append("  " + lua_file_name + "    [Succ]");
    else
        ui->generate_log_txt->append("  " + lua_file_name + "    [Fail]");

    ui->statusBar->showMessage("生成完毕");

    return true;
}
