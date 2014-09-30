#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <solarus/Arguments.h>
#include <solarus/MainLoop.h>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    addAction(ui->actionNew_quest);
    addAction(ui->actionLoad_quest);
    addAction(ui->actionExit);
    addAction(ui->actionRun_quest);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionRun_quest_triggered()
{
    // TODO run the open quest instead
    QFileDialog dialog(nullptr, tr("Choose quest directory"));
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    if (!dialog.exec()) {
        return;
    }

    QStringList file_names = dialog.selectedFiles();
    if (file_names.empty()) {
        return;
    }
    dialog.close();

    // TODO run quest in a separate thread
    Solarus::Arguments arguments;
    arguments.add_argument(file_names.first().toStdString());
    Solarus::MainLoop main_loop(arguments);
    main_loop.run();
}

void MainWindow::on_actionExit_triggered()
{
    std::exit(0);
}
