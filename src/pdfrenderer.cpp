#include "pdfrenderer.h"
#include <QDir>
#include <QFileInfo>

PDFRenderer::PDFRenderer(QObject *parent)
    : QObject(parent)
{}

PDFRenderer::~PDFRenderer() = default;

bool PDFRenderer::loadDocument(const QString &filePath)
{
    closeDocument();
    m_doc = std::make_unique<QPdfDocument>();

    // Подключаемся к сигналам статуса
    connect(m_doc.get(), &QPdfDocument::statusChanged,
            this, &PDFRenderer::onStatusChanged);

    m_filePath = filePath;
    QPdfDocument::Error err = m_doc->load(filePath);
    if (err != QPdfDocument::Error::None) {
        qWarning() << "Failed to load PDF:" << filePath << err;
        m_doc.reset();
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
    return v.toString();
}

QString PDFRenderer::documentAuthor() const
{
    if (!m_doc || !isOpen()) return {};
    QVariant v = m_doc->metaData(QPdfDocument::MetaDataField::Creator);
    return v.toString();
}

QString PDFRenderer::fileName() const { return m_filePath; }

QSizeF PDFRenderer::pageSize(int pageNum) const
{
    if (!m_doc || !isOpen() || pageNum < 0) return {};
    return m_doc->pagePointSize(pageNum);
}
