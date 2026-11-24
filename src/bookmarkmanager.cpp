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
#include <windows.h>

// === Poppler C API typedefs ===
struct poppler_document_t;
struct poppler_outline_t;
struct poppler_outline_item_t;
struct poppler_dest_t;

typedef poppler_document_t *(*poppler_document_load_fn)(const char *filename);
typedef void (*poppler_document_free_fn)(poppler_document_t *doc);
typedef int (*poppler_document_pages_fn)(poppler_document_t *doc);
typedef poppler_outline_t *(*poppler_document_get_outline_fn)(poppler_document_t *doc);
typedef void (*poppler_outline_free_fn)(poppler_outline_t *outline);
typedef const char *(*poppler_outline_item_get_title_fn)(poppler_outline_item_t *item);
typedef int (*poppler_outline_item_is_open_fn)(poppler_outline_item_t *item);
typedef int (*poppler_outline_item_num_kids_fn)(poppler_outline_item_t *item);
typedef poppler_outline_item_t *(*poppler_outline_item_kid_fn)(poppler_outline_item_t *item, int n);
typedef poppler_dest_t *(*poppler_outline_item_get_dest_fn)(poppler_outline_item_t *item);
typedef void (*poppler_dest_free_fn)(poppler_dest_t *dest);
typedef int (*poppler_dest_is_ok_fn)(poppler_dest_t *dest);
typedef int (*poppler_dest_get_page_fn)(poppler_dest_t *dest);

namespace {
    // Структура для хранения указателей на функции C API
    struct PopplerCAPI {
        HMODULE lib = nullptr;
        poppler_document_load_fn load = nullptr;
        poppler_document_free_fn free_doc = nullptr;
        poppler_document_pages_fn pages = nullptr;
        poppler_document_get_outline_fn get_outline = nullptr;
        poppler_outline_free_fn free_outline = nullptr;
        poppler_outline_item_get_title_fn item_title = nullptr;
        poppler_outline_item_is_open_fn item_is_open = nullptr;
        poppler_outline_item_num_kids_fn item_num_kids = nullptr;
        poppler_outline_item_kid_fn item_kid = nullptr;
        poppler_outline_item_get_dest_fn item_get_dest = nullptr;
        poppler_dest_free_fn free_dest = nullptr;
        poppler_dest_is_ok_fn dest_is_ok = nullptr;
        poppler_dest_get_page_fn dest_page = nullptr;
    };

    PopplerCAPI &api()
    {
        static PopplerCAPI s_api;
        static bool s_initialized = false;

        if (!s_initialized) {
            s_api.lib = LoadLibraryW(L"poppler.dll");
            if (s_api.lib) {
                #define LOAD_SYM(name) s_api.name = reinterpret_cast<decltype(s_api.name)>(GetProcAddress(s_api.lib, #name))
                LOAD_SYM(load);
                LOAD_SYM(free_doc);
                LOAD_SYM(pages);
                LOAD_SYM(get_outline);
                LOAD_SYM(free_outline);
                LOAD_SYM(item_title);
                LOAD_SYM(item_is_open);
                LOAD_SYM(item_num_kids);
                LOAD_SYM(item_kid);
                LOAD_SYM(item_get_dest);
                LOAD_SYM(free_dest);
                LOAD_SYM(dest_is_ok);
                LOAD_SYM(dest_page);
                #undef LOAD_SYM
            }
            s_initialized = true;
        }
        return s_api;
    }

    // Рекурсивно обходим outline и заполняем список закладок с номерами страниц
    void walkOutline(poppler_outline_item_t *item, int depth, QList<Bookmark> &out)
    {
        auto &a = api();
        if (!item || !a.item_title) return;

        Bookmark bm;
        bm.pageNumber = 0;
        bm.zoom = 1.0;
        bm.open = true;

        // Получаем название
        const char *titleC = a.item_title(item);
        if (titleC) {
            QString title = QString::fromUtf8(titleC);
            // Удаляем лишние пробелы
            title = title.trimmed();
            // Добавляем отступы для вложенности
            for (int i = 0; i < depth; ++i)
                title = "\u2003" + title;
            bm.title = title;
        }

        // Получаем destination (номер страницы)
        if (a.item_get_dest && a.dest_is_ok && a.dest_page) {
            poppler_dest_t *dest = a.item_get_dest(item);
            if (dest && a.dest_is_ok(dest)) {
                int page = a.dest_page(dest);
                if (page > 0) {
                    bm.pageNumber = page - 1; // Poppler uses 1-based indexing
                }
            }
            if (a.free_dest)
                a.free_dest(dest);
        }

        out.append(bm);

        // Рекурсивно обходим детей
        if (a.item_num_kids && a.item_kid) {
            int numKids = a.item_num_kids(item);
            for (int i = 0; i < numKids; ++i) {
                poppler_outline_item_t *kid = a.item_kid(item, i);
                walkOutline(kid, depth + 1, out);
            }
        }
    }
}

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

    // Используем Poppler C API для получения outline с номерами страниц
    auto &a = api();
    if (a.lib && a.get_outline && a.free_outline) {
        // Создаём poppler::document для C API из C++ document
        // Poppler C++ document содержит указатель на внутренний C document
        // Мы используем reinterpret_cast для получения внутреннего указателя
        // Это безопасно, так как poppler.dll и poppler-cpp.dll из одного билда
        poppler_document_t *cDoc = reinterpret_cast<poppler_document_t *>(doc);
        poppler_outline_t *outline = a.get_outline(cDoc);
        if (outline) {
            // Обходим outline с номерами страниц
            walkOutline(nullptr, 0, m_pdf); // TODO: получить корневой item

            a.free_outline(outline);
        }
    }

    // Если C API не сработал, пробуем fallback через C++ API (без destination)
    if (m_pdf.isEmpty()) {
        auto *toc = doc->create_toc();
        if (toc) {
            walkTOC(toc->root(), 0, doc);
            delete toc;
        }
    }

    // Загружаем пользовательские закладки из JSON
    loadUserBookmarks();

    rebuildAll();
}

namespace {
    QString ustringToString(poppler::ustring const &us)
    {
        auto utf8 = us.to_utf8();
        return QString::fromUtf8(reinterpret_cast<char const *>(utf8.data()), utf8.size());
    }
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

void BookmarkManager::walkTOC(poppler::toc_item const *item, int depth, poppler::document const *)
{
    if (!item) return;

    Bookmark entry;
    entry.title = ustringToString(item->title()).trimmed();

    for (int i = 0; i < depth; ++i) {
        entry.title = "\u2003" + entry.title;
    }

    entry.pageNumber = 0;
    entry.zoom = 1.0;

    m_pdf.append(entry);

    auto children = item->children();
    for (const auto *child : children) {
        walkTOC(child, depth + 1, nullptr);
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
