#!/bin/bash

# Locate the absolute directory where this script sits
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ENV_PATH="$SCRIPT_DIR/.env"

echo "🔌 Loading environment configuration..."
if [ -f "$ENV_PATH" ]; then
    # 'set -a' marks all subsequent variables for automatic export
    set -a
    source "$ENV_PATH"
    set +a
else
    echo "❌ Error: Local configuration file '.env' missing!"
    echo "Please create a .env file containing TARGET_IP, TARGET_USER, and TARGET_DIR definitions."
    exit 1
fi

echo "🧹 [0/3] Stopping any running game instances on the Pi..."
# pkill matches the process name. '|| true' prevents the script 
# from crashing if the game wasn't already running.
ssh "${TARGET_USER}@${TARGET_IP}" "pkill -9 game || true"

echo "🚀 [1/3] Synchronizing source files over the network to $TARGET_USER@$TARGET_IP..."
rsync -avz --delete \
    --exclude='build/' \
    --exclude='.git/' \
    --exclude='.vscode/' \
    ./ "${TARGET_USER}@${TARGET_IP}:${TARGET_DIR}"

echo "🛠️ [2/3] Compiling natively on the Raspberry Pi 4..."
ssh "${TARGET_USER}@${TARGET_IP}" "
    cd ${TARGET_DIR} && \
    cmake -B build/native -DBUILD_NATIVE_DRM=ON -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build/native -j\$(nproc)
"

if [ $? -eq 0 ]; then
    echo "🕶️ [3/3] Executing application on the Pi's VR Display..."
    ssh -t "${TARGET_USER}@${TARGET_IP}" "cd ${TARGET_DIR}/build/native && ./game"
else
    echo "❌ Compilation failed. Aborting remote execution."
fi