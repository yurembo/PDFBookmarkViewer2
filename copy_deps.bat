@echo off
setlocal

set OUT=C:\Users\yurem\PDFBookmarkViewer\build\Release
set VCPKG_BIN=D:\yurem\vcpkg\installed\x64-windows\bin
set QT_PLUGINS=D:\Qt\6.12.0\msvc2022_64\plugins

if not exist "%OUT%\platforms" mkdir "%OUT%\platforms"
if not exist "%OUT%\imageformats" mkdir "%OUT%\imageformats"

copy /y "%QT_PLUGINS%\platforms\qwindows.dll" "%OUT%\platforms\" >nul
copy /y "%QT_PLUGINS%\platforms\qdirect2d.dll" "%OUT%\platforms\" >nul
copy /y "%QT_PLUGINS%\imageformats\qjpeg.dll" "%OUT%\imageformats\" >nul
copy /y "%QT_PLUGINS%\imageformats\qgif.dll" "%OUT%\imageformats\" >nul
copy /y "%QT_PLUGINS%\imageformats\qsvg.dll" "%OUT%\imageformats\" >nul
copy /y "%QT_PLUGINS%\imageformats\qico.dll" "%OUT%\imageformats\" >nul
copy /y "%QT_PLUGINS%\imageformats\qwebp.dll" "%OUT%\imageformats\" >nul
copy /y "%QT_PLUGINS%\imageformats\qtiff.dll" "%OUT%\imageformats\" >nul
copy /y "%QT_PLUGINS%\imageformats\qtga.dll" "%OUT%\imageformats\" >nul
copy /y "%QT_PLUGINS%\imageformats\qicns.dll" "%OUT%\imageformats\" >nul
copy /y "%QT_PLUGINS%\imageformats\qwbmp.dll" "%OUT%\imageformats\" >nul
copy /y "%QT_PLUGINS%\imageformats\qpdf.dll" "%OUT%\imageformats\" >nul

copy /y "D:\Qt\6.12.0\msvc2022_64\bin\Qt6Core.dll" "%OUT%\" >nul
copy /y "D:\Qt\6.12.0\msvc2022_64\bin\Qt6Gui.dll" "%OUT%\" >nul
copy /y "D:\Qt\6.12.0\msvc2022_64\bin\Qt6Widgets.dll" "%OUT%\" >nul
copy /y "D:\Qt\6.12.0\msvc2022_64\bin\Qt6Concurrent.dll" "%OUT%\" >nul
copy /y "D:\Qt\6.12.0\msvc2022_64\bin\Qt6Pdf.dll" "%OUT%\" >nul
copy /y "D:\Qt\6.12.0\msvc2022_64\bin\Qt6Network.dll" "%OUT%\" >nul

copy /y "%VCPKG_BIN%\icuin78.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\icuuc78.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\icudt78.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\iconv-2.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\icuio78.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\icutu78.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\z.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\zstd.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\double-conversion.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\pcre2-16.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\pcre2-32.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\pcre2-8.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\pcre2-posix.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\harfbuzz.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\harfbuzz-gpu.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\harfbuzz-raster.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\harfbuzz-subset.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\harfbuzz-vector.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\freetype.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\libpng16.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\bz2.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\brotlidec.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\brotlicommon.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\md4c.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\poppler.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\poppler-qt6.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\jpeg62.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\openjp2.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\tiff.dll" "%OUT%\" >nul
copy /y "%VCPKG_BIN%\liblzma.dll" "%OUT%\" >nul

exit /b 0
