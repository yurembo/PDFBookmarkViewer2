#include "bookmarkmanager.h"
#include "pdfrenderer.h"

#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-toc.h>
#include <poppler/cpp/poppler-destination.h>
#include <poppler/cpp/poppler-page.h>

BookmarkManager::BookmarkManager(QObject *parent)
    : QObject(parent)
{}

void BookmarkManager::readBookmarksFromPDF()
{
    m_pdf.clear();
    m_user.clear();
    m_all.clear();

    if (!m_r || !m_r->isOpen()) {
        loadUserBookmarks();
        return;
    }

    auto *doc = m_r->popplerDoc();
    if (!doc) {
        loadUserBookmarks();
        return;
    }

    // Загружаем TOC (table of contents / outline) из PDF
    auto *toc = doc->create_toc();
    if (toc) {
        walkTOC(toc->root(), 0, doc);
        delete toc;
    }

    // Загружаем пользовательские закладки из JSON
    loadUserBookmarks();

    rebuildAll();
}

void BookmarkManager::loadUserBookmarks()
{
    m_user.clear();
    m_all.clear();

    QString path = jsonFilePath();
    if (path.isEmpty()) {
        rebuildAll();
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "JSON not found:" << path;
        rebuildAll();
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << err.error;
        rebuildAll();
        return;
    }
    if (!doc.isArray()) {
        rebuildAll();
        return;
    }

    for (const auto &v : doc.array()) {
        if (!v.isObject()) continue;
        auto o = v.toObject();
        Bookmark bm;
        bm.id = o["id"].toString(makeUUID());
        bm.title = o["title"].toString();
        bm.pageNumber = o["page"].toInt();
        bm.zoom = o["zoom"].toDouble(1.0);
        bm.open = o["open"].toBool(true);
        bm.color = QColor(o["color"].toString("#0050b4"));
        m_user.append(bm);
    }

    qDebug() << "Loaded" << m_user.size() << "user bookmarks from" << path;
    rebuildAll();
}

bool BookmarkManager::saveUserBookmarks()
{
    QString path = jsonFilePath();
    if (path.isEmpty()) {
        qWarning() << "No PDF file open, cannot save JSON";
        return false;
    }

    QJsonArray arr;
    for (const auto &bm : m_user) {
        QJsonObject o;
        o["id"] = bm.id;
        o["title"] = bm.title;
        o["page"] = bm.pageNumber;
        o["zoom"] = bm.zoom;
        o["color"] = QString("#%1%2%3")
            .arg(bm.color.red(), 2, 16, QChar('0'))
            .arg(bm.color.green(), 2, 16, QChar('0'))
            .arg(bm.color.blue(), 2, 16, QChar('0'));
        o["open"] = bm.open;
        arr.append(o);
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "Cannot write JSON:" << path;
        return false;
    }

    file.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
    file.close();

    qDebug() << "Saved" << m_user.size() << "user bookmarks to" << path;
    return true;
}

void BookmarkManager::addUserBookmark(const QString &title, int page, double zoom)
{
    if (!m_r || page < 0 || page >= m_r->pageCount()) return;
    Bookmark bm(title, page, zoom);
    m_user.append(bm);
    emit bookmarkAdded(bm);
    rebuildAll();
    saveUserBookmarks();
}

void BookmarkManager::removeUserBookmark(const QString &id)
{
    for (int i = m_user.size() - 1; i >= 0; --i) {
        if (m_user[i].id == id) {
            emit bookmarkRemoved(m_user[i].id);
            m_user.removeAt(i);
            rebuildAll();
            saveUserBookmarks();
            return;
        }
    }
}

void BookmarkManager::updateUserBookmark(const QString &id, const QString &title)
{
    for (auto &bm : m_user) {
        if (bm.id == id) {
            bm.title = title;
            emit bookmarkUpdated(id, title);
            rebuildAll();
            saveUserBookmarks();
            return;
        }
    }
}

void BookmarkManager::moveUserBookmark(const QString &id, int delta)
{
    for (int i = 0; i < (int)m_user.size(); ++i) {
        if (m_user[i].id == id) {
            int ni = i + delta;
            if (ni >= 0 && ni < (int)m_user.size()) {
                qSwap(m_user[i], m_user[ni]);
                rebuildAll();
                saveUserBookmarks();
            }
            return;
        }
    }
}

QString BookmarkManager::jsonFilePath() const
{
    if (!m_r || !m_r->isOpen()) return {};
    QString pdfPath = m_r->fileName();
    QFileInfo fi(pdfPath);
    return fi.dir().filePath(fi.baseName() + "_bookmarks.json");
}

namespace {
    // Конвертирует poppler::ustring в QString
    QString ustringToString(poppler::ustring const &us)
    {
        auto utf8 = us.to_utf8();
        return QString::fromUtf8(reinterpret_cast<char const *>(utf8.data()), utf8.size());
    }
}

void BookmarkManager::walkTOC(poppler::toc_item const *item, int depth, poppler::document const *doc)
{
    if (!item) return;

    Bookmark entry;
    entry.title = ustringToString(item->title()).trimmed();

    // Устанавливаем отступ для отображения вложенности
    for (int i = 0; i < depth; ++i) {
        entry.title = "\u2003" + entry.title;
    }

    entry.pageNumber = 0;
    entry.zoom = 1.0;

    m_pdf.append(entry);

    // Рекурсивно обходим детей
    auto children = item->children();
    for (const auto *child : children) {
        walkTOC(child, depth + 1, doc);
    }
}

void BookmarkManager::rebuildAll()
{
    m_all = m_pdf;

    // Разделитель "Закладки"
    Bookmark separator;
    separator.title = "Закладки";
    separator.id = "---separator---";
    separator.pageNumber = -1;
    separator.zoom = 0.0;
    separator.color = QColor(100, 100, 100);
    m_all.append(separator);

    m_all.append(m_user);
    emit bookmarksChanged();
}
