#!/bin/sh

set -e

DIR=vendor/llvm

if [ ! -d "$DIR/.git" ]; then
    echo ">>> Fetching LLVM..."
    git clone \
        --filter=blob:none \
        --no-checkout \
        "https://github.com/XboxDev/llvm-nxdk.git" \
        "$DIR"

    git -C "$DIR" sparse-checkout init --cone
    git -C "$DIR" sparse-checkout set \
        "libcxx" \
        "libc"

    git -C "$DIR" checkout nxdk
else
    # FIXME
    git -C "$DIR" pull
fi
