#include "bookmarkwidget.h"
#include "bookmarkmanager.h"
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QCursor>
#include <QContextMenuEvent>

BookmarkWidget::BookmarkWidget(BookmarkManager *mgr, QWidget *p)
    : QWidget(p)
    , m_mgr(mgr)
{
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);

    m_tree = new QTreeView(this);
    m_tree->setModel(&m_model);
    m_tree->header()->hide();
    m_tree->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    m_tree->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tree->setIndentation(16);
    m_tree->setStyleSheet(
        "QTreeView { background-color: #1e1e1e; border: none; color: #d4d4d4; font-size: 12px; }"
        "QTreeView::item { padding: 4px 8px; }"
        "QTreeView::item:selected { background-color: #094771; color: white; }"
        "QTreeView::item:hover:!selected { background-color: #2a2d2e; }");

    lay->addWidget(m_tree);

    connect(m_tree, &QTreeView::clicked, this, &BookmarkWidget::onClicked);
    connect(m_mgr, &BookmarkManager::bookmarksChanged, this, [this]() { refreshView(); });
}

void BookmarkWidget::onClicked(const QModelIndex &idx)
{
    if (!idx.isValid()) return;
    QString id = idx.data(Qt::UserRole).toString();
    if (id.isEmpty()) return;
    
    int index = m_mgr->indexOf(id);
    if (index >= 0) {
        Bookmark *bm = m_mgr->bookmarkAt(index);
        if (bm) emit bookmarkActivated(bm->pageNumber);
    }
}

void BookmarkWidget::refreshView()
{
    m_model.clear();
    m_model.setColumnCount(1);
    const auto &bms = m_mgr->bookmarks();
    for (int i = 0; i < bms.size(); ++i) {
        const Bookmark &bm = bms.at(i);
        QString t = QString("%1  (стр. %2)").arg(bm.title).arg(bm.pageNumber + 1);
        auto *item = new QStandardItem(t);
        item->setData(bm.id, Qt::UserRole);
        m_model.appendRow(item);
    }
}

void BookmarkWidget::contextMenuEvent(QContextMenuEvent *ev)
{
    QMenu menu(this);

    QAction *addAct = menu.addAction("+ Добавить закладку");
    QAction *renameAct = menu.addAction("Переименовать");
    QAction *moveUpAct = menu.addAction("Переместить вверх");
    QAction *moveDownAct = menu.addAction("Переместить вниз");
    QAction *removeAct = menu.addAction("Удалить");

    QModelIndex idx = m_tree->indexAt(ev->pos());
    bool has = idx.isValid();
    renameAct->setEnabled(has);
    moveUpAct->setEnabled(has && idx.row() > 0);
    moveDownAct->setEnabled(has && idx.row() < m_model.rowCount() - 1);
    removeAct->setEnabled(has);

    connect(addAct, &QAction::triggered, this, &BookmarkWidget::onAdd);
    connect(renameAct, &QAction::triggered, this, &BookmarkWidget::onRename);
    connect(removeAct, &QAction::triggered, this, &BookmarkWidget::onRemove);
    connect(moveUpAct, &QAction::triggered, this, &BookmarkWidget::onMoveUp);
    connect(moveDownAct, &QAction::triggered, this, &BookmarkWidget::onMoveDown);

    menu.exec(ev->globalPos());
}

void BookmarkWidget::onAdd()
{
    // Сигнал обрабатывается в MainWindow
}

void BookmarkWidget::onRemove()
{
    QModelIndex idx = m_tree->indexAt(QCursor::pos());
    if (!idx.isValid()) return;
    QString id = idx.data(Qt::UserRole).toString();
    if (!id.isEmpty()) m_mgr->removeBookmark(id);
}

void BookmarkWidget::onRename()
{
    QModelIndex idx = m_tree->indexAt(QCursor::pos());
    if (!idx.isValid()) return;
    QString id = idx.data(Qt::UserRole).toString();
    QString cur = idx.data(Qt::DisplayRole).toString();
    int sp = cur.indexOf("  (стр.");
    QString title = (sp >= 0) ? cur.left(sp) : cur;

    bool ok;
    QString t = QInputDialog::getText(this, "Переименовать",
        "Название закладки:", QLineEdit::Normal, title, &ok);
    if (ok && !t.trimmed().isEmpty())
        m_mgr->updateBookmark(id, t.trimmed());
}

void BookmarkWidget::onMoveUp()
{
    QModelIndex idx = m_tree->indexAt(QCursor::pos());
    if (!idx.isValid() || idx.row() == 0) return;
    m_mgr->moveBookmark(idx.data(Qt::UserRole).toString(), -1);
}

void BookmarkWidget::onMoveDown()
{
    QModelIndex idx = m_tree->indexAt(QCursor::pos());
    if (!idx.isValid()) return;
    m_mgr->moveBookmark(idx.data(Qt::UserRole).toString(), 1);
}
