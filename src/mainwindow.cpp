#include "mainwindow.h"
#include "pdfrenderer.h"
#include "bookmarkmanager.h"
#include "bookmarkwidget.h"
#include "thumbnailwidget.h"
#include "continuousview.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QDockWidget>
#include <QFileInfo>
#include <QDebug>
#include <QInputDialog>
#include <QLineEdit>

MainWindow::MainWindow(QWidget *p)
    : QMainWindow(p)
{
    setWindowTitle("PDF Bookmark Viewer");
    setMinimumSize(800, 600);

    m_renderer = new PDFRenderer(this);
    m_bmManager = new BookmarkManager(this);
    m_bmManager->setRenderer(m_renderer);

    connect(m_renderer, &PDFRenderer::documentLoaded, this, [this]() {
        m_file = m_renderer->fileName();
        m_total = m_renderer->pageCount();
        m_bmManager->readBookmarksFromPDF();
        updateNav();
        m_page = 0;
        m_pageView->renderAll();
        m_pageView->setCurrentPage(0);
        m_pageCombo->setCurrentIndex(0);
        m_thumbWidget->setCurrentPage(0);

        QString title = "PDF Bookmark Viewer";
        if (!m_file.isEmpty())
            title += " — " + QFileInfo(m_file).fileName();
        setWindowTitle(title);

        QString meta = m_renderer->documentTitle();
        if (meta.isEmpty()) meta = m_file;
        m_fileLabel->setText(meta);
    });

    connect(m_renderer, &PDFRenderer::documentClosed, this, [this]() {
        m_file.clear(); m_total = 0; m_page = -1;
        updateNav();
        m_pageView->clear();
    });

    setupUI();
    setupMenus();
    setupDock();
    setupStatus();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    auto *cent = new QWidget(this);
    auto *lay = new QVBoxLayout(cent);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    // Тулбар навигации
    auto *tb = new QWidget(cent);
    auto *tbLay = new QHBoxLayout(tb);
    tbLay->setContentsMargins(4, 4, 4, 4);
    tbLay->setSpacing(4);

    auto *prev = new QToolButton(tb); prev->setText("◀");
    connect(prev, &QToolButton::clicked, this, &MainWindow::onPrevPage);

    auto *next = new QToolButton(tb); next->setText("▶");
    connect(next, &QToolButton::clicked, this, &MainWindow::onNextPage);

    auto *first = new QToolButton(tb); first->setText("⏮");
    connect(first, &QToolButton::clicked, this, &MainWindow::onFirstPage);

    auto *last = new QToolButton(tb); last->setText("⏭");
    connect(last, &QToolButton::clicked, this, &MainWindow::onLastPage);

    m_pageCombo = new QComboBox(tb);
    m_pageCombo->setFixedWidth(80);
    m_pageCombo->setEnabled(false);
    connect(m_pageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int i) { if (i >= 0) onPageChanged(i); });

    auto *zin = new QToolButton(tb); zin->setText("+");
    connect(zin, &QToolButton::clicked, this, &MainWindow::onZoomIn);

    auto *zout = new QToolButton(tb); zout->setText("−");
    connect(zout, &QToolButton::clicked, this, &MainWindow::onZoomOut);

    auto *zfit = new QToolButton(tb); zfit->setText("∅");
    connect(zfit, &QToolButton::clicked, this, &MainWindow::onZoomFit);

    tbLay->addWidget(first);
    tbLay->addWidget(prev);
    tbLay->addWidget(m_pageCombo);
    tbLay->addWidget(next);
    tbLay->addWidget(last);
    tbLay->addStretch();
    tbLay->addWidget(zin);
    tbLay->addWidget(zout);
    tbLay->addWidget(zfit);

    m_pageView = new ContinuousView(m_renderer, cent);

    connect(m_pageView, &ContinuousView::currentPageChanged, this, &MainWindow::onPageChanged);
    connect(m_pageView, &ContinuousView::visiblePageChanged, this, [this](int p) {
        if (p >= 0 && p < m_total) {
            m_pageInfo->setText(QString("Стр: %1 / %2").arg(p + 1).arg(m_total));
        }
    });
    connect(m_pageView, &ContinuousView::zoomChanged, this, &MainWindow::onZoomChanged);

    lay->addWidget(tb);
    lay->addWidget(m_pageView);
    setCentralWidget(cent);
}

