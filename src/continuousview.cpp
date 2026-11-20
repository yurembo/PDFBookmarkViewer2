#include "continuousview.h"
#include "pdfrenderer.h"

#include <QMouseEvent>
#include <QScrollBar>
#include <QPoint>
#include <QTimer>

ContinuousView::ContinuousView(PDFRenderer *r, QWidget *p)
    : QScrollArea(p)
    , m_r(r)
{
    auto *w = new QWidget(this);
    m_layout = new QVBoxLayout(w);
    m_layout->setContentsMargins(10, 10, 10, 10);
    m_layout->setSpacing(10);
    m_layout->setAlignment(Qt::AlignCenter);

    setWidget(w);
    setWidgetResizable(true);
    setAlignment(Qt::AlignCenter);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setMouseTracking(true);

    // Debounce для зума
    m_zoomTimer.setSingleShot(true);
    m_zoomTimer.setInterval(200);
    connect(&m_zoomTimer, &QTimer::timeout, this, &ContinuousView::zoomApply);
}

void ContinuousView::renderAll()
{
    clear();

    if (!m_r || !m_r->isOpen()) return;

    int count = m_r->pageCount();
    for (int i = 0; i < count; ++i)
        addPage(i);
}

void ContinuousView::clear()
{
    for (auto *label : m_labels) {
        m_layout->removeWidget(label);
        label->deleteLater();
    }
    m_labels.clear();
    m_cachedImages.clear();
    m_current = -1;
}

void ContinuousView::setCurrentPage(int p)
{
    if (p < 0 || p >= (int)m_labels.size()) return;
    if (p == m_current) return;

    m_current = p;

    if (p >= 0 && p < (int)m_labels.size()) {
        auto *label = m_labels[p];
        QRect rect = label->geometry();
        int targetY = rect.top() - viewport()->height() / 2;
        targetY = qMax(0, targetY);
        verticalScrollBar()->setValue(targetY);

        emit visiblePageChanged(p);
    }

    emit currentPageChanged(m_current);
}

void ContinuousView::setZoom(double z)
{
    if (z <= 0.1 || z > 5.0) return;

    bool changed = !qFuzzyCompare(m_zoom, z);
    if (!changed) return;

    m_zoom = z;
    m_zoomTimer.start();
    emit zoomChanged(m_zoom);
}

void ContinuousView::zoomApply()
{
    if (!m_r || !m_r->isOpen() || m_labels.isEmpty()) return;

    for (int i = 0; i < (int)m_labels.size() && i < (int)m_cachedImages.size(); ++i) {
        const QImage &base = m_cachedImages.at(i);
        if (base.isNull()) continue;

        int targetW = qRound(base.width() * m_zoom);
        int targetH = qRound(base.height() * m_zoom);
        m_labels[i]->setPixmap(QPixmap::fromImage(base.scaled(targetW, targetH, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    }
}

void ContinuousView::zoomIn()
{
    setZoom(m_zoom * 1.25);
}

void ContinuousView::zoomOut()
{
    setZoom(m_zoom / 1.25);
}

void ContinuousView::zoomFit()
{
    if (!m_r || !m_r->isOpen() || m_r->pageCount() == 0) return;
    if (m_labels.isEmpty() || m_cachedImages.isEmpty()) return;

    // Используем base image (без учёта текущего zoom) для точного расчёта
    const QImage &base = m_cachedImages.first();
    if (base.isNull()) return;

    int viewW = viewport()->width();
    int viewH = viewport()->height();
    int pageW = base.width();
    int pageH = base.height();

    double fitX = static_cast<double>(viewW) / pageW;
    double fitY = static_cast<double>(viewH) / pageH;
    double fitZoom = qMin(fitX, fitY);

    setZoom(qBound(0.1, fitZoom, 3.0));
}

QLabel *ContinuousView::addPage(int pageNum)
{
    QImage img = m_r->renderPageAsImage(pageNum, 150);
    if (img.isNull()) return nullptr;

    m_cachedImages.append(img);

    auto *label = new QLabel();
    label->setPixmap(QPixmap::fromImage(img));
    label->setMouseTracking(true);
    label->setAttribute(Qt::WA_Hover);
    label->setProperty("pageNum", pageNum);

    m_labels.append(label);
    m_layout->addWidget(label);

    return label;
}

void ContinuousView::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton) {
        auto *label = qobject_cast<QLabel*>(viewport()->childAt(ev->pos()));
        if (label) {
            int pageNum = label->property("pageNum").toInt();
            if (pageNum >= 0 && pageNum < (int)m_labels.size()) {
                setCurrentPage(pageNum);
                emit bookmarkActivated(pageNum, m_zoom);
                return;
            }
        }
    }
    QScrollArea::mousePressEvent(ev);
}

void ContinuousView::wheelEvent(QWheelEvent *ev)
{
    int oldPage = m_current;

    QScrollArea::wheelEvent(ev);

    if (m_zoomTimer.isActive()) {
        m_zoomTimer.start();
    }

    int newPage = visiblePage();
    if (newPage >= 0 && newPage != oldPage) {
        emit visiblePageChanged(newPage);
    }
}

void ContinuousView::keyPressEvent(QKeyEvent *ev)
{
    switch (ev->key()) {
    case Qt::Key_Down:
        verticalScrollBar()->setValue(verticalScrollBar()->value() + 20);
        emit visiblePageChanged(m_current);
        break;
    case Qt::Key_Up:
        verticalScrollBar()->setValue(verticalScrollBar()->value() - 20);
        emit visiblePageChanged(m_current);
        break;
    case Qt::Key_PageDown:
        if (m_current < (int)m_labels.size() - 1) {
            setCurrentPage(m_current + 1);
            zoomApply();
        }
        break;
    case Qt::Key_PageUp:
        if (m_current > 0) {
            setCurrentPage(m_current - 1);
            zoomApply();
        }
        break;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        zoomIn();
        break;
    case Qt::Key_Minus:
        zoomOut();
        break;
    case Qt::Key_0:
        zoomFit();
        break;
    default:
        QScrollArea::keyPressEvent(ev);
    }
}

int ContinuousView::visiblePage() const
{
    if (m_labels.isEmpty()) return -1;

    QPoint center(viewport()->width() / 2, viewport()->height() / 2);
    auto *label = qobject_cast<QLabel*>(childAt(center));
    if (label) return label->property("pageNum").toInt();
    return -1;
}

void ContinuousView::mouseReleaseEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev);
}

void ContinuousView::mouseMoveEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev);
}
