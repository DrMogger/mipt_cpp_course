#!/usr/bin/env bash
# QEMU's GDB stub works on Apple Silicon where ptrace in amd64 containers does not.
set -euo pipefail
export PATH="/opt/homebrew/bin:/usr/local/bin:$PATH"

repo_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
container_name="nano-edr-clion-qemu"
image="nano-edr-clion:local"
port=12345
remote_root="/workspaces/mipt_cpp_course"

if docker container inspect "$container_name" >/dev/null 2>&1; then
    owner="$(docker inspect --format '{{index .Config.Labels "mipt-course.clion-debug"}}' "$container_name")"
    if [[ "$owner" != "$repo_dir" ]]; then
        echo "Container $container_name belongs to another project." >&2
        exit 1
    fi
    docker rm --force "$container_name" >/dev/null
fi

if [[ "${1:-}" == "--stop" ]]; then
    exit 0
fi

scenario="${1:-scenarios/phishing_macro.log}"
docker run --rm --platform linux/amd64 \
    --mount "type=bind,src=$repo_dir,dst=$remote_root" \
    --workdir "$remote_root/project" "$image" \
    bash -lc 'cmake --preset clion-debug && cmake --build --preset clion-debug --target nano-edr --parallel 4'

docker run --detach --rm --platform linux/amd64 \
    --name "$container_name" --label "mipt-course.clion-debug=$repo_dir" \
    --publish "127.0.0.1:$port:$port" \
    --mount "type=bind,src=$repo_dir,dst=$remote_root" \
    --workdir "$remote_root/project" "$image" \
    bash -lc 'QEMU_GDB="$1" exec build/clion-debug/nano-edr "$2"' \
    bash "$port" "$scenario" >/dev/null

# Check the listening socket without consuming the GDB connection.
docker exec "$container_name" python3 -c '
import time
for attempt in range(50):
    with open("/proc/net/tcp") as sockets:
        if any(line.split()[1].endswith(":3039") and line.split()[3] == "0A"
               for line in list(sockets)[1:]):
            break
    time.sleep(0.1)
else:
    raise SystemExit("QEMU GDB server did not start")
'
echo "QEMU GDB: 127.0.0.1:$port ($scenario)"
