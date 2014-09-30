#ifndef SOLARUSEDITOR_QUESTMANAGER_H
#define SOLARUSEDITOR_QUESTMANAGER_H

#include <QObject>

/**
 * @brief Stores the current quest and sends signals when it changes.
 */
class QuestManager : public QObject {
  Q_OBJECT

public:

  explicit QuestManager(QObject* parent = nullptr);

  QString get_quest_path();
  bool set_quest_path(QString quest_path);

  QString get_quest_name();

signals:

  void current_quest_changed(QString quest_path);

  // TODO
  // resource_element_added
  // resource_element_removed
  // resource_element_moved
  // resource_element_renamed

private:

  QString quest_path;   /**< Path of the current quest. */
};

#endif
