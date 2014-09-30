#include "mainwindow.h"
#include <solarus/Arguments.h>
#include <solarus/MainLoop.h>
#include <QFileDialog>

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {

    ui->setupUi(this);

    // Add menu actions to this widget so that key shortcuts work
    // when menus are closed.
    addAction(ui->actionNew_quest);
    addAction(ui->actionLoad_quest);
    addAction(ui->actionExit);
    addAction(ui->actionRun_quest);
}

QString MainWindow::get_quest_path() {
    return quest_path;
}

void MainWindow::set_quest_path(QString quest_path) {
    this->quest_path = quest_path;
}

void MainWindow::on_actionRun_quest_triggered() {
    if (quest_path.isEmpty()) {
        return;
    }

    // TODO run quest in a separate thread
    Solarus::Arguments arguments;
    arguments.add_argument(quest_path.toStdString());
    Solarus::MainLoop main_loop(arguments);
    main_loop.run();
}

void MainWindow::on_actionExit_triggered() {
    std::exit(0);
}

void MainWindow::on_actionLoad_quest_triggered() {
    QFileDialog dialog(nullptr, tr("Select quest directory"));
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    if (!dialog.exec()) {
        return;
    }

    QStringList file_names = dialog.selectedFiles();
    if (file_names.empty()) {
        return;
    }

    set_quest_path(file_names.first());
}
