#!/bin/sh
source build_dep.common


PIXMAN_CONFOPTS_COMMON="--enable-static --disable-shared --disable-gtk \
    --disable-static-testprogs"

CAIRO_CFLAGS_COMMON="-I$(pwd)/pixman-0.34.0/pixman"=
CAIRO_CONFOPTS_COMMON="--enable-static --enable-gl=no --enable-ft=yes \
    --disable-shared --enable-xlib=no --enable-xlib-xrender=no \
    --enable-xcb=no --enable-xlib-xcb=no --enable-xcb-shm=no --enable-qt=no \
    --enable-quartz=no --enable-quartz-font=no --enable-quartz-image=no \
    --enable-win32=no --enable-win32-font=no --enable-skia=no --enable-os2=no \
    --enable-beos=no --enable-drm=no --enable-gallium=no --enable-png=yes \
    --enable-glesv2=no --enable-cogl=no --enable-directfb=no --enable-vg=no \
    --enable-egl=no --enable-glx=no --enable-wgl=no --enable-script=no \
    --enable-fc=no --enable-ps=no --enable-pdf=no --enable-svg=no \
    --enable-full-testing=no --enable-interpreter=no \
    --enable-static-testprogs=yes"

export PKG_CONFIG_PATH_IN="$(pwd)/build-{PLATFORM}/lib/pkgconfig" \

case `uname` in
	Linux)
		CFLAGS="-m64\\ -g" LDFLAGS="-m64\\ -L$(pwd)/build-linux/lib" \
		    build_dep "linux" "$PIXMAN_CONFOPTS_COMMON" \
		    "pixman" "$PIXMAN_PRODUCT" && \
		    build_dep "windows" "$PIXMAN_CONFOPTS_COMMON \
		    --host=x86_64-w64-mingw32.posix.shared" \
		    "pixman" \
		CFLAGS="$CAIRO_CFLAGS_COMMON\\ -g\\ -m64" LDFLAGS="-m64" \
		    build_dep "linux" "$CAIRO_CONFOPTS_COMMON" \
		    "cairo" && \
		CFLAGS="$CAIRO_CFLAGS_COMMON" \
		    build_dep "windows" "$CAIRO_CONFOPTS_COMMON \
		    --host=x86_64-w64-mingw32.posix.static" \
		    "cairo"
		;;
	Darwin)
		# We must disable built-in support for thread-local-storage,
		# because it messes with VMProtect packed files on OSX.
		# CUSTPREMAKECMD="gsed -i '/^#define TLS/d;' config.h" \
		CFLAGS="-m64" LDFLAGS="-m64" \
		    build_dep "macos" "$PIXMAN_CONFOPTS_COMMON"\
		    "pixman" && \
		CFLAGS="$CAIRO_CFLAGS_COMMON\\ -m64" LDFLAGS="-m64" \
		    build_dep "macos" "$CAIRO_CONFOPTS_COMMON" \
		    "cairo"
		;;
	*)
		echo "Unsupported build platform" >&2
		exit 1
		;;
esac
