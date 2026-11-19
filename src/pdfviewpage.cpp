#include "pdfviewpage.h"
#include "pdfrenderer.h"
#include <QScrollBar>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPainter>

PDFViewPage::PDFViewPage(PDFRenderer *r, QWidget *p)
    : QAbstractScrollArea(p)
    , m_r(r)
    , m_page(-1)
    , m_zoom(1.0)
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setMinimumSize(100, 100);
    setStyleSheet("PDFViewPage { background-color: #3b3b3b; }");
}

void PDFViewPage::setPage(int p)
{
    if (p < 0 || !m_r || p == m_page) return;
    m_page = p;
    render();
    updateScrollBars();
    update();
}

void PDFViewPage::setZoom(double z)
{
    if (z <= 0) return;
    m_zoom = z;
    render();
    updateScrollBars();
    update();
    emit zoomChanged(m_zoom);
}

void PDFViewPage::zoomFit()
{
    if (!m_img.isNull()) {
        double scale = static_cast<double>(viewport()->height()) / m_img.height();
        setZoom(scale);
    }
}

void PDFViewPage::zoomWidth()
{
    if (!m_img.isNull()) {
        double scale = static_cast<double>(viewport()->width()) / m_img.width();
        setZoom(scale);
    }
}

QSizeF PDFViewPage::pageSize() const
{
    if (!m_r || m_page < 0) return {};
    return m_r->pageSize(m_page);
}

void PDFViewPage::render()
{
    if (!m_r || m_page < 0) {
        m_img = QImage();
        return;
    }
    // Рендерим в реальном размере страницы
    QSizeF pt = m_r->pageSize(m_page);
    int w = qRound(pt.width() * m_zoom);
    int h = qRound(pt.height() * m_zoom);
    if (w <= 0 || h <= 0) return;
    m_img = m_r->renderPage(m_page, w, h, 1.0);
}

void PDFViewPage::updateScrollBars()
{
    if (m_img.isNull()) return;

    int vw = viewport()->width();
    int vh = viewport()->height();

    setHorizontalScrollBarPolicy(m_img.width() > vw ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(m_img.height() > vh ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);

    hbar()->setRange(0, qMax(0, m_img.width() - vw));
    vbar()->setRange(0, qMax(0, m_img.height() - vh));
}

void PDFViewPage::paintEvent(QPaintEvent *)
{
    QPainter painter(viewport());
    painter.fillRect(viewport()->rect(), QColor(59, 59, 59));
    if (m_img.isNull()) {
        painter.setPen(QColor(150,150,150));
        painter.drawText(viewport()->rect(), Qt::AlignCenter,
            m_page >= 0 ? QString("Страница %1").arg(m_page + 1) : "Откройте PDF-файл");
        return;
    }
    painter.drawImage(0, 0, m_img);
}

void PDFViewPage::resizeEvent(QResizeEvent *)
{
    viewport()->update();
}

void PDFViewPage::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::MiddleButton ||
        (ev->button() == Qt::LeftButton && ev->modifiers() == Qt::AltModifier)) {
        m_panning = true;
        m_drag = ev->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void PDFViewPage::mouseMoveEvent(QMouseEvent *ev)
{
    if (!m_panning) return;
    QPoint d = m_drag - ev->pos();
    m_drag = ev->pos();
    hbar()->setValue(hbar()->value() + d.x());
    vbar()->setValue(vbar()->value() + d.y());
}

void PDFViewPage::mouseReleaseEvent(QMouseEvent *)
{
    if (m_panning) { m_panning = false; setCursor(Qt::ArrowCursor); }
}

void PDFViewPage::wheelEvent(QWheelEvent *ev)
{
    if (ev->modifiers() == Qt::ControlModifier) {
        if (ev->angleDelta().y() > 0) zoomIn();
        else if (ev->angleDelta().y() < 0) zoomOut();
    } else {
        hbar()->setValue(hbar()->value() - ev->angleDelta().x());
        vbar()->setValue(vbar()->value() - ev->angleDelta().y());
    }
}

void PDFViewPage::contextMenuEvent(QContextMenuEvent *ev)
{
    QMenu menu(this);

    QAction *fitH = menu.addAction("По высоте");
    QAction *fitW = menu.addAction("По ширине");
    QAction *act1 = menu.addAction("100%");
    QAction *plus = menu.addAction("+");
    QAction *minus = menu.addAction("\u2212");

    connect(fitH, &QAction::triggered, this, [this]{ zoomFit(); });
    connect(fitW, &QAction::triggered, this, [this]{ zoomWidth(); });
    connect(act1, &QAction::triggered, this, [this]{ setZoom(1.0); });
    connect(plus, &QAction::triggered, this, &PDFViewPage::zoomIn);
    connect(minus, &QAction::triggered, this, &PDFViewPage::zoomOut);

    menu.exec(ev->globalPos());
}
