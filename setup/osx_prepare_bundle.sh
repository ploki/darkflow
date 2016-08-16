#!/bin/sh

set -e

BUNDLE="$1"
LIBRARY="$BUNDLE/Contents/Library/"
FRAMEWORKS="$BUNDLE/Contents/Frameworks"
RPATH="@rpath/"
QTLIBPATH="/Library/Qt/5.7/clang_64/lib"
QTFRAMEWORKS="QtCore QtDBus QtGui QtPrintSupport QtWidgets"

if [ -z "$BUNDLE" -o ! -d "$BUNDLE/Contents" ] ; then
    echo "usage: $0 <bundle path>" >&2
    exit 1
fi

function resolve_libs ()
{
    local FILE="$1"
    otool -L "$FILE" | awk '/.usr.local./ { print $1} ' | while read ; do
        local LIBPATH="$REPLY"
        local LIBFILE="$(basename $LIBPATH)"
        install_name_tool -change "$LIBPATH" "$RPATH$LIBFILE" "$FILE"
        if [ ! -f "$LIBRARY$LIBFILE" ] ; then
            cp -f "$LIBPATH" "$LIBRARY"
            chmod 755 "$LIBRARY$LIBFILE"
            resolve_libs "$LIBRARY$LIBFILE"
        fi
    done
}

mkdir -p "$LIBRARY"
mkdir -p "$FRAMEWORKS"
mkdir -p "$BUNDLE/Contents/MacOS/platforms"

cp -rf /usr/local/Cellar/imagemagick/6.9.5-5/lib/ImageMagick "$BUNDLE/Contents/Library/"
find "$BUNDLE/Contents/Library/ImageMagick" -type f -name '*.so' -exec chmod 755 {} \;

cp -f /usr/local/bin/dcraw "$BUNDLE/Contents/MacOS/"
chmod 755 "$BUNDLE/Contents/MacOS/dcraw"
install_name_tool -add_rpath @executable_path/../Library "$BUNDLE/Contents/MacOS/dcraw"

cp -f "$QTLIBPATH/../plugins/platforms/libqcocoa.dylib" "$BUNDLE/Contents/MacOS/platforms"
for framework in $QTFRAMEWORKS ; do
    cp -rf "$QTLIBPATH/$framework.framework" "$FRAMEWORKS"
done

perl -pi -e "s,libdir=.*,libdir='./ImageMagick/modules-Q16/coders',g" "$BUNDLE/Contents/Library/ImageMagick/modules-Q16/coders/"*.la
perl -pi -e "s,libdir=.*,libdir='./ImageMagick/modules-Q16/filters',g" "$BUNDLE/Contents/Library/ImageMagick/modules-Q16/filters/"*.la

resolve_libs "$BUNDLE/Contents/MacOS/darkflow"
resolve_libs "$BUNDLE/Contents/MacOS/dcraw"

for plugin in "$BUNDLE/Contents/Library/ImageMagick/modules-Q16/coders/"*.so ; do
    resolve_libs "$plugin"
done

for plugin in "$BUNDLE/Contents/Library/ImageMagick/modules-Q16/filters/"*.so ; do
    resolve_libs "$plugin"
done

(
    cd "$BUNDLE/.."
    hdiutil create -srcfolder darkflow.app -volname darkflow -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW darkflow.dmg
)
