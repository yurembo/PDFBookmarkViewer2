#include "pdfrenderer.h"
#include <QDir>
#include <QFileInfo>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-toc.h>
#include <poppler/cpp/poppler-destination.h>

PDFRenderer::PDFRenderer(QObject *parent)
    : QObject(parent)
{}

PDFRenderer::~PDFRenderer() = default;

bool PDFRenderer::loadDocument(const QString &filePath)
{
    closeDocument();

    // Конвертируем QString в std::string для Poppler (UTF-8)
    std::string filePathStr = filePath.toUtf8().constData();

    // Загружаем Poppler для чтения metadata
    auto popplerDoc = poppler::document::load_from_file(filePathStr);
    if (!popplerDoc || popplerDoc->is_locked()) {
        if (popplerDoc && popplerDoc->is_locked()) {
            qWarning() << "Password-protected PDF";
        } else {
            qWarning() << "Failed to load PDF:" << filePath;
        }
        return false;
    }
    m_popplerDoc.reset(popplerDoc);

    // Загружаем QPdfDocument для рендеринга (асинхронно)
    m_doc = std::make_unique<QPdfDocument>();
    connect(m_doc.get(), &QPdfDocument::statusChanged,
            this, &PDFRenderer::onStatusChanged);

    m_filePath = filePath;
    QPdfDocument::Error err = m_doc->load(filePath);
    if (err != QPdfDocument::Error::None) {
        qWarning() << "QPdfDocument load error:" << filePath << err;
        m_doc.reset();
        m_popplerDoc.reset();
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
    m_popplerDoc.reset();
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
    // Fallback: метаданные из Poppler CPP
    if (m_popplerDoc) {
        auto title = m_popplerDoc->get_title();
        if (!title.empty()) {
            auto utf8 = title.to_utf8();
            return QString::fromUtf8(reinterpret_cast<const char *>(utf8.data()), utf8.size());
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
        auto author = m_popplerDoc->get_author();
        if (!author.empty()) {
            auto utf8 = author.to_utf8();
            return QString::fromUtf8(reinterpret_cast<const char *>(utf8.data()), utf8.size());
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