void MainWindow::setupMenus()
{
    // Файл
    auto *fMenu = menuBar()->addMenu("&Файл");

    auto *openAct = new QAction("☐ Открыть...", this);
    openAct->setShortcut(Qt::CTRL | Qt::Key_O);
    connect(openAct, &QAction::triggered, this, &MainWindow::onOpenFile);
    fMenu->addAction(openAct);

    auto *closeAct = new QAction("✕ Закрыть", this);
    closeAct->setShortcut(Qt::CTRL | Qt::Key_W);
    connect(closeAct, &QAction::triggered, this, &MainWindow::onCloseDocument);
    fMenu->addAction(closeAct);

    fMenu->addSeparator();

    auto *quitAct = new QAction("Выход", this);
    quitAct->setShortcut(Qt::CTRL | Qt::Key_Q);
    connect(quitAct, &QAction::triggered, this, &MainWindow::onQuit);
    fMenu->addAction(quitAct);

    // Вид
    auto *vMenu = menuBar()->addMenu("&Вид");

    vMenu->addAction("◀ Предыдущая", this, &MainWindow::onPrevPage)->setShortcut(Qt::Key_Left);
    vMenu->addAction("▶ Следующая", this, &MainWindow::onNextPage)->setShortcut(Qt::Key_Right);
    vMenu->addSeparator();
    vMenu->addAction("Увеличить", this, &MainWindow::onZoomIn)->setShortcut(Qt::CTRL | Qt::Key_Plus);
    vMenu->addAction("Уменьшить", this, &MainWindow::onZoomOut)->setShortcut(Qt::CTRL | Qt::Key_Minus);
    vMenu->addAction("По высоте", this, &MainWindow::onZoomFit)->setShortcut(Qt::CTRL | Qt::Key_0);
    vMenu->addAction("100%", this, &MainWindow::onZoomActual)->setShortcut(Qt::CTRL | Qt::Key_1);
    vMenu->addSeparator();

    auto *toggleBm = new QAction("Панель закладок", this);
    toggleBm->setCheckable(true); toggleBm->setChecked(true);
    toggleBm->setShortcut(Qt::Key_F9);
    connect(toggleBm, &QAction::toggled, this, [this](bool on) { m_bmDock->setVisible(on); });
    vMenu->addAction(toggleBm);

    auto *toggleThumb = new QAction("Панель страниц", this);
    toggleThumb->setCheckable(true); toggleThumb->setChecked(true);
    connect(toggleThumb, &QAction::toggled, this, [this](bool on) { m_thumbDock->setVisible(on); });
    vMenu->addAction(toggleThumb);

    // Закладки
    auto *bMenu = menuBar()->addMenu("&Закладки");

    auto *addBm = new QAction("+ Добавить закладку", this);
    addBm->setShortcut(Qt::CTRL | Qt::Key_D);
    connect(addBm, &QAction::triggered, this, &MainWindow::onAddBookmark);
    bMenu->addAction(addBm);

    // Справка
    auto *hMenu = menuBar()->addMenu("&Справка");
    auto *about = new QAction("О программе", this);
    connect(about, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "О программе", "PDF Bookmark Viewer v1.0\n"
            "Qt 6 + Poppler\nВсе библиотеки Open Source.");
    });
    hMenu->addAction(about);
}

void MainWindow::setupDock()
{
    m_bmWidget = new BookmarkWidget(m_bmManager, this);
    m_bmDock = new QDockWidget("Закладки", this);
    m_bmDock->setWidget(m_bmWidget);
    m_bmDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, m_bmDock);

    connect(m_bmWidget, &BookmarkWidget::bookmarkActivated, this, &MainWindow::onBookmarkActivated);

    m_thumbWidget = new ThumbnailWidget(m_renderer, this);
    m_thumbDock = new QDockWidget("Страницы", this);
    m_thumbDock->setWidget(m_thumbWidget);
    m_thumbDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_thumbDock);
    connect(m_thumbWidget, &ThumbnailWidget::pageRequested, this, &MainWindow::onThumbPage);
}

