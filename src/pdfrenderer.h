#ifndef PDFRENDERER_H
#define PDFRENDERER_H

#include <QObject>
#include <QString>
#include <QSizeF>
#include <QPdfDocument>
#include <memory>

struct PopplerDocument;

class PDFRenderer : public QObject
{
    Q_OBJECT
public:
    explicit PDFRenderer(QObject *parent = nullptr);
    ~PDFRenderer();

    bool loadDocument(const QString &filePath);
    void closeDocument();

    bool isOpen() const;
    bool isLoading() const;
    bool isReady() const;
    int pageCount() const;

    QString documentTitle() const;
    QString documentAuthor() const;
    QString fileName() const;

    QSizeF pageSize(int pageNum) const;

    // Возвращает необработанный указатель на Poppler::Document
    PopplerDocument *popplerDoc() const { return m_popplerDoc; }

    QPdfDocument *native() const { return m_doc.get(); }

signals:
    void documentLoaded();
    void documentClosed();
    void documentStatusChanged(QPdfDocument::Status status);

private:
    void onStatusChanged(QPdfDocument::Status status);

    std::unique_ptr<QPdfDocument> m_doc;
    PopplerDocument *m_popplerDoc = nullptr;
    QString m_filePath;
    QPdfDocument::Status m_status = QPdfDocument::Status::Null;
};

#endif
