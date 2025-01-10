#!/usr/bin/env sh

set -e

echo "Running autoreconf -ivf ..."
autoreconf -iv || { echo "Error: autoreconf failed. Please install autoconf." >&2; exit 1; }

echo "Running autoheader..."
autoheader || { echo "Error: autoheader failed. Please install autoconf." >&2; exit 1; }

echo "Running automake --add-missing --copy ..."
automake --add-missing --copy || { echo "Error: automake failed. Please install automake." >&2; exit 1; }

echo "Running aclocal..."
aclocal || { echo "Error: aclocal failed. Please install automake." >&2; exit 1; }

echo "Done!"
echo "Now run './configure' and 'make'."
