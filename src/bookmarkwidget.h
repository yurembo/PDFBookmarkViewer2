#ifndef BOOKMARKWIDGET_H
#define BOOKMARKWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QTreeView>
#include <QModelIndex>

class BookmarkManager;

class BookmarkWidget : public QWidget
{
    Q_OBJECT
public:
    BookmarkWidget(BookmarkManager *mgr, QWidget *p = nullptr);

signals:
    void bookmarkActivated(int pageNum);

protected:
    void contextMenuEvent(QContextMenuEvent *) override;

private slots:
    void onAdd();
    void onRemove();
    void onRename();
    void onMoveUp();
    void onMoveDown();
    void onClicked(const QModelIndex &idx);

private:
    void refreshView();

    BookmarkManager *m_mgr;
    QStandardItemModel m_model;
    QTreeView *m_tree;
};

#endif