void MainWindow::setupStatus()
{
    m_pageInfo = new QLabel("Стр: - / -", this);
    m_zoomLabel = new QLabel("100%", this);
    m_fileLabel = new QLabel("", this);

    statusBar()->addPermanentWidget(m_fileLabel);
    statusBar()->addPermanentWidget(m_pageInfo);
    statusBar()->addPermanentWidget(m_zoomLabel);
}

void MainWindow::updateNav()
{
    m_pageCombo->clear();
    for (int i = 1; i <= m_total; ++i)
        m_pageCombo->addItem(QString("Стр. %1").arg(i));
    m_pageCombo->setEnabled(m_total > 0);
}

int MainWindow::currentPage() const
{
    return m_pageView ? m_pageView->currentPage() : -1;
}

// === Slots ===

void MainWindow::onOpenFile()
{
    QString fp = QFileDialog::getOpenFileName(this, "Открыть PDF", QString(), "PDF (*.pdf)");
    if (fp.isEmpty()) return;
    if (!m_renderer->loadDocument(fp))
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть PDF");
}

void MainWindow::onCloseDocument()
{
    m_renderer->closeDocument();
}

void MainWindow::onQuit() { close(); }

void MainWindow::onNextPage()
{
    if (m_page >= 0 && m_page < m_total - 1) onPageChanged(m_page + 1);
}

void MainWindow::onPrevPage()
{
    if (m_page > 0) onPageChanged(m_page - 1);
}

void MainWindow::onFirstPage() { if (m_total > 0) onPageChanged(0); }
void MainWindow::onLastPage()  { if (m_total > 0) onPageChanged(m_total - 1); }

void MainWindow::onZoomIn()
{
    if (m_pageView) m_pageView->zoomIn();
}

void MainWindow::onZoomOut()
{
    if (m_pageView) m_pageView->zoomOut();
}

void MainWindow::onZoomFit()
{
    if (m_pageView) m_pageView->zoomFit();
}

void MainWindow::onZoomActual()
{
    if (m_pageView) m_pageView->setZoom(1.0);
}

void MainWindow::onAddBookmark()
{
    if (!m_renderer->isOpen()) {
        QMessageBox::warning(this, "Предупреждение", "Откройте PDF-файл");
        return;
    }
    int pg = currentPage();
    if (pg < 0) return;

    QString def = QString("Закладка %1 (стр. %2)")
        .arg(m_bmManager->userBookmarks().size() + 1)
        .arg(pg + 1);

    bool ok;
    QString title = QInputDialog::getText(this, "Новая закладка",
        "Название:", QLineEdit::Normal, def, &ok);
    if (!ok || title.trimmed().isEmpty()) return;

    m_bmManager->addUserBookmark(title.trimmed(), pg);
}

void MainWindow::onPageChanged(int p)
{
    if (p < 0 || p >= m_total) return;
    m_page = p;
    m_pageView->setCurrentPage(p);
    m_pageCombo->setCurrentIndex(p);
    m_thumbWidget->setCurrentPage(p);
    m_pageInfo->setText(QString("Стр: %1 / %2").arg(p + 1).arg(m_total));
}

void MainWindow::onThumbPage(int p) { onPageChanged(p); }

void MainWindow::onBookmarkActivated(int p, double z)
{
    // -2 означает "добавить закладку" через контекстное меню
    if (p == -2) {
        onAddBookmark();
        return;
    }
    if (p < 0 || p >= m_total) return;
    m_page = p;
    m_pageView->setCurrentPage(p);
    m_pageView->setZoom(z);
    m_pageCombo->setCurrentIndex(p);
    m_thumbWidget->setCurrentPage(p);
    m_pageInfo->setText(QString("Стр: %1 / %2").arg(p + 1).arg(m_total));
}

void MainWindow::onZoomChanged(double z)
{
    m_zoomLabel->setText(QString("%1%").arg(qRound(z * 100)));
}
