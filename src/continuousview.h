#ifndef CONTINUOUSVIEW_H
#define CONTINUOUSVIEW_H

#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QList>
#include <QTimer>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>

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
    void bookmarkActivated(int p, double z);

public slots:
    void zoomIn();
    void zoomOut();
    void zoomFit();

protected:
    void mousePressEvent(QMouseEvent *ev) override;
    void wheelEvent(QWheelEvent *ev) override;
    void keyPressEvent(QKeyEvent *ev) override;

private:
    void zoomApply();
    int visiblePage() const;

    QLabel *addPage(int pageNum);

    PDFRenderer *m_r = nullptr;
    QVBoxLayout *m_layout = nullptr;
    QList<QLabel*> m_labels;
    int m_current = -1;
    double m_zoom = 1.0;
    QTimer m_zoomTimer;
};

#endif
