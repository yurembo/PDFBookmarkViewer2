#ifndef PDFEDITOR_H
#define PDFEDITOR_H

#include <QString>
#include <QVector>

// Структура для дерева закладок (вложенные)
struct OutlineNode {
    QString title;
    int page; // 0-based
    QVector<OutlineNode> children;
    
    OutlineNode() : page(-1) {}
    OutlineNode(const QString &t, int p) : title(t), page(p) {}
};

class PdfEditor
{
public:
    // Добавить закладки в PDF-файл
    static bool addBookmarks(const QString &inputPath,
                             const QString &outputPath,
                             const QVector<OutlineNode> &bookmarks,
                             int pageCount);
};

#endif
