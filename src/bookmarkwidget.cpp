#include "bookmarkwidget.h"
#include "bookmarkmanager.h"
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QFont>
#include <QContextMenuEvent>
#include <QItemSelectionModel>
#include <QScrollBar>

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
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tree->setStyleSheet(
        "QTreeView { background-color: #1e1e1e; border: none; color: #d4d4d4; font-size: 12px; }"
        "QTreeView::item { padding: 4px 8px; }"
        "QTreeView::item:selected { background-color: #094771; color: white; }"
        "QTreeView::item:hover:!selected { background-color: #2a2d2e; }");

    lay->addWidget(m_tree);

    connect(m_tree, &QTreeView::clicked, this, &BookmarkWidget::onClicked);
    connect(m_tree, &QTreeView::customContextMenuRequested, this, &BookmarkWidget::showContextMenu);
    connect(m_tree, &QTreeView::pressed, this, &BookmarkWidget::onTreePressed);
    connect(m_mgr, &BookmarkManager::bookmarkAdded, this, [this](const Bookmark &bm) { onNewBookmarkAdded(bm.id); });
    connect(m_mgr, &BookmarkManager::bookmarksChanged, this, [this]() { refreshView(); });
}

void BookmarkWidget::onClicked(const QModelIndex &idx)
{
    if (!idx.isValid() || isSeparator(idx)) return;

    QString id = idx.data(Qt::UserRole).toString();
    if (id.isEmpty()) return;

    const auto &all = m_mgr->allBookmarks();
    for (const auto &bm : all) {
        if (bm.id == id) {
            emit bookmarkActivated(bm.pageNumber, bm.zoom);
            return;
        }
    }
}

void BookmarkWidget::onTreePressed(const QModelIndex &idx)
{
    if (!idx.isValid()) return;
    m_selectedId = idx.data(Qt::UserRole).toString();
    m_scrollPos = m_tree->verticalScrollBar()->value();
}

void BookmarkWidget::onNewBookmarkAdded(const QString &id)
{
    m_selectedId = id;
    m_scrollToBottom = true;
}

void BookmarkWidget::refreshView()
{
    m_model.clear();
    m_model.setColumnCount(1);

    const auto &all = m_mgr->allBookmarks();
    const auto &pdf = m_mgr->pdfBookmarks();

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
            item->setData(0, Qt::UserRole + 1); // 0 = separator
        } else {
            // Проверяем, есть ли эта закладка в PDF outline
            bool isPdf = false;
            for (const auto &pb : pdf) {
                if (pb.id == bm.id) { isPdf = true; break; }
            }

            if (isPdf) {
                // Закладка из PDF outline — жирный шрифт
                QString t = QString("%1  (стр. %2)").arg(bm.title).arg(bm.pageNumber + 1);
                item->setText(t);
                item->setData(1, Qt::UserRole + 1); // 1 = PDF bookmark
                QFont font;
                font.setBold(true);
                item->setFont(font);
            } else {
                // Пользовательская закладка
                QString t = QString("%1  (стр. %2)").arg(bm.title).arg(bm.pageNumber + 1);
                item->setText(t);
                item->setData(2, Qt::UserRole + 1); // 2 = user bookmark
            }
        }

        m_model.appendRow(item);
    }

    // Восстанавливаем выбор
    if (!m_selectedId.isEmpty()) {
        int row = 0;
        for (const auto &bm : all) {
            if (bm.id == m_selectedId) {
                QModelIndex idx = m_model.index(row, 0);
                m_tree->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
                m_tree->scrollTo(idx);
                break;
            }
            row++;
        }
    }

    // Восстанавливаем скролл или прокручиваем вниз при добавлении новой закладки
    if (m_scrollToBottom) {
        m_tree->verticalScrollBar()->setValue(m_tree->verticalScrollBar()->maximum());
        m_scrollToBottom = false;
    } else if (!m_selectedId.isEmpty()) {
        m_tree->verticalScrollBar()->setValue(m_scrollPos);
    }

    // Сбрасываем, чтобы не срабатывало повторно
    m_selectedId.clear();
}

bool BookmarkWidget::isSeparator(const QModelIndex &idx) const
{
    if (!idx.isValid()) return true;
    int type = idx.data(Qt::UserRole + 1).toInt();
    return type == 0; // 0 = separator
}

bool BookmarkWidget::isUserBookmark(const QModelIndex &idx) const
{
    if (!idx.isValid()) return false;
    int type = idx.data(Qt::UserRole + 1).toInt();
    return type == 2; // 2 = user bookmark
}

void BookmarkWidget::showContextMenu(const QPoint &pos)
{
    QModelIndex idx = m_tree->indexAt(pos);
    if (!idx.isValid()) return;

    // Сохраняем id и позицию скролла для восстановления после refreshView
    m_selectedId = idx.data(Qt::UserRole).toString();
    m_scrollPos = m_tree->verticalScrollBar()->value();

    // Только для пользовательских закладок — показываем меню
    if (!isUserBookmark(idx)) {
        return;
    }

    QMenu menu(this);

    QAction *renameAct = menu.addAction("Переименовать");
    QAction *moveUpAct = menu.addAction("Переместить вверх");
    QAction *moveDownAct = menu.addAction("Переместить вниз");
    QAction *removeAct = menu.addAction("Удалить");

    moveUpAct->setEnabled(idx.row() > 0);

    connect(renameAct, &QAction::triggered, this, [this, idx]() { onRename(idx); });
    connect(moveUpAct, &QAction::triggered, this, [this, idx]() { onMoveUp(idx); });
    connect(moveDownAct, &QAction::triggered, this, [this, idx]() { onMoveDown(idx); });
    connect(removeAct, &QAction::triggered, this, [this, idx]() { onRemove(idx); });

    menu.exec(m_tree->viewport()->mapToGlobal(pos));
}

void BookmarkWidget::onRemove(const QModelIndex &idx)
{
    QString id = idx.data(Qt::UserRole).toString();
    if (!id.isEmpty()) m_mgr->removeUserBookmark(id);
}

void BookmarkWidget::onRename(const QModelIndex &idx)
{
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

void BookmarkWidget::onMoveUp(const QModelIndex &idx)
{
    if (idx.row() == 0) return;
    m_mgr->moveUserBookmark(idx.data(Qt::UserRole).toString(), -1);
}

void BookmarkWidget::onMoveDown(const QModelIndex &idx)
{
    m_mgr->moveUserBookmark(idx.data(Qt::UserRole).toString(), 1);
}
