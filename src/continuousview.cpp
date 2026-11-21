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

    connect(m_pdfView->pageNavigator(), &QPdfPageNavigator::currentPageChanged,
            this, &ContinuousView::onCurrentPageChanged);
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
    return m_pdfView->pageNavigator()->currentPage();
}

void ContinuousView::setCurrentPage(int p)
{
    m_pdfView->pageNavigator()->jump(p, QPointF(0, 0));
}

double ContinuousView::zoom() const
{
    return m_pdfView->zoomFactor();
}

void ContinuousView::setZoom(double z)
{
    if (z <= 0.1 || z > 5.0) return;
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
    m_pdfView->setZoomMode(QPdfView::ZoomMode::FitToWidth);
    emit zoomChanged(m_pdfView->zoomFactor());
}
