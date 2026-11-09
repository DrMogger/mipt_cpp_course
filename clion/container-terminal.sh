#!/usr/bin/env bash
# Open the existing course container; do not rebuild or replace it.
set -euo pipefail
export PATH="/opt/homebrew/bin:/usr/local/bin:$PATH"

repo_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
remote_root="/workspaces/mipt_cpp_course"

if ! docker info >/dev/null 2>&1; then
    echo "Запусти Docker Desktop и открой новую вкладку Terminal в CLion." >&2
    exit 1
fi

container="${MIPT_COURSE_CONTAINER:-}"
if [[ -z "$container" ]]; then
    container="$(docker ps --all --quiet --filter "label=devcontainer.local_folder=$repo_dir")"
fi
if [[ -z "$container" || "$container" == *$'\n'* ]]; then
    echo "Не найден единственный контейнер курса. Укажи его имя в MIPT_COURSE_CONTAINER." >&2
    exit 1
fi

mounted_repo="$(docker inspect --format '{{range .Mounts}}{{if eq .Destination "/workspaces/mipt_cpp_course"}}{{.Source}}{{end}}{{end}}' "$container")"
if [[ "$mounted_repo" != "$repo_dir" ]]; then
    echo "Контейнер $container не содержит этот репозиторий в $remote_root." >&2
    exit 1
fi

if [[ "$(docker inspect --format '{{.State.Running}}' "$container")" != true ]]; then
    docker start "$container" >/dev/null
fi

terminal_args=(-i)
if [[ -t 0 && -t 1 ]]; then
    terminal_args+=(-t)
fi
if [[ $# -eq 0 ]]; then
    set -- /bin/bash -il
fi

exec docker exec "${terminal_args[@]}" \
    --workdir "$remote_root/project" --env "TERM=${TERM:-xterm-256color}" \
    "$container" "$@"
