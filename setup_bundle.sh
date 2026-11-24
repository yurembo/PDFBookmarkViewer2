#!/bin/bash
set -e

BUNDLE_DIR="$1"
APP_BINARY="$2"
POPPLER_QT6="/opt/homebrew/opt/poppler-qt6"
POPPLER="/opt/homebrew/opt/poppler"

echo "=== Setting up PDFBookmarkViewer bundle ==="

FRAMEWORKS_DIR="$BUNDLE_DIR/Contents/Frameworks"
mkdir -p "$FRAMEWORKS_DIR"

# Copy Poppler dylibs (actual files, not symlinks)
echo "Copying Poppler dylibs..."
cp -f "$POPPLER/lib/libpoppler-cpp.3.1.0.dylib" "$FRAMEWORKS_DIR/libpoppler-cpp.dylib"
cp -f "$POPPLER/lib/libpoppler.166.0.0.dylib" "$FRAMEWORKS_DIR/libpoppler.166.dylib"
cp -f "$POPPLER_QT6/lib/libpoppler-qt6.3.14.0.dylib" "$FRAMEWORKS_DIR/libpoppler-qt6.dylib"

# Fix install names in main executable
echo "Fixing install names in executable..."
/usr/bin/install_name_tool -change \
    "$POPPLER_QT6/lib/libpoppler-cpp.3.dylib" \
    "@rpath/libpoppler-cpp.dylib" \
    "$APP_BINARY"

/usr/bin/install_name_tool -change \
    "$POPPLER_QT6/lib/libpoppler-qt6.3.dylib" \
    "@rpath/libpoppler-qt6.dylib" \
    "$APP_BINARY"

# Fix install names inside copied dylibs
echo "Fixing install names in dylibs..."
/usr/bin/install_name_tool -id "@rpath/libpoppler-cpp.dylib" \
    "$FRAMEWORKS_DIR/libpoppler-cpp.dylib"

/usr/bin/install_name_tool -id "@rpath/libpoppler.166.dylib" \
    "$FRAMEWORKS_DIR/libpoppler.166.dylib"

/usr/bin/install_name_tool -id "@rpath/libpoppler-qt6.dylib" \
    "$FRAMEWORKS_DIR/libpoppler-qt6.dylib"

# Fix libpoppler-qt6 internal deps -> point to @rpath versions
echo "Fixing internal dependencies..."
/usr/bin/install_name_tool -change \
    "/opt/homebrew/opt/poppler/lib/libpoppler.166.dylib" \
    "@rpath/libpoppler.166.dylib" \
    "$FRAMEWORKS_DIR/libpoppler-qt6.dylib"

/usr/bin/install_name_tool -change \
    "/opt/homebrew/opt/poppler-qt6/lib/libpoppler-cpp.3.dylib" \
    "@rpath/libpoppler-cpp.dylib" \
    "$FRAMEWORKS_DIR/libpoppler-qt6.dylib"

/usr/bin/install_name_tool -change \
    "/opt/homebrew/opt/poppler/lib/libpoppler.166.0.0.dylib" \
    "@rpath/libpoppler.166.dylib" \
    "$FRAMEWORKS_DIR/libpoppler-qt6.dylib"

# Fix libpoppler-cpp internal deps
/usr/bin/install_name_tool -change \
    "/opt/homebrew/opt/poppler/lib/libpoppler.166.dylib" \
    "@rpath/libpoppler.166.dylib" \
    "$FRAMEWORKS_DIR/libpoppler-cpp.dylib"

/usr/bin/install_name_tool -change \
    "/opt/homebrew/opt/poppler/lib/libpoppler.166.0.0.dylib" \
    "@rpath/libpoppler.166.dylib" \
    "$FRAMEWORKS_DIR/libpoppler-cpp.dylib"

# Deep sign the bundle
echo "Codesigning bundle..."
/usr/bin/codesign --deep --force --sign - --timestamp=none "$BUNDLE_DIR"

echo "=== Done! ==="
