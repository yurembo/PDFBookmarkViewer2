#include "continuousview.h"
#include <QPdfView>
#include <QPdfDocument>
#include <QPdfPageNavigator>
#include <QVBoxLayout>
#include <QPointF>

ContinuousView::ContinuousView(QWidget *p)
    : QWidget(p)
{
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    m_pdfView = new QPdfView(this);
    m_pdfView->setPageMode(QPdfView::PageMode::MultiPage);
    m_pdfView->setZoomMode(QPdfView::ZoomMode::Custom);
    m_pdfView->setPageSpacing(8);

    lay->addWidget(m_pdfView, 1);

    // Подключаемся к сигналу currentPageChanged navigator'а
    auto *nav = m_pdfView->pageNavigator();
    if (nav) {
        connect(nav, &QPdfPageNavigator::currentPageChanged,
                this, &ContinuousView::onCurrentPageChanged);
    }
}

ContinuousView::~ContinuousView() = default;

void ContinuousView::setDocument(QPdfDocument *doc)
{
    m_doc = doc;
    m_pdfView->setDocument(doc);
}

void ContinuousView::onCurrentPageChanged(int p)
{
    emit currentPageChanged(p);
    emit visiblePageChanged(p);
}

int ContinuousView::currentPage() const
{
    if (!m_pdfView || !m_pdfView->pageNavigator())
        return -1;
    return m_pdfView->pageNavigator()->currentPage();
}

void ContinuousView::setCurrentPage(int p)
{
    if (!m_pdfView || !m_pdfView->pageNavigator())
        return;
    m_pdfView->pageNavigator()->jump(p, QPointF(0, 0));
}

double ContinuousView::zoom() const
{
    if (!m_pdfView)
        return 1.0;
    return m_pdfView->zoomFactor();
}

void ContinuousView::setZoom(double z)
{
    if (z <= 0.1 || z > 5.0) return;
    if (!m_pdfView) return;
    m_pdfView->setZoomMode(QPdfView::ZoomMode::Custom);
    m_pdfView->setZoomFactor(z);
    emit zoomChanged(z);
}

void ContinuousView::zoomIn()
{
    setZoom(zoom() * 1.25);
}

void ContinuousView::zoomOut()
{
    setZoom(zoom() / 1.25);
}

void ContinuousView::zoomFit()
{
    if (!m_pdfView) return;
    m_pdfView->setZoomMode(QPdfView::ZoomMode::FitToWidth);
    emit zoomChanged(m_pdfView->zoomFactor());
}
