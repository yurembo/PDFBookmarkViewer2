#include "bookmarkmanager.h"
#include "pdfrenderer.h"
#include "pdfeditor.h"

#include <QFile>
#include <QProcess>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>

BookmarkManager::BookmarkManager(QObject *parent) : QObject(parent) {}

void BookmarkManager::readBookmarksFromPDF()
{
    if (!m_r || !m_r->isOpen()) {
        m_bm.clear();
        emit bookmarksChanged();
        return;
    }
    m_bm.clear();

    Poppler::Document *doc = m_r->native();
    if (!doc) { emit bookmarksChanged(); return; }

    QList<Poppler::OutlineItem> outline = doc->outline();
    walkOutline(outline);
    emit bookmarksChanged();
}

bool BookmarkManager::saveBookmarksToPDF() const
{
    if (!m_r || !m_r->isOpen()) {
        QMessageBox::warning(nullptr, "Предупреждение", "Откройте PDF-файл");
        return false;
    }
    
    if (m_bm.isEmpty()) {
        QMessageBox::information(nullptr, "Информация",
            "Нет закладок для сохранения. Добавьте закладки и попробуйте снова.");
        return false;
    }
    
    QString filePath = m_r->fileName();
    if (filePath.isEmpty()) {
        QMessageBox::critical(nullptr, "Ошибка", "Неизвестен путь к файлу");
        return false;
    }
    
    QString tempPath = filePath + ".tmp.pdf";
    
    // Создаём JSON с параметрами
    QJsonObject params;
    params["input_path"] = filePath;
    params["output_path"] = tempPath;
    
    QJsonArray bookmarks;
    for (const auto &bm : m_bm) {
        QJsonObject bmObj;
        bmObj["title"] = bm.title;
        bmObj["page"] = bm.pageNumber + 1; // PDF страницы с 1
        bmObj["zoom"] = bm.zoom;
        bookmarks.append(bmObj);
    }
    params["bookmarks"] = bookmarks;
    
    // Путь к скрипту
    QString scriptPath = QDir::toNativeSeparators(
        QDir::currentPath() + "/scripts/save_bookmarks.py");
    if (!QFile::exists(scriptPath)) {
        scriptPath = QDir::toNativeSeparators(
            QCoreApplication::applicationDirPath() + "/scripts/save_bookmarks.py");
    }
    
    if (!QFile::exists(scriptPath)) {
        QMessageBox::critical(nullptr, "Ошибка",
            "Скрипт save_bookmarks.py не найден.\n"
            "Поместите его в папку scripts рядом с exe.");
        return false;
    }
    
    // Записываем JSON во временный файл с кодировкой UTF-8
    QString jsonPath = tempPath + ".json";
    QFile jsonFile(jsonPath);
    if (!jsonFile.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(nullptr, "Ошибка", "Не удалось создать временный файл");
        return false;
    }
    jsonFile.write(QJsonDocument(params).toJson(QJsonDocument::Compact));
    jsonFile.close();
    
    // Вызываем Python с путём к JSON файлу (экранируем пробелы)
    QProcess process;
    QString escapedJsonPath = "\"" + jsonPath.replace("\\", "\\\\") + "\"";
    process.start("python", {scriptPath, escapedJsonPath});
    process.waitForFinished(10000);
    
    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();
    
    if (process.exitCode() == 0 && output.contains("Success")) {
        // Заменяем оригинальный файл
        QFile::remove(filePath);
        QFile::rename(tempPath, filePath);
        qDebug() << "Bookmarks saved successfully!";
        return true;
    } else {
        QFile::remove(tempPath);
        QMessageBox::critical(nullptr, "Ошибка",
            "Не удалось сохранить закладки.\n"
            "Error: " + error);
        return false;
    }
}

void BookmarkManager::walkOutline(const QList<Poppler::OutlineItem> &items)
{
    for (const auto &item : items) {
        Bookmark entry;
        entry.title = item.name().trimmed();

        auto dest = item.destination();
        if (dest && dest->pageNumber() > 0) {
            int pg = dest->pageNumber() - 1;
            if (pg >= 0 && pg < m_r->pageCount())
                entry.pageNumber = pg;
        }

        m_bm.append(entry);

        if (item.hasChildren())
            walkOutline(item.children().toList());
    }
}

void BookmarkManager::addBookmark(const QString &title, int page, double zoom)
{
    if (!m_r || page < 0 || page >= m_r->pageCount()) return;
    Bookmark bm(title, page, zoom);
    m_bm.append(bm);
    emit bookmarkAdded(bm);
    emit bookmarksChanged();
}

void BookmarkManager::removeBookmark(const QString &id)
{
    for (int i = m_bm.size() - 1; i >= 0; --i) {
        if (m_bm[i].id == id) {
            emit bookmarkRemoved(m_bm[i].id);
            m_bm.removeAt(i);
            emit bookmarksChanged();
            return;
        }
    }
}

void BookmarkManager::updateBookmark(const QString &id, const QString &title)
{
    for (auto &bm : m_bm) {
        if (bm.id == id) {
            bm.title = title;
            emit bookmarkUpdated(id, title);
            emit bookmarksChanged();
            return;
        }
    }
}

void BookmarkManager::moveBookmark(const QString &id, int delta)
{
    for (int i = 0; i < (int)m_bm.size(); ++i) {
        if (m_bm[i].id == id) {
            int ni = i + delta;
            if (ni >= 0 && ni < (int)m_bm.size()) {
                qSwap(m_bm[i], m_bm[ni]);
                emit bookmarksChanged();
            }
            return;
        }
    }
}

Bookmark* BookmarkManager::bookmarkAt(int i)
{
    if (i < 0 || i >= (int)m_bm.size()) return nullptr;
    return &m_bm[i];
}

const Bookmark* BookmarkManager::bookmarkAt(int i) const
{
    if (i < 0 || i >= (int)m_bm.size()) return nullptr;
    return &m_bm[i];
}

int BookmarkManager::indexOf(const QString &id) const
{
    for (int i = 0; i < (int)m_bm.size(); ++i)
        if (m_bm[i].id == id) return i;
    return -1;
}

QByteArray BookmarkManager::exportJSON() const
{
    QJsonArray arr;
    for (const auto &bm : m_bm) {
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
    return QJsonDocument(arr).toJson(QJsonDocument::Compact);
}

bool BookmarkManager::importJSON(const QByteArray &data)
{
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) return false;
    if (!doc.isArray()) return false;

    m_bm.clear();
    for (const auto &v : doc.array()) {
        if (!v.isObject()) continue;
        auto o = v.toObject();
        Bookmark bm;
        bm.id = o["id"].toString();
        bm.title = o["title"].toString();
        bm.pageNumber = o["page"].toInt();
        bm.zoom = o["zoom"].toDouble(1.0);
        bm.open = o["open"].toBool(true);
        bm.color = QColor(o["color"].toString("#0050b4"));
        m_bm.append(bm);
    }
    emit bookmarksChanged();
    return true;
}
