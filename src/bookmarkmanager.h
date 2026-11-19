#ifndef BOOKMARKMANAGER_H
#define BOOKMARKMANAGER_H

#include <QObject>
#include <QString>
#include <QColor>
#include <QList>
#include <QUuid>
#include <QSharedPointer>
#include <poppler-qt6.h>
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
    void readBookmarksFromPDF();
    bool saveBookmarksToPDF() const;

    const QList<Bookmark>& bookmarks() const { return m_bm; }
    void addBookmark(const QString &title, int page, double zoom = 1.0);
    void removeBookmark(const QString &id);
    void updateBookmark(const QString &id, const QString &title);
    void moveBookmark(const QString &id, int delta);
    Bookmark* bookmarkAt(int i);
    const Bookmark* bookmarkAt(int i) const;
    int indexOf(const QString &id) const;

    QByteArray exportJSON() const;
    bool importJSON(const QByteArray &data);

signals:
    void bookmarksChanged();
    void bookmarkAdded(const Bookmark &bm);
    void bookmarkRemoved(const QString &id);
    void bookmarkUpdated(const QString &id, const QString &title);

private:
    void walkOutline(const QList<Poppler::OutlineItem> &items);

    PDFRenderer *m_r = nullptr;
    QList<Bookmark> m_bm;
};

#endif
