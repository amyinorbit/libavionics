#!/usr/bin/env bash

PLATFORM=$1
OUTDIR="$(pwd)/build-$PLATFORM"

if [ ! -n "$PLATFORM" ]; then
    echo "please select a platform to build for"
    exit 1
fi

# Make sure we use the cross compiler when making a windows build
if [[ "$PLATFORM" = "windows" ]]; then
    ADDCONF="--host=x86_64-w64-mingw32.static.posix"
else
    ADDCONF=""
fi


if [[ "$PLATFORM" = "linux" ]]; then
    FPIC="-fPIC"
fi

FREETYPE_CONFOPTS_COMMON="--disable-shared --enable-static \
    --with-zlib=no --with-bzip2=no --with-png=no \
    --with-harfbuzz=no --with-old-mac-fonts=no --with-fsspec=no \
    --with-fsref=no --with-quickdraw-toolbox=no \
    --with-quickdraw-carbon=no --with-ats=no --with-brotli=no"

PIXMAN_CONFOPTS_COMMON="--enable-static --disable-shared --disable-gtk \
    --disable-static-testprogs --disable-libpng --disable-openmp"

CAIRO_CONFOPTS_COMMON="--enable-static --disable-egl --disable-glesv2 \
    --disable-glesv3 --disable-glx --disable-gl --disable-valgrind \
    --disable-xlib --enable-ft --disable-shared --disable-xlib-xrender \
    --disable-xcb --disable-svg --disable-full-testing --disable-interpreter \
    --disable-gallium --disable-beos --disable-cogl --disable-directfb \
    --disable-fc --disable-ps --disable-glesv2 --disable-win32 \
    --disable-win32-font --disable-drm --disable-png --disable-script --disable-quartz \
    --disable-wgl --disable-gobject --disable-trace --disable-symbol-lookup" # --disable-pdf 

# CAIRO_CONFOPTS_COMMON="--enable-static --enable-gl=no --enable-ft=yes \
#     --disable-shared --enable-xlib=no --enable-xlib-xrender=no \
#     --enable-xcb=no --enable-xlib-xcb=no --enable-xcb-shm=no --enable-qt=no \
#     --enable-quartz=no --enable-quartz-font=no --enable-quartz-image=no \
#     --enable-win32=no --enable-win32-font=no --enable-skia=no --enable-os2=no \
#     --enable-beos=no --enable-drm=no --enable-gallium=no --enable-png=no \
#     --enable-glesv2=no --enable-cogl=no --enable-directfb=no --enable-vg=no \
#     --enable-egl=no --enable-glx=no --enable-wgl=no --enable-script=no \
#     --enable-fc=no --enable-ps=no --enable-pdf=no --enable-svg=no \
#     --enable-full-testing=no --enable-interpreter=no \
#     --enable-static-testprogs=yes --enable-gobject=no"

# Prepare directories if needed
if [ ! -d $OUTDIR ]; then
    mkdir -p $OUTDIR
fi


# Build freetype
(
if [ ! -f "$OUTDIR/lib/libfreetype.a" ]; then
    export GNUMAKEFLAGS=--no-print-directory
    cd freetype2
    ./autogen.sh --prefix=$OUTDIR $FREETYPE_CONFOPTS_COMMON $ADDCONF || exit 1
    ./configure --prefix=$OUTDIR $FREETYPE_CONFOPTS_COMMON $ADDCONF || exit 1
    make -j9 || exit 1
    make install || exit 1
fi
)

# Build pixman for Cairo
(
if [ ! -f "$OUTDIR/lib/libpixman-1.a" ]; then
    export GNUMAKEFLAGS=--no-print-directory
    cd pixman
    ./autogen.sh --prefix=$OUTDIR $PIXMAN_CONFOPTS_COMMON $ADDCONF || exit 1
    ./configure --prefix=$OUTDIR $FREETYPE_CONFOPTS_COMMON $ADDCONF || exit 1
    make -j9 || exit 1
    make install || exit 1
fi
)

# And finally, build Cairo!
(
if [ ! -f "$OUTDIR/lib/libcairo.a" ]; then
    export GNUMAKEFLAGS=--no-print-directory
    cd cairo
    # Make sure we use the libraries we built, and not the system ones
    export pixman_CFLAGS="-I$OUTDIR/include/pixman-1"
    export pixman_LIBS="$OUTDIR/lib/libpixman-1.a"
    export FREETYPE_CFLAGS="-I$OUTDIR/include/freetype2"
    export FREETYPE_LIBS="$OUTDIR/lib/libfreetype.a"
    export PKG_CONFIG_PATH="$OUTDIR/lib/pkgconfig"
    ./autogen.sh --prefix=$OUTDIR $CAIRO_CONFOPTS_COMMON $ADDCONF || exit 1
    ./configure --prefix=$OUTDIR $CAIRO_CONFOPTS_COMMON $ADDCONF || exit 1
    make -j9 || exit 1
    make install || exit 1
fi
)
