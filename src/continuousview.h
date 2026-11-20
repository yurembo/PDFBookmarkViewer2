#ifndef CONTINUOUSVIEW_H
#define CONTINUOUSVIEW_H

#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QSharedPointer>
#include <QMap>
#include <QTimer>

class PDFRenderer;

class ContinuousView : public QScrollArea
{
    Q_OBJECT
public:
    explicit ContinuousView(PDFRenderer *r, QWidget *p = nullptr);

    void renderAll();
    void clear();
    int currentPage() const { return m_current; }
    void setCurrentPage(int p);

    double zoom() const { return m_zoom; }
    void setZoom(double z);

signals:
    void currentPageChanged(int p);
    void visiblePageChanged(int p);
    void zoomChanged(double z);

public slots:
    void zoomIn();
    void zoomOut();
    void zoomFit();

private:
    void scaleVisiblePages();
    void scaleAllCachedPages();
    void mousePressEvent(QMouseEvent *ev) override;
    void wheelEvent(QWheelEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
    void keyPressEvent(QKeyEvent *ev) override;
    int visiblePage() const;

    QLabel *addPage(int pageNum);
    void removePage(int pageNum);

    PDFRenderer *m_r = nullptr;
    QVBoxLayout *m_layout = nullptr;
    QList<QLabel*> m_labels;
    QList<QSharedPointer<QPixmap>> m_originalPixmaps;
    int m_current = -1;
    double m_zoom = 1.0;

    // Debounce для зума — ждём 150мс после последнего действия
    QTimer m_zoomTimer;
};

#endif
