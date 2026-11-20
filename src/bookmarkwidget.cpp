#include "bookmarkwidget.h"
#include "bookmarkmanager.h"
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QFont>
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
    if (!idx.isValid() || isSeparator(idx)) return;

    QString id = idx.data(Qt::UserRole).toString();
    if (id.isEmpty()) return;

    // Ищем индекс ID в userBookmarks
    int index = -1;
    const auto &ubs = m_mgr->userBookmarks();
    for (int i = 0; i < ubs.size(); ++i) {
        if (ubs[i].id == id) {
            index = i;
            break;
        }
    }

    if (index >= 0) {
        emit bookmarkActivated(ubs[index].pageNumber);
    }
}

void BookmarkWidget::refreshView()
{
    m_model.clear();
    m_model.setColumnCount(1);

    const auto &all = m_mgr->allBookmarks();
    for (const auto &bm : all) {
        auto *item = new QStandardItem();
        item->setData(bm.id, Qt::UserRole);

        if (bm.id == "---separator---") {
            // Разделитель "Закладки"
            item->setText(bm.title);
            QFont font;
            font.setBold(true);
            font.setPointSize(11);
            item->setFont(font);
            item->setForeground(QBrush(QColor(0, 120, 215)));
            item->setEditable(false);
        } else {
            // Обычная закладка
            QString t = QString("%1  (стр. %2)").arg(bm.title).arg(bm.pageNumber + 1);
            item->setText(t);
        }

        m_model.appendRow(item);
    }
}

bool BookmarkWidget::isSeparator(const QModelIndex &idx) const
{
    if (!idx.isValid()) return true;
    return idx.data(Qt::UserRole).toString() == "---separator---";
}

bool BookmarkWidget::isUserBookmark(const QModelIndex &idx) const
{
    if (!idx.isValid()) return false;
    QString id = idx.data(Qt::UserRole).toString();
    return !id.isEmpty() && id != "---separator---";
}

void BookmarkWidget::contextMenuEvent(QContextMenuEvent *ev)
{
    QMenu menu(this);

    QModelIndex idx = m_tree->indexAt(ev->pos());

    if (isSeparator(idx)) {
        // Контекстное меню для раздела "Закладки" — только добавить
        QAction *addAct = menu.addAction("+ Добавить закладку");
        connect(addAct, &QAction::triggered, this, &BookmarkWidget::onAdd);
    } else if (isUserBookmark(idx)) {
        // Контекстное меню для пользовательской закладки
        QAction *addAct = menu.addAction("+ Добавить закладку");
        QAction *renameAct = menu.addAction("✎ Переименовать");
        QAction *moveUpAct = menu.addAction("▲ Переместить вверх");
        QAction *moveDownAct = menu.addAction("▼ Переместить вниз");
        QAction *removeAct = menu.addAction("✕ Удалить");

        connect(addAct, &QAction::triggered, this, &BookmarkWidget::onAdd);
        connect(renameAct, &QAction::triggered, this, &BookmarkWidget::onRename);
        connect(removeAct, &QAction::triggered, this, &BookmarkWidget::onRemove);
        connect(moveUpAct, &QAction::triggered, this, &BookmarkWidget::onMoveUp);
        connect(moveDownAct, &QAction::triggered, this, &BookmarkWidget::onMoveDown);
    }
    // Для PDF outline закладок — контекстное меню не показываем

    menu.exec(ev->globalPos());
}

void BookmarkWidget::onAdd()
{
    // Сигнал обрабатывается в MainWindow
    emit bookmarkActivated(-2); // -2 означает "добавить закладку"
}

void BookmarkWidget::onRemove()
{
    QModelIndex idx = m_tree->indexAt(QCursor::pos());
    if (!isUserBookmark(idx)) return;
    QString id = idx.data(Qt::UserRole).toString();
    if (!id.isEmpty()) m_mgr->removeUserBookmark(id);
}

void BookmarkWidget::onRename()
{
    QModelIndex idx = m_tree->indexAt(QCursor::pos());
    if (!isUserBookmark(idx)) return;
    QString id = idx.data(Qt::UserRole).toString();
    QString cur = idx.data(Qt::DisplayRole).toString();
    int sp = cur.indexOf("  (стр.");
    QString title = (sp >= 0) ? cur.left(sp) : cur;

    bool ok;
    QString t = QInputDialog::getText(this, "Переименовать",
        "Название закладки:", QLineEdit::Normal, title, &ok);
    if (ok && !t.trimmed().isEmpty())
        m_mgr->updateUserBookmark(id, t.trimmed());
}

void BookmarkWidget::onMoveUp()
{
    QModelIndex idx = m_tree->indexAt(QCursor::pos());
    if (!isUserBookmark(idx) || idx.row() == 0) return;
    m_mgr->moveUserBookmark(idx.data(Qt::UserRole).toString(), -1);
}

void BookmarkWidget::onMoveDown()
{
    QModelIndex idx = m_tree->indexAt(QCursor::pos());
    if (!isUserBookmark(idx)) return;
    m_mgr->moveUserBookmark(idx.data(Qt::UserRole).toString(), 1);
}
