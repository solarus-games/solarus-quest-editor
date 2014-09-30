#include "quest_manager.h"
#include <QFile>

/**
 * @brief Creates a quest manager.
 * @param parent The parent QObject or nullptr.
 */
QuestManager::QuestManager(QObject* parent) :
  QObject(parent) {
  setObjectName("quest_manager");
}

/**
 * @brief Returns the path of the current quest.
 * @return The current quest path (above the data directory).
 */
QString QuestManager::get_quest_path() {
  return quest_path;
}

/**
 * @brief Sets another quest as the current one.
 * @param quest_path The quest path to open.
 * @return @c true in case of success, @c false if no quest was found in this
 * directory (the quest path is left unchanged in this case).
 */
bool QuestManager::set_quest_path(QString quest_path) {

  bool success = QFile(quest_path + "/data/quest.dat").exists();

  if (success) {
    this->quest_path = quest_path;
    emit current_quest_changed(quest_path);
  }
  return success;
}

/**
 * @brief Returns the name of the quest directory.
 *
 * The name returned is the last component of the quest path.
 *
 * @return The name of the quest directory.
 */
QString QuestManager::get_quest_name() {

  int index = quest_path.lastIndexOf('/');
  if (index == -1) {
    return quest_path;
  }

  return quest_path.section('/', -1);
}
