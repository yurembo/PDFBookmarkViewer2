#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QComboBox>

class PDFRenderer;
class BookmarkManager;
class BookmarkWidget;
class ThumbnailWidget;
class ContinuousView;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *p = nullptr);
    ~MainWindow();

private slots:
    void onOpenFile();
    void onCloseDocument();
    void onSaveBookmarks();
    void onQuit();
    void onNextPage();
    void onPrevPage();
    void onFirstPage();
    void onLastPage();
    void onZoomIn();
    void onZoomOut();
    void onZoomFit();
    void onZoomActual();
    void onAddBookmark();
    void onReadBookmarks();
    void onPageChanged(int p);
    void onThumbPage(int p);
    void onBookmarkActivated(int p);
    void onZoomChanged(double z);

private:
    void setupUI();
    void setupMenus();
    void setupDock();
    void setupStatus();
    void updateNav();
    int currentPage() const;

    PDFRenderer *m_renderer = nullptr;
    BookmarkManager *m_bmManager = nullptr;

    ContinuousView *m_pageView = nullptr;
    BookmarkWidget *m_bmWidget = nullptr;
    ThumbnailWidget *m_thumbWidget = nullptr;

    QComboBox *m_pageCombo = nullptr;
    QLabel *m_pageInfo = nullptr;
    QLabel *m_zoomLabel = nullptr;
    QLabel *m_fileLabel = nullptr;

    int m_page = -1;
    int m_total = 0;
    QString m_file;

    QDockWidget *m_bmDock = nullptr;
    QDockWidget *m_thumbDock = nullptr;
};

#endif
