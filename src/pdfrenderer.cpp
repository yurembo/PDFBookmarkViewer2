#include "pdfrenderer.h"
#include <QDir>
#include <QFileInfo>
#include <QXmlStreamReader>

PDFRenderer::PDFRenderer(QObject *parent)
    : QObject(parent)
{}

PDFRenderer::~PDFRenderer() = default;

bool PDFRenderer::loadDocument(const QString &filePath)
{
    closeDocument();

    // Загружаем Poppler для чтения outline/метаданных
    auto popplerDoc = Poppler::Document::load(filePath);
    if (!popplerDoc || popplerDoc->isLocked()) {
        if (popplerDoc && popplerDoc->isLocked()) {
            qWarning() << "Password-protected PDF";
        } else {
            qWarning() << "Failed to load PDF:" << filePath;
        }
        m_popplerDoc = nullptr;
        return false;
    }
    m_popplerDoc = popplerDoc.release();

    // Загружаем QPdfDocument для рендеринга (асинхронно)
    m_doc = std::make_unique<QPdfDocument>();
    connect(m_doc.get(), &QPdfDocument::statusChanged,
            this, &PDFRenderer::onStatusChanged);

    m_filePath = filePath;
    QPdfDocument::Error err = m_doc->load(filePath);
    if (err != QPdfDocument::Error::None) {
        qWarning() << "QPdfDocument load error:" << filePath << err;
        m_doc.reset();
        m_popplerDoc = nullptr;
        return false;
    }
    return true;
}

void PDFRenderer::closeDocument()
{
    if (m_doc) {
        disconnect(m_doc.get(), nullptr, this, nullptr);
        m_doc->close();
    }
    m_doc.reset();
    delete m_popplerDoc;
    m_popplerDoc = nullptr;
    m_filePath.clear();
    m_status = QPdfDocument::Status::Null;
    emit documentClosed();
}

bool PDFRenderer::isOpen() const { return m_doc && m_status == QPdfDocument::Status::Ready; }
bool PDFRenderer::isLoading() const { return m_status == QPdfDocument::Status::Loading; }
bool PDFRenderer::isReady() const { return m_status == QPdfDocument::Status::Ready; }

void PDFRenderer::onStatusChanged(QPdfDocument::Status status)
{
    m_status = status;
    if (status == QPdfDocument::Status::Ready) {
        emit documentLoaded();
    } else if (status == QPdfDocument::Status::Error ||
               status == QPdfDocument::Status::Null) {
        if (m_status == QPdfDocument::Status::Error) {
            qWarning() << "PDF document error";
        }
        emit documentClosed();
    }
}

int PDFRenderer::pageCount() const
{
    return m_doc && m_status == QPdfDocument::Status::Ready
        ? m_doc->pageCount() : 0;
}

QString PDFRenderer::documentTitle() const
{
    if (!m_doc || !isOpen()) return {};
    QVariant v = m_doc->metaData(QPdfDocument::MetaDataField::Title);
    if (!v.toString().isEmpty()) return v.toString();
    // Fallback: метаданные из Poppler (XML)
    if (m_popplerDoc) {
        QString xml = m_popplerDoc->metadata();
        if (!xml.isEmpty()) {
            QXmlStreamReader xr(xml);
            while (!xr.atEnd()) {
                xr.readNext();
                if (xr.tokenType() == QXmlStreamReader::StartElement &&
                    xr.name().toString() == "title" &&
                    xr.namespaceUri().toString() == "http://purl.org/dc/elements/1.1/") {
                    return xr.readElementText().trimmed();
                }
            }
        }
    }
    return {};
}

QString PDFRenderer::documentAuthor() const
{
    if (!m_doc || !isOpen()) return {};
    QVariant v = m_doc->metaData(QPdfDocument::MetaDataField::Creator);
    if (!v.toString().isEmpty()) return v.toString();
    if (m_popplerDoc) {
        QString xml = m_popplerDoc->metadata();
        if (!xml.isEmpty()) {
            QXmlStreamReader xr(xml);
            while (!xr.atEnd()) {
                xr.readNext();
                if (xr.tokenType() == QXmlStreamReader::StartElement &&
                    xr.name().toString() == "creator" &&
                    xr.namespaceUri().toString() == "http://purl.org/dc/elements/1.1/") {
                    return xr.readElementText().trimmed();
                }
            }
        }
    }
    return {};
}

QString PDFRenderer::fileName() const { return m_filePath; }

QSizeF PDFRenderer::pageSize(int pageNum) const
{
    if (!m_doc || !isOpen() || pageNum < 0) return {};
    return m_doc->pagePointSize(pageNum);
}
