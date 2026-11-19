#ifndef THUMBNAILWIDGET_H
#define THUMBNAILWIDGET_H

#include <QWidget>
#include <QImage>
#include <QList>
#include <QPainter>
#include <QMouseEvent>
#include <QScrollBar>

class PDFRenderer;

class ThumbnailWidget : public QWidget
{
    Q_OBJECT
public:
    ThumbnailWidget(PDFRenderer *r, QWidget *p = nullptr);

    void setCurrentPage(int p);
    void updateThumbnails();

signals:
    void pageRequested(int pageNum);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *ev) override;
    void resizeEvent(QResizeEvent *) override;

private:
    int pageAtY(int y) const;

    PDFRenderer *m_r;
    int m_currentPage;
    QList<QImage> m_thumbs;
    static constexpr int kWidth = 120;
    static constexpr int kSpacing = 8;
};

#endif
