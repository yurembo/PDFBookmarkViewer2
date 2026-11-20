#include "thumbnailwidget.h"
#include "pdfrenderer.h"

ThumbnailWidget::ThumbnailWidget(PDFRenderer *r, QWidget *p)
    : QWidget(p)
    , m_r(r)
    , m_currentPage(-1)
{
    setMinimumWidth(140);
    setMaximumWidth(220);
    setStyleSheet("ThumbnailWidget { background-color: #2d2d30; }");
    updateThumbnails();
}

void ThumbnailWidget::setCurrentPage(int p)
{
    m_currentPage = p;
    update();
}

void ThumbnailWidget::updateThumbnails()
{
    m_thumbs.clear();
    if (!m_r || !m_r->isOpen()) {
        update();
        return;
    }

    QList<QImage> thumbs;
    int count = m_r->pageCount();
    for (int i = 0; i < count; ++i) {
        QImage thumb = m_r->renderPageAsImage(i, 96);
        int h = qRound(static_cast<double>(thumb.height()) * kWidth / thumb.width());
        thumbs.append(thumb.scaled(kWidth, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    m_thumbs = thumbs;
    update();
}

int ThumbnailWidget::pageAtY(int y) const
{
    int cy = kSpacing;
    for (int i = 0; i < m_thumbs.size(); ++i) {
        int h = m_thumbs[i].height();
        if (y >= cy && y <= cy + h) return i;
        cy += h + kSpacing;
    }
    return -1;
}

void ThumbnailWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor(45, 45, 48));

    int cy = kSpacing;
    for (int i = 0; i < m_thumbs.size(); ++i) {
        if (i == m_currentPage) {
            painter.setPen(QPen(QColor(0, 120, 215), 2));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(2, cy - 1, kWidth + 2, m_thumbs[i].height() + 2);
        }
        painter.drawImage(4, cy, m_thumbs[i]);
        cy += m_thumbs[i].height() + kSpacing;
    }
}

void ThumbnailWidget::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton) {
        int page = pageAtY(ev->pos().y());
        if (page >= 0) {
            emit pageRequested(page);
            setCurrentPage(page);
        }
    }
}

void ThumbnailWidget::resizeEvent(QResizeEvent *)
{
    update();
}
