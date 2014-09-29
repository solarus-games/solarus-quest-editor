#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <solarus/CommandLine.h>
#include <solarus/MainLoop.h>

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
    Solarus::CommandLine* command_line;
}

void MainWindow::on_actionExit_triggered()
{
    std::exit(0);
}
