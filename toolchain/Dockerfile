FROM debian:bookworm-slim
RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc-mipsel-linux-gnu binutils-mipsel-linux-gnu libc6-dev-mipsel-cross qemu-user make python3 && rm -rf /var/lib/apt/lists/*
WORKDIR /work
