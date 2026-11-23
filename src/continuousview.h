#ifndef CONTINUOUSVIEW_H
#define CONTINUOUSVIEW_H

#include <QWidget>

class QPdfView;
class QPdfDocument;
class QPdfPageNavigator;

class ContinuousView : public QWidget
{
    Q_OBJECT
public:
    explicit ContinuousView(QWidget *p = nullptr);
    ~ContinuousView();

    void setDocument(QPdfDocument *doc);

    int currentPage() const;
    void setCurrentPage(int p);

    double zoom() const;
    void setZoom(double z);

signals:
    void currentPageChanged(int p);
    void visiblePageChanged(int p);
    void zoomChanged(double z);

public slots:
    void zoomIn();
    void zoomOut();
    void zoomFit();

private slots:
    void onCurrentPageChanged(int p);

private:
    QPdfView *m_pdfView = nullptr;
    QPdfDocument *m_doc = nullptr;
};

#endif
