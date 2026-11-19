# PDF Bookmark Viewer

Кросс-платформенное приложение для просмотра PDF-файлов с поддержкой закладок.

## Возможности

- Открытие и просмотр PDF-файлов
- Добавление, редактирование, удаление закладок
- Сохранение закладок непосредственно в PDF-файл
- Миниатюры страниц для быстрой навигации
- Масштабирование (колесо мыши с Ctrl, кнопки)
- Перетаскивание (Alt+ЛКМ или средняя кнопка мыши)
- Тёмная тема интерфейса
- Кросс-платформенность (Windows, Linux, macOS)

## Требования

- **Qt 6.12** (или новее)
- **Poppler** (poppler-qt5) — Open Source библиотека для работы с PDF
- **CMake 3.16+**
- **C++17**-совместимый компилятор

### Установка зависимостей

#### Windows (vcpkg)

```bash
vcpkg install poppler[qt5]:x64-windows
vcpkg integrate install
```

#### Linux (Ubuntu/Debian)

```bash
sudo apt install libpoppler-dev libpoppler-qt5-dev qt6-base-dev cmake
```

#### Linux (Fedora)

```bash
sudo dnf install poppler-cpp-devel poppler-qt5-devel qt6-qtbase-devel cmake
```

#### macOS (Homebrew)

```bash
brew install poppler qt@6
export QT_DIR=$(brew --prefix qt@6)
export POPPLER_DIR=$(brew --prefix poppler)
```

## Сборка

```bash
cd PDFBookmarkViewer

# Создание build-директории
mkdir build && cd build

# Конфигурация (Windows - vcpkg)
cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake

# Конфигурация (Linux)
cmake ..

# Конфигурация (macOS)
cmake .. -DQt6_DIR=$QT_DIR/lib/cmake/Qt6 \
         -Dpoppler-qt5_DIR=$POPPLER_DIR/lib/cmake/poppler-qt5

# Сборка
cmake --build . --config Release
```

## Использование

- **Открыть файл**: Файл → Открыть (Ctrl+O)
- **Добавить закладку**: Закладки → Добавить (Ctrl+D, Insert)
- **Сохранить закладки**: Файл → Сохранить в PDF (Ctrl+S)
- **Переключить панель закладок**: Вид → Панель закладок (F9)
- **Масштабирование**: Ctrl+колёсико мыши, или +/−
- **Перетаскивание**: Alt+ЛКМ или средняя кнопка мыши

## Архитектура

```
PDFBookmarkViewer/
├── CMakeLists.txt              # Сборочная конфигурация
├── src/
│   ├── main.cpp                # Точка входа
│   ├── mainwindow.h/.cpp       # Главное окно приложения
│   ├── pdfrenderer.h/.cpp      # Обёртка над Poppler
│   ├── bookmarkmanager.h/.cpp  # Управление закладками
│   ├── pdfviewpage.h/.cpp      # Виджет просмотра страницы
│   ├── thumbnailwidget.h/.cpp  # Миниатюры страниц
│   └── bookmarkwidget.h/.cpp   # Панель закладок
└── resources/
    └── icons.qrc               # Ресурсы Qt
```

## Стек технологий

| Компонент | Библиотека | Лицензия |
|-----------|-----------|----------|
| GUI | Qt 6 | Qt Commercial / LGPL v3 |
| PDF | Poppler | GPL v2 / GPL v3 |
| Сборка | CMake | BSD |

## Лицензия проекта

MIT
