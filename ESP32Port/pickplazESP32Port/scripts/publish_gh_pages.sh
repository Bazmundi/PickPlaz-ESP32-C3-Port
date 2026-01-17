#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
DOCS_DIR="${PROJECT_DIR}/docs"
DOCS_VENV="${DOCS_DIR}/.venv"
SPHINX_BIN="${DOCS_VENV}/bin/sphinx-build"
BUILD_DIR="${DOCS_DIR}/build/html"

if [[ ! -x "${SPHINX_BIN}" ]]; then
    echo "Docs venv not found at ${DOCS_VENV}."
    echo "Create it with: (cd docs && uv venv && uv sync --group dev)"
    exit 1
fi

if ! command -v doxygen >/dev/null 2>&1; then
    echo "Missing doxygen on PATH."
    exit 1
fi

if ! command -v dot >/dev/null 2>&1; then
    echo "Missing Graphviz 'dot' on PATH."
    exit 1
fi

echo "Building docs (Doxygen + Sphinx)..."
make -C "${DOCS_DIR}" clean-all
make -C "${DOCS_DIR}" docs

if [[ ! -d "${BUILD_DIR}" ]]; then
    echo "Build output not found at ${BUILD_DIR}"
    exit 1
fi

WORKTREE_DIR="$(mktemp -d)"

cleanup() {
    git -C "${PROJECT_DIR}" worktree remove --force "${WORKTREE_DIR}" >/dev/null 2>&1 || true
    rm -rf "${WORKTREE_DIR}" >/dev/null 2>&1 || true
}
trap cleanup EXIT

if git -C "${PROJECT_DIR}" show-ref --verify --quiet refs/heads/gh-pages; then
    git -C "${PROJECT_DIR}" worktree add "${WORKTREE_DIR}" gh-pages
else
    git -C "${PROJECT_DIR}" worktree add --detach "${WORKTREE_DIR}"
    git -C "${WORKTREE_DIR}" checkout --orphan gh-pages
    git -C "${WORKTREE_DIR}" rm -rf . >/dev/null 2>&1 || true
fi

find "${WORKTREE_DIR}" -mindepth 1 -maxdepth 1 ! -name ".git" -exec rm -rf {} +
cp -a "${BUILD_DIR}/." "${WORKTREE_DIR}/"
touch "${WORKTREE_DIR}/.nojekyll"

git -C "${WORKTREE_DIR}" add -A
if git -C "${WORKTREE_DIR}" diff --cached --quiet; then
    echo "No changes to publish on gh-pages."
else
    git -C "${WORKTREE_DIR}" commit -m "Update docs"
    echo "Created gh-pages commit."
fi

echo "To publish: git push origin gh-pages"
