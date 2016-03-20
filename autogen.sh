#!/bin/sh

aclocal $ACLOCAL_FLAGS || {
    echo "error: aclocal $ACLOCAL_FLAGS failed"
    exit 1
}

autoconf || {
    echo "error: autoconf failed"
    exit 1
}

if [ -z "$NOCONFIGURE" ]; then
    ./configure "$@"
fi

# vim:set shiftwidth=4 expandtab:
