#ifndef DOCUMENTVIEW_H
#define DOCUMENTVIEW_H

#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>

class PDFRenderer;

class DocumentView : public QScrollArea
{
    Q_OBJECT
public:
    explicit DocumentView(PDFRenderer *r, QWidget *p = nullptr);

    void renderAll();
    void clear();
    int pageCount() const;
    int currentPixelOffset() const;

signals:
    void pageClicked(int pageNum);

private:
    void addPage(int pageNum);
    void removePage(int pageNum);

    PDFRenderer *m_r;
    QVBoxLayout *m_layout;
    QList<QLabel*> m_labels;
};

#endif
