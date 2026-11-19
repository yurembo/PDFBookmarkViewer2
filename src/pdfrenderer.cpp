#include "pdfrenderer.h"
#include <QImage>
#include <QDebug>
#include <QXmlStreamReader>
#include <memory>

PDFRenderer::PDFRenderer(QObject *parent) : QObject(parent) {}
PDFRenderer::~PDFRenderer() = default;

bool PDFRenderer::loadDocument(const QString &filePath)
{
    closeDocument();
    m_doc = Poppler::Document::load(filePath);
    if (!m_doc || m_doc->isLocked()) {
        if (m_doc && m_doc->isLocked())
            qWarning() << "Password-protected PDF";
        else
            qWarning() << "Failed to load PDF:" << filePath;
        m_doc.reset();
        return false;
    }
    m_filePath = filePath;
    emit documentLoaded();
    return true;
}

void PDFRenderer::closeDocument()
{
    m_doc.reset();
    m_filePath.clear();
    emit documentClosed();
}

bool PDFRenderer::isOpen() const { return m_doc && !m_doc->isLocked(); }
int PDFRenderer::pageCount() const { return m_doc ? m_doc->numPages() : 0; }

QString PDFRenderer::documentTitle() const
{
    if (!m_doc) return {};
    QString xml = m_doc->metadata();
    if (xml.isEmpty()) return {};
    QXmlStreamReader xr(xml);
    while (!xr.atEnd()) {
        xr.readNext();
        if (xr.tokenType() == QXmlStreamReader::StartElement &&
            xr.name() == "title" &&
            xr.namespaceUri() == "http://purl.org/dc/elements/1.1/") {
            return xr.readElementText().trimmed();
        }
    }
    return {};
}

QString PDFRenderer::documentAuthor() const
{
    if (!m_doc) return {};
    QString xml = m_doc->metadata();
    if (xml.isEmpty()) return {};
    QXmlStreamReader xr(xml);
    while (!xr.atEnd()) {
        xr.readNext();
        if (xr.tokenType() == QXmlStreamReader::StartElement &&
            xr.name() == "creator" &&
            xr.namespaceUri() == "http://purl.org/dc/elements/1.1/") {
            return xr.readElementText().trimmed();
        }
    }
    return {};
}

QString PDFRenderer::fileName() const { return m_filePath; }

QImage PDFRenderer::renderPage(int pageNum, int widthPx, int, double) const
{
    if (!m_doc || pageNum < 0 || pageNum >= m_doc->numPages())
        return {};
    auto page = m_doc->page(pageNum);
    if (!page) return {};

    QSizeF sz = page->pageSizeF();
    double dpi = 150;

    return page->renderToImage(dpi, dpi);
}

QImage PDFRenderer::renderPageAsImage(int pageNum, double dpi) const
{
    if (!m_doc || pageNum < 0 || pageNum >= m_doc->numPages())
        return {};
    return m_doc->page(pageNum)->renderToImage(dpi, dpi);
}

QSizeF PDFRenderer::pageSize(int pageNum) const
{
    if (!m_doc || pageNum < 0 || pageNum >= m_doc->numPages())
        return {};
    return m_doc->page(pageNum)->pageSizeF();
}

Poppler::Document *PDFRenderer::native() const
{
    return m_doc.get();
}
