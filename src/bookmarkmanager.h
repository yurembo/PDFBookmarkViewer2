#ifndef BOOKMARKMANAGER_H
#define BOOKMARKMANAGER_H

#include <QObject>
#include <QString>
#include <QColor>
#include <QList>
#include <QUuid>
#include <QSharedPointer>

class PDFRenderer;

inline QString makeUUID() {
    return QUuid::createUuid().toString().remove('{').remove('}');
}

struct Bookmark {
    QString id;
    QString title;
    int pageNumber = 0;
    double zoom = 1.0;
    QColor color;
    bool open = true;

    Bookmark()
        : id(makeUUID())
        , color(0, 80, 180) {}

    Bookmark(const QString &t, int p, double z = 1.0)
        : id(makeUUID())
        , title(t)
        , pageNumber(p)
        , zoom(z)
        , color(0, 80, 180) {}
};

class BookmarkManager : public QObject
{
    Q_OBJECT
public:
    explicit BookmarkManager(QObject *parent = nullptr);

    void setRenderer(PDFRenderer *r) { m_r = r; }

    // Загрузить пользовательские закладки из JSON
    void loadUserBookmarks();

    // Сохранить пользовательские закладки в JSON-файл рядом с PDF
    bool saveUserBookmarks();

    // Объединённый список для отображения
    const QList<Bookmark>& allBookmarks() const { return m_all; }

    // Пользовательские закладки
    const QList<Bookmark>& userBookmarks() const { return m_user; }

    void addUserBookmark(const QString &title, int page, double zoom = 1.0);
    void removeUserBookmark(const QString &id);
    void updateUserBookmark(const QString &id, const QString &title);
    void moveUserBookmark(const QString &id, int delta);

    // Путь к JSON-файлу (рядом с PDF)
    QString jsonFilePath() const;

signals:
    void bookmarksChanged();
    void bookmarkAdded(const Bookmark &bm);
    void bookmarkRemoved(const QString &id);
    void bookmarkUpdated(const QString &id, const QString &title);

private:
    void rebuildAll();

    PDFRenderer *m_r = nullptr;
    QList<Bookmark> m_user;
    QList<Bookmark> m_all;
};

#endif
