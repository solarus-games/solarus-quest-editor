#ifndef SOLARUSEDITOR_QUEST_TREE_VIEW_H
#define SOLARUSEDITOR_QUEST_TREE_VIEW_H

#include "quest_manager.h"
#include <QPointer>
#include <QTreeView>

/**
 * @brief A view of the quest files.
 */
class QuestTreeView : public QTreeView {
  Q_OBJECT

public:
  explicit QuestTreeView(QWidget* parent = nullptr);

  void set_quest_manager(QuestManager& quest_manager);

signals:

public slots:
  void current_quest_changed(QString quest_path);

private:

  QPointer<QuestManager> quest_manager;    /**< The quest manager observed or nullptr. */

};

#endif
