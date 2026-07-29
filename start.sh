#!/usr/bin/env bash

ROOT_DIR="$(pwd)"
BOT_DIR="$ROOT_DIR/lichess-bot"

cmake --build build
source "$BOT_DIR/venv/bin/activate"
source "$ROOT_DIR/.env"
export LICHESS_BOT_TOKEN

cd "$BOT_DIR"

python lichess-bot.py
