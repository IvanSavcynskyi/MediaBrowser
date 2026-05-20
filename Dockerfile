FROM node:22-bookworm

ENV VCPKG_ROOT=/opt/vcpkg
ENV VCPKG_DEFAULT_TRIPLET=x64-linux

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    bison \
    ca-certificates \
    cmake \
    curl \
    flex \
    git \
    ninja-build \
    perl \
    pkg-config \
    python3 \
    tar \
    unzip \
    zip \
    && rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/microsoft/vcpkg.git "$VCPKG_ROOT" \
    && "$VCPKG_ROOT/bootstrap-vcpkg.sh"

WORKDIR /app

COPY vcpkg.json ./
RUN "$VCPKG_ROOT/vcpkg" install --triplet "$VCPKG_DEFAULT_TRIPLET"

COPY frontend/package*.json ./frontend/
RUN cd frontend && npm ci

COPY . .

RUN cd frontend && npm run build

RUN mkdir -p logs

RUN cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DVCPKG_INSTALLED_DIR=/app/vcpkg_installed \
    -DVCPKG_MANIFEST_INSTALL=OFF \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
    && cmake --build build --config Release

EXPOSE 8080

CMD ["./build/MediaBrowser"]
