#include "quest_manager.h"
#include "main_window.h"
#include <solarus/Arguments.h>
#include <solarus/MainLoop.h>
#include <solarus/SolarusFatal.h>
#include <solarus/lowlevel/Debug.h>
#include <QFileDialog>
#include <QMessageBox>
#include <iostream>

/**
 * @brief Creates a main window.
 * @param parent The parent widget or nullptr.
 * @param quest_manager The quest manager.
 */
MainWindow::MainWindow(QWidget* parent, QuestManager& quest_manager) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  quest_manager(quest_manager) {

  ui->setupUi(this);

  // Add menu actions to this widget so that key shortcuts work
  // when menus are closed.
  addAction(ui->actionNew_quest);
  addAction(ui->actionLoad_quest);
  addAction(ui->actionExit);
  addAction(ui->actionRun_quest);

  connect(&quest_manager, SIGNAL(current_quest_changed(QString)),
          this, SLOT(current_quest_changed(QString)));
}

/**
 * @brief Returns the quest manager.
 * @return The quest manager.
 */
QuestManager& MainWindow::get_quest_manager() {
  return quest_manager;
}

/**
 * @brief Slot called when the user triggers the "Exit" action.
 */
void MainWindow::on_actionExit_triggered() {
  std::exit(0);
}

/**
 * @brief Slot called when the user triggers the "Load quest" action.
 */
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

  QString quest_path = file_names.first();
  if (!quest_manager.set_quest_path(quest_path)) {
    QMessageBox messageBox;
    messageBox.setIcon(QMessageBox::Warning);
    messageBox.setText("No quest was found in directory\n'" + quest_path + "'");
    messageBox.exec();
  }
}

/**
 * @brief Slot called when the user triggers the "Run quest" action.
 */
void MainWindow::on_actionRun_quest_triggered() {

  QString quest_path = quest_manager.get_quest_path();
  if (quest_path.isEmpty()) {
    return;
  }

  // TODO run quest in a separate thread
  try {
    Solarus::Arguments arguments;
    arguments.add_argument(quest_path.toStdString());
    Solarus::MainLoop main_loop(arguments);
    main_loop.run();
  }
  catch (const Solarus::SolarusFatal& ex) {
    // The run did not end well.
    std::cout << "Quest terminated unexpectedly: " << ex.what() << std::endl;
  }
}

/**
 * @brief Slot called when the user opens another quest.
 */
void MainWindow::current_quest_changed(QString /* quest_path */) {
  update_title();
}

/**
 * @brief Updates the title of the window from the current quest.
 */
void MainWindow::update_title() {

  QString title = "Solarus Quest Editor";
  QString quest_name = quest_manager.get_quest_name();
  if (!quest_name.isEmpty()) {
    title = quest_name + " - " + title;
  }

  setWindowTitle(title);
}
