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
    
    // Используем стандартные scroll bars
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setMouseTracking(true);
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
    for (auto *label : std::as_const(m_labels)) {
        m_layout->removeWidget(label);
        label->deleteLater();
    }
    m_labels.clear();
    m_originalPixmaps.clear();
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
        
        // Всегда эмитим оба сигнала
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
    scaleAllPages();
    emit zoomChanged(m_zoom);
}

void ContinuousView::scaleAllPages()
{
    for (int i = 0; i < m_labels.size() && i < (int)m_originalPixmaps.size(); ++i) {
        const QPixmap *orig = m_originalPixmaps[i].data();
        if (!orig || orig->isNull()) continue;

        int targetW = qRound(orig->width() * m_zoom);
        int targetH = qRound(orig->height() * m_zoom);
        m_labels[i]->setPixmap(orig->scaled(targetW, targetH, Qt::KeepAspectRatio, Qt::SmoothTransformation));
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
    if (m_labels.isEmpty()) return;

    QPixmap pm0 = m_labels[0]->pixmap(Qt::ReturnByValue);
    double pageW = pm0.width();
    double pageH = pm0.height();
    int viewW = viewport()->width();
    int viewH = viewport()->height();

    double fitX = viewW / pageW;
    double fitY = viewH / pageH;
    double fitZoom = qMin(fitX, fitY);

    setZoom(qBound(0.1, fitZoom, 3.0));
}

QLabel *ContinuousView::addPage(int pageNum)
{
    QImage img = m_r->renderPageAsImage(pageNum, 150);
    if (img.isNull()) return nullptr;

    auto *pm = new QPixmap(QPixmap::fromImage(img));
    m_originalPixmaps.append(QSharedPointer<QPixmap>(pm));

    auto *label = new QLabel();
    label->setPixmap(pm->scaled(pm->width(), pm->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    label->setMouseTracking(true);
    label->setAttribute(Qt::WA_Hover);
    label->setProperty("pageNum", pageNum);

    m_labels.append(label);
    m_layout->addWidget(label);

    return label;
}

void ContinuousView::removePage(int pageNum)
{
    if (pageNum < 0 || pageNum >= (int)m_labels.size()) return;

    auto *label = m_labels.takeAt(pageNum);
    m_originalPixmaps.takeAt(pageNum);
    m_layout->removeWidget(label);
    label->deleteLater();

    if (m_current == pageNum) {
        m_current = qBound(0, m_current, (int)m_labels.size() - 1);
    } else if (m_current > pageNum) {
        --m_current;
    }
}

void ContinuousView::mousePressEvent(QMouseEvent *ev)
{
    // Обрабатываем только клики по страницам
    if (ev->button() == Qt::LeftButton) {
        auto *label = qobject_cast<QLabel*>(viewport()->childAt(ev->pos()));
        if (label) {
            int pageNum = label->property("pageNum").toInt();
            if (pageNum >= 0 && pageNum < (int)m_labels.size()) {
                setCurrentPage(pageNum);
                return;
            }
        }
        // Клик по пустому месту - игнорируем
    }
    QScrollArea::mousePressEvent(ev);
}

void ContinuousView::wheelEvent(QWheelEvent *ev)
{
    // Сохраняем текущую страницу
    int oldPage = m_current;
    
    // Вызываем стандартную прокрутку
    QScrollArea::wheelEvent(ev);
    
    // Определяем новую видимую страницу
    int newPage = visiblePage();
    
    // Если страница изменилась, эмитим сигнал
    if (newPage >= 0 && newPage != oldPage) {
        emit visiblePageChanged(newPage);
    }
}

void ContinuousView::keyPressEvent(QKeyEvent *ev)
{
    switch (ev->key()) {
    case Qt::Key_Down:
        // Плавная прокрутка на 20 пикселей вниз
        verticalScrollBar()->setValue(verticalScrollBar()->value() + 20);
        emit visiblePageChanged(m_current);
        break;
    case Qt::Key_Up:
        // Плавная прокрутка на 20 пикселей вверх
        verticalScrollBar()->setValue(verticalScrollBar()->value() - 20);
        emit visiblePageChanged(m_current);
        break;
    case Qt::Key_PageDown:
        // Переход на следующую страницу
        if (m_current < (int)m_labels.size() - 1) {
            setCurrentPage(m_current + 1);
        }
        break;
    case Qt::Key_PageUp:
        // Переход на предыдущую страницу
        if (m_current > 0) {
            setCurrentPage(m_current - 1);
        }
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
    // Игнорируем
    Q_UNUSED(ev);
}

void ContinuousView::mouseMoveEvent(QMouseEvent *ev)
{
    // Игнорируем
    Q_UNUSED(ev);
}
