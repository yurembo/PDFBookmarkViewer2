@echo off
setlocal

set OUT=%~1
set QT_BIN=D:\Qt\6.12.0\msvc2022_64\bin
set QT_PLUGINS=D:\Qt\6.12.0\msvc2022_64\plugins
set VCPKG_BIN=D:\yurem\vcpkg\installed\x64-windows\bin

:: Создаём директории
if not exist "%OUT%\platforms" mkdir "%OUT%\platforms"
if not exist "%OUT%\imageformats" mkdir "%OUT%\imageformats"

:: Копируем плагины (игнорируем ошибки если уже есть)
copy /y "%QT_PLUGINS%\platforms\qwindows.dll" "%OUT%\platforms\" >nul 2>&1
copy /y "%QT_PLUGINS%\platforms\qdirect2d.dll" "%OUT%\platforms\" >nul 2>&1
copy /y "%QT_PLUGINS%\imageformats\qjpeg.dll" "%OUT%\imageformats\" >nul 2>&1
copy /y "%QT_PLUGINS%\imageformats\qgif.dll" "%OUT%\imageformats\" >nul 2>&1
copy /y "%QT_PLUGINS%\imageformats\qsvg.dll" "%OUT%\imageformats\" >nul 2>&1
copy /y "%QT_PLUGINS%\imageformats\qico.dll" "%OUT%\imageformats\" >nul 2>&1
copy /y "%QT_PLUGINS%\imageformats\qwebp.dll" "%OUT%\imageformats\" >nul 2>&1
copy /y "%QT_PLUGINS%\imageformats\qtiff.dll" "%OUT%\imageformats\" >nul 2>&1
copy /y "%QT_PLUGINS%\imageformats\qtga.dll" "%OUT%\imageformats\" >nul 2>&1
copy /y "%QT_PLUGINS%\imageformats\qicns.dll" "%OUT%\imageformats\" >nul 2>&1
copy /y "%QT_PLUGINS%\imageformats\qwbmp.dll" "%OUT%\imageformats\" >nul 2>&1
copy /y "%QT_PLUGINS%\imageformats\qpdf.dll" "%OUT%\imageformats\" >nul 2>&1

:: Qt DLL
copy /y "%QT_BIN%\Qt6Core.dll" "%OUT%\" >nul 2>&1
copy /y "%QT_BIN%\Qt6Gui.dll" "%OUT%\" >nul 2>&1
copy /y "%QT_BIN%\Qt6Widgets.dll" "%OUT%\" >nul 2>&1
copy /y "%QT_BIN%\Qt6Concurrent.dll" "%OUT%\" >nul 2>&1
copy /y "%QT_BIN%\Qt6Pdf.dll" "%OUT%\" >nul 2>&1

:: Зависимости Qt
copy /y "%QT_BIN%\icuin78.dll" "%OUT%\" >nul 2>&1
copy /y "%QT_BIN%\icuuc78.dll" "%OUT%\" >nul 2>&1
copy /y "%QT_BIN%\icudt78.dll" "%OUT%\" >nul 2>&1
copy /y "%QT_BIN%\z.dll" "%OUT%\" >nul 2>&1
copy /y "%QT_BIN%\double-conversion.dll" "%OUT%\" >nul 2>&1
copy /y "%QT_BIN%\pcre2-16.dll" "%OUT%\" >nul 2>&1
copy /y "%QT_BIN%\zstd.dll" "%OUT%\" >nul 2>&1
copy /y "%QT_BIN%\libpng16.dll" "%OUT%\" >nul 2>&1
copy /y "%QT_BIN%\harfbuzz.dll" "%OUT%\" >nul 2>&1
copy /y "%QT_BIN%\freetype.dll" "%OUT%\" >nul 2>&1
copy /y "%QT_BIN%\bz2.dll" "%OUT%\" >nul 2>&1
copy /y "%QT_BIN%\brotlidec.dll" "%OUT%\" >nul 2>&1
copy /y "%QT_BIN%\brotlicommon.dll" "%OUT%\" >nul 2>&1
copy /y "%QT_BIN%\md4c.dll" "%OUT%\" >nul 2>&1

:: Vcpkg DLLs
copy /y "%VCPKG_BIN%\poppler-qt6.dll" "%OUT%\" >nul 2>&1
copy /y "%VCPKG_BIN%\poppler.dll" "%OUT%\" >nul 2>&1
copy /y "%VCPKG_BIN%\jpeg62.dll" "%OUT%\" >nul 2>&1
copy /y "%VCPKG_BIN%\openjp2.dll" "%OUT%\" >nul 2>&1
copy /y "%VCPKG_BIN%\tiff.dll" "%OUT%\" >nul 2>&1
copy /y "%VCPKG_BIN%\liblzma.dll" "%OUT%\" >nul 2>&1

echo Done copying all dependencies
exit /b 0
