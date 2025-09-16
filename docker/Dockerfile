FROM debian:bookworm

RUN apt update \
    && DEBIAN_FRONTEND=noninteractive apt install -y --no-install-recommends \
        build-essential \
        cmake \
        ca-certificates \
        socat \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /tmini_plus_emulator
COPY . /tmini_plus_emulator

RUN mkdir -p linux/build \
    && cd linux/build \
    && cmake .. \
    && cmake --build . -- -j"$(nproc)"

WORKDIR /

CMD ["./tmini_plus_emulator/linux/build/tmini_plus_emulator", \
     "/tmp/ttyEMUL", \
     "./tmini_plus_emulator/data/example_data.txt", \
     "--stream"]
