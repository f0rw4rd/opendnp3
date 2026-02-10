# Multi-stage Docker build for opendnp3 Python wheel (Linux x86_64)
#
# Produces a per-Python-version wheel with the pybind11 native extension.
# The extension is ABI-specific (cpXY), not universal.

ARG PYTHON_VERSION=3.12
FROM python:${PYTHON_VERSION}-slim-bookworm AS builder

ARG PACKAGE_VERSION=3.1.2.1

# Install build dependencies
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        git \
        build-essential \
        cmake \
        libssl-dev \
        python3-dev \
    && rm -rf /var/lib/apt/lists/*

# Copy the full source tree
WORKDIR /build/opendnp3
COPY . .

# Build the C++ library and pybind11 extension
RUN mkdir -p cmake-build && \
    cd cmake-build && \
    cmake \
        -DDNP3_PYTHON=ON \
        -DDNP3_STATIC_LIBS=ON \
        -DDNP3_TLS=ON \
        -DDNP3_TESTS=OFF \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
        .. && \
    cmake --build . --parallel $(nproc)

# Create Python package directory for wheel building
WORKDIR /build/wheel-staging
RUN mkdir -p opendnp3

# Copy the compiled pybind11 extension (.so) into the package
RUN cp /build/opendnp3/cmake-build/python/_opendnp3*.so opendnp3/ && \
    echo "=== Extension files ===" && \
    ls -la opendnp3/*.so

# Copy __init__.py
COPY opendnp3/__init__.py opendnp3/__init__.py

# Copy project metadata files
COPY setup_wheel.py setup.py
COPY README.md LICENSE NOTICE ./

# Build the wheel
RUN pip install --no-cache-dir setuptools wheel && \
    PACKAGE_VERSION=${PACKAGE_VERSION} python setup.py bdist_wheel && \
    echo "=== Wheel contents ===" && \
    python -c "import zipfile, sys; [print(f.filename) for f in zipfile.ZipFile(sys.argv[1]).filelist]" dist/*.whl

# Generate SHA256 checksums
RUN cd dist && sha256sum *.whl > SHA256SUMS

# Test stage: install the wheel and verify it imports
FROM python:${PYTHON_VERSION}-slim-bookworm AS tester

RUN apt-get update && \
    apt-get install -y --no-install-recommends libssl3 && \
    rm -rf /var/lib/apt/lists/*

COPY --from=builder /build/wheel-staging/dist/*.whl /tmp/wheels/
RUN pip install /tmp/wheels/*.whl && \
    python -c "import opendnp3; print('opendnp3 import OK')" && \
    python -c "import opendnp3; print(f'DNP3Manager: {opendnp3.DNP3Manager}')" && \
    python -c "import opendnp3; m = opendnp3.DNP3Manager(1); m.Shutdown(); print('Manager lifecycle OK')" && \
    echo "All import tests passed"

# Final output stage
FROM python:${PYTHON_VERSION}-slim-bookworm
WORKDIR /wheels
COPY --from=builder /build/wheel-staging/dist/*.whl /wheels/
COPY --from=builder /build/wheel-staging/dist/SHA256SUMS /wheels/
