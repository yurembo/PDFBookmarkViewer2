#ifndef PDFVIEWPAGE_H
#define PDFVIEWPAGE_H

#include <QWidget>
#include <QScrollArea>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QContextMenuEvent>
#include <QPainter>

class PDFRenderer;

class PDFViewPage : public QAbstractScrollArea
{
    Q_OBJECT
public:
    explicit PDFViewPage(PDFRenderer *r, QWidget *p = nullptr);

    void setPage(int p);
    int page() const { return m_page; }

    void setZoom(double z);
    double zoom() const { return m_zoom; }
    void zoomIn() { setZoom(m_zoom * 1.25); }
    void zoomOut() { setZoom(m_zoom / 1.25); }
    void zoomFit();
    void zoomWidth();
    QSizeF pageSize() const;

signals:
    void pageChanged(int);
    void zoomChanged(double);

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void wheelEvent(QWheelEvent *) override;
    void contextMenuEvent(QContextMenuEvent *) override;

private:
    void render();
    void updateScrollBars();

    QScrollBar *hbar() { return horizontalScrollBar(); }
    QScrollBar *vbar() { return verticalScrollBar(); }

    PDFRenderer *m_r;
    int m_page;
    QImage m_img;
    double m_zoom;
    bool m_panning = false;
    QPoint m_drag;
};

#endif
