#ifndef BOOKMARKMANAGER_H
#define BOOKMARKMANAGER_H

#include <QObject>
#include <QString>
#include <QColor>
#include <QList>
#include <QUuid>
#include <QSharedPointer>
#include <poppler/qt6/poppler-qt6.h>
#include <poppler/cpp/poppler-document.h>

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

    // Загрузить outline из PDF
    void readBookmarksFromPDF();

    // Сохранить пользовательские закладки в JSON-файл рядом с PDF
    bool saveUserBookmarks();

    // Загрузить пользовательские закладки из JSON-файла рядом с PDF
    void loadUserBookmarks();

    // Объединённый список для отображения: PDF outline + разделитель + пользовательские
    const QList<Bookmark>& allBookmarks() const { return m_all; }

    // Доступ к PDF outline
    const QList<Bookmark>& pdfBookmarks() const { return m_pdf; }
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
    void walkOutline(const QList<Poppler::OutlineItem> &items);
    void rebuildAll();

    PDFRenderer *m_r = nullptr;
    QList<Bookmark> m_pdf;       // Закладки из PDF outline
    QList<Bookmark> m_user;      // Пользовательские закладки
    QList<Bookmark> m_all;       // Объединённый список (m_pdf + "Закладки" + m_user)
};

#endif
