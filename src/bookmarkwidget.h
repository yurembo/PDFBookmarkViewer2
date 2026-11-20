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
    void bookmarkActivated(int pageNum, double zoom);

private slots:
    void onRename(const QModelIndex &idx);
    void onRemove(const QModelIndex &idx);
    void onMoveUp(const QModelIndex &idx);
    void onMoveDown(const QModelIndex &idx);
    void onClicked(const QModelIndex &idx);
    void showContextMenu(const QPoint &pos);

private:
    void refreshView();
    bool isSeparator(const QModelIndex &idx) const;
    bool isUserBookmark(const QModelIndex &idx) const;

    BookmarkManager *m_mgr;
    QStandardItemModel m_model;
    QTreeView *m_tree;
};

#endif
