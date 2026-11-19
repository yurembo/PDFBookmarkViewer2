#ifndef PDFRENDERER_H
#define PDFRENDERER_H

#include <QObject>
#include <QSize>
#include <QImage>
#include <QString>
#include <poppler-qt6.h>

class PDFRenderer : public QObject
{
    Q_OBJECT
public:
    explicit PDFRenderer(QObject *parent = nullptr);
    ~PDFRenderer();

    bool loadDocument(const QString &filePath);
    void closeDocument();

    bool isOpen() const;
    int pageCount() const;
    QString documentTitle() const;
    QString documentAuthor() const;
    QString fileName() const;

    QImage renderPage(int pageNum, int widthPx, int heightPx, double scale = 1.0) const;
    QImage renderPageAsImage(int pageNum, double dpi = 150) const;
    QSizeF pageSize(int pageNum) const;

    Poppler::Document *native() const;

signals:
    void documentLoaded();
    void documentClosed();

private:
    std::unique_ptr<Poppler::Document> m_doc;
    QString m_filePath;
};

#endif
