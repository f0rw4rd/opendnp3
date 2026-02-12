# Dockerfile for opendnp3 interoperability tests against FreyrSCADA DNP3.
#
# The FreyrSCADA DNP3 library is PROPRIETARY software owned by
# FreyrSCADA Embedded Solution Pvt Ltd.  It is NOT open source.
# This Dockerfile downloads it at build time solely for the purpose
# of interoperability testing.  The binary is NOT redistributed.
#
# Usage:
#   docker build -t opendnp3-interop -f docker/interop/Dockerfile .
#   docker run --rm opendnp3-interop
#
# The container runs both the existing integration tests and the
# FreyrSCADA interop tests.

FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        g++ \
        git \
        libssl-dev \
        unzip \
        wget \
        ca-certificates \
        pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Download FreyrSCADA Linux SDK
# NOTE: This is proprietary software used ONLY for interop testing.
RUN mkdir -p /opt/freyr-dnp3 && \
    wget -q -O /tmp/DNP3-LinuxSDK.zip \
        "https://github.com/FreyrSCADA/DNP3/raw/master/DNP3-Linux-SDK/DNP3-LinuxSDK.zip" && \
    unzip -o /tmp/DNP3-LinuxSDK.zip -d /opt/freyr-dnp3/ && \
    rm /tmp/DNP3-LinuxSDK.zip

# Copy opendnp3 source
COPY . /src/opendnp3
WORKDIR /src/opendnp3

# Build opendnp3 with tests and FreyrSCADA interop tests
RUN mkdir -p /src/opendnp3/build && \
    cd /src/opendnp3/build && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DDNP3_TESTS=ON \
        -DDNP3_STATIC_LIBS=ON \
        -DDNP3_TLS=ON \
        -DDNP3_INTEROP_TESTS=ON \
        -DDNP3_FREYR_INTEROP_TESTS=ON \
        -DFREYRSCADA_DNP3_ROOT=/opt/freyr-dnp3/LinuxSDK/x86_64 \
        .. && \
    cmake --build . --parallel $(nproc)

# -----------------------------------------------------------------
# Runtime stage: slim image that just runs the tests
# -----------------------------------------------------------------
FROM ubuntu:22.04 AS runner

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        libstdc++6 \
        libssl3 \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /src/opendnp3/build/cpp/tests/unit/unittests                    /opt/tests/unittests
COPY --from=builder /src/opendnp3/build/cpp/tests/integration/integrationtests      /opt/tests/integrationtests
COPY --from=builder /src/opendnp3/build/cpp/tests/interop/interoptests              /opt/tests/interoptests
COPY --from=builder /src/opendnp3/build/cpp/tests/interop-freyr/freyr-interoptests  /opt/tests/freyr-interoptests

# Copy the stepfunc/dnp3 shared library needed at runtime
COPY --from=builder /src/opendnp3/build/_deps/stepfunc_dnp3-src/lib/x86_64-unknown-linux-gnu/libdnp3_ffi.so /opt/tests/lib/

# Copy TLS certificates to the path compiled into the interoptests binary
COPY --from=builder /src/opendnp3/build/cpp/tests/interop/certs/ /src/opendnp3/build/cpp/tests/interop/certs/

WORKDIR /opt/tests

ENV LD_LIBRARY_PATH=/opt/tests/lib

# By default, run all tests. Override CMD to run specific tests.
CMD ["sh", "-c", "\
    echo '=== Running unit tests ===' && \
    ./unittests --reporter compact && \
    echo '' && \
    echo '=== Running integration tests ===' && \
    ./integrationtests --reporter compact && \
    echo '' && \
    echo '=== Running stepfunc interop tests ===' && \
    ./interoptests --reporter compact && \
    echo '' && \
    echo '=== Running FreyrSCADA interop tests ===' && \
    ./freyr-interoptests --reporter compact && \
    echo '' && \
    echo '=== All tests passed ===' \
"]
