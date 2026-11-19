#include "pdfeditor.h"

#include <QFile>
#include <QByteArray>
#include <QString>
#include <QDebug>

// Создаём PDF-строку из текста (эскейп скобок, бэкслэшей)
static QString makePdfString(const QString &text)
{
    QString s;
    for (QChar c : text) {
        if (c == QLatin1Char('(')) s += QLatin1String("\\(");
        else if (c == QLatin1Char(')')) s += QLatin1String("\\)");
        else if (c == QLatin1Char('\\')) s += QLatin1String("\\\\");
        else s += c;
    }
    return s;
}

// Создаём объект закладки (без вложенности для простоты)
static QByteArray createBookmarkObject(int objId, const OutlineNode &node, int pageObjId)
{
    QByteArray obj;
    obj.append(QString("%1 0 obj\n").arg(objId).toUtf8());
    obj.append("<<\n");
    obj.append("/Type /OutlinItem\n");
    obj.append("/Title (");
    obj.append(makePdfString(node.title).toUtf8());
    obj.append(")\n");
    obj.append("/Dest [");
    obj.append(QString::number(pageObjId).toUtf8());
    obj.append(" /Fit]\n");
    
    // Дочерние элементы
    if (!node.children.isEmpty()) {
        // TODO: реализовать рекурсивно
        obj.append("/First /1 0 R\n");
        obj.append("/Last /1 0 R\n");
    }
    
    obj.append(">>\nendobj\n");
    return obj;
}

// Создаём объект /Outlines
static QByteArray createOutlinesObject(int objId, int firstBookmarkObjId, int lastBookmarkObjId)
{
    QByteArray obj;
    obj.append(QString("%1 0 obj\n").arg(objId).toUtf8());
    obj.append("<<\n");
    obj.append("/Type /Outlines\n");
    obj.append("/First " + QString::number(firstBookmarkObjId).toUtf8() + "\n");
    obj.append("/Last " + QString::number(lastBookmarkObjId).toUtf8() + "\n");
    obj.append(">>\nendobj\n");
    return obj;
}

bool PdfEditor::addBookmarks(const QString &inputPath,
                              const QString &outputPath,
                              const QVector<OutlineNode> &bookmarks,
                              int pageCount)
{
    // 1. Читаем исходный PDF
    QFile inFile(inputPath);
    if (!inFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open PDF";
        return false;
    }
    QByteArray pdfData = inFile.readAll();
    inFile.close();
    
    // 2. Находим место перед %%EOF
    int eofPos = pdfData.lastIndexOf("%%EOF");
    if (eofPos == -1) {
        qDebug() << "No %%EOF found";
        return false;
    }
    
    // Отрезаем %%EOF и trailer
    QByteArray mainPdf = pdfData.left(eofPos);
    
    // 3. Парсим trailer для получения /Size и /Root
    int trailerStart = mainPdf.lastIndexOf("trailer");
    if (trailerStart == -1) {
        qDebug() << "No trailer found";
        return false;
    }
    
    int trailerEnd = mainPdf.indexOf(">>", trailerStart);
    if (trailerEnd == -1) {
        qDebug() << "No trailer end found";
        return false;
    }
    trailerEnd += 2;
    
    QByteArray trailerContent = mainPdf.mid(trailerStart, trailerEnd - trailerStart);
    
    // Получаем существующий Size
    int sizePos = trailerContent.indexOf("/Size");
    int sizeEnd = trailerContent.indexOf("\n", sizePos);
    int existingSize = 0;
    if (sizePos != -1 && sizeEnd != -1) {
        QString sizeStr = QString::fromUtf8(trailerContent.mid(sizePos + 6, sizeEnd - sizePos - 6)).trimmed();
        existingSize = sizeStr.toInt();
    }
    
    qDebug() << "Existing object count:" << existingSize;
    
    // 4. Создаём объекты закладок
    int nextObjId = existingSize;
    QVector<QByteArray> bookmarkObjects;
    
    for (const auto &node : bookmarks) {
        QByteArray obj = createBookmarkObject(++nextObjId, node, 1);
        bookmarkObjects.append(obj);
    }
    
    if (bookmarkObjects.isEmpty()) {
        qDebug() << "No bookmarks to add";
        return false;
    }
    
    // 5. Создаём объект /Outlines
    int outlinesObjId = ++nextObjId;
    QByteArray outlinesObj = createOutlinesObject(outlinesObjId, bookmarkObjects.first().mid(0, 3).toInt(), 
                                                   bookmarkObjects.last().mid(0, 3).toInt());
    
    // 6. Обновляем trailer — добавляем /Outlines в root
    // Находим /Root в trailer
    int rootPos = trailerContent.indexOf("/Root");
    if (rootPos != -1) {
        // Добавляем /Outlines после /Root
        int rootLineEnd = trailerContent.indexOf("\n", rootPos);
        if (rootLineEnd != -1) {
            QByteArray newEntry = "/Outlines " + QString::number(outlinesObjId).toUtf8() + "\n";
            trailerContent.insert(rootLineEnd + 1, newEntry);
        }
    }
    
    // Обновляем /Size
    if (sizePos != -1 && sizeEnd != -1) {
        int newSize = nextObjId + 1;
        QString newSizeStr = QString::number(newSize);
        trailerContent.replace(sizePos + 6, sizeEnd - sizePos - 6, newSizeStr.toUtf8());
    }
    
    // 7. Собираем новый PDF
    QByteArray newData;
    newData.append(mainPdf.left(trailerStart)); // Без старого trailer
    newData.append(trailerContent);
    newData.append("\n");
    
    // Добавляем объекты закладок
    for (const auto &obj : bookmarkObjects) {
        newData.append(obj);
    }
    
    // Добавляем объект /Outlines
    newData.append(outlinesObj);
    
    // Добавляем %%EOF
    newData.append(pdfData.mid(eofPos));
    
    // 8. Сохраняем
    QFile outFile(outputPath);
    if (!outFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Cannot open output file";
        return false;
    }
    outFile.write(newData);
    outFile.close();
    
    qDebug() << "PDF saved with" << bookmarkObjects.size() << "bookmarks, total objects:" << nextObjId;
    return true;
}
