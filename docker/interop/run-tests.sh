#!/usr/bin/env bash
#
# Build and run the opendnp3 interop test Docker container.
#
# The FreyrSCADA DNP3 library is proprietary software.
# It is downloaded at Docker build time for interop testing ONLY.
#
# Usage:
#   ./docker/interop/run-tests.sh                  # run all tests
#   ./docker/interop/run-tests.sh --freyr-only     # run only FreyrSCADA interop tests
#   ./docker/interop/run-tests.sh --stepfunc-only  # run only stepfunc interop tests
#   ./docker/interop/run-tests.sh --unit-only      # run only unit tests
#   ./docker/interop/run-tests.sh --build-only     # build but don't run

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

IMAGE_NAME="opendnp3-interop"

echo "Building Docker image: ${IMAGE_NAME}"
docker build \
    -t "${IMAGE_NAME}" \
    -f "${SCRIPT_DIR}/Dockerfile" \
    "${REPO_ROOT}"

if [[ "${1:-}" == "--build-only" ]]; then
    echo "Build complete. Skipping test execution."
    exit 0
fi

# Determine which command to run
CMD=""
case "${1:-}" in
    --freyr-only)
        CMD="./freyr-interoptests --reporter compact"
        ;;
    --stepfunc-only)
        CMD="./interoptests --reporter compact"
        ;;
    --unit-only)
        CMD="./unittests --reporter compact"
        ;;
    --integration-only)
        CMD="./integrationtests --reporter compact"
        ;;
    *)
        # Run all (default CMD from Dockerfile)
        CMD=""
        ;;
esac

echo "Running tests..."
if [[ -n "${CMD}" ]]; then
    docker run --rm --network host "${IMAGE_NAME}" sh -c "${CMD}"
else
    docker run --rm --network host "${IMAGE_NAME}"
fi
