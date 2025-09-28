#!/usr/bin/env bash
# compile.sh — arranca el contenedor, limpia y construye dentro de él, luego lo para

# Nombre del contenedor que creaste con 'docker run --name'
CONTAINER="TPE-Arqui"

# Ruta dentro del contenedor donde está el Toolchain
TOOLCHAIN_DIR="/root/Toolchain"
PROJECT_DIR="/root"

# 1) Inicia el contenedor (si ya estaba arrancado, ignora el error)
docker start "$CONTAINER" >/dev/null 2>&1

# 2) Limpia y compila Toolchain como tu usuario (mismo UID/GID que en host)
UID_GID="$(id -u):$(id -g)"
docker exec -u "$UID_GID" -w "$TOOLCHAIN_DIR" -it "$CONTAINER" make clean
docker exec -u "$UID_GID" -w "$TOOLCHAIN_DIR" -it "$CONTAINER" make

# 3) Limpia y compila el resto del proyecto
docker exec -u "$UID_GID" -w "$PROJECT_DIR" -it "$CONTAINER" make clean
docker exec -u "$UID_GID" -w "$PROJECT_DIR" -it "$CONTAINER" make

# 4) Para el contenedor
# echo "✅ Compilación completada dentro del contenedor '$CONTAINER'."

#!/bin/bash
qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 -audiodev pa,id=speaker -machine pcspk-audiodev=speaker

