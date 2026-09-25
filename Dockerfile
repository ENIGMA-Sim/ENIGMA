# syntax=docker/dockerfile:1
# ---------------------------------------------------------------------------
# ENIGMA simulator — ready-to-run image with SimGrid 4.1 (C++ + Python).
#
#   docker build -t enigma .
#   docker run --rm -it enigma                                   # shell in /enigma
#   docker run --rm -it -v "$PWD/out:/out" enigma \
#       bash -c "OUT_PREFIX=/out/uc7 ./use_cases/07_mobility_france_trains/run.sh"
# ---------------------------------------------------------------------------

ARG UBUNTU_VERSION=24.04

# ===========================================================================
# Stage 1 — build SimGrid 4.1 from source into /opt/simgrid-4.1
# ===========================================================================
FROM ubuntu:${UBUNTU_VERSION} AS simgrid-build

ARG SIMGRID_VERSION=4.1
ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates curl build-essential cmake \
        libboost-dev libboost-context-dev libboost-stacktrace-dev \
        python3 python3-dev pybind11-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
RUN curl -fsSL "https://framagit.org/simgrid/simgrid/-/archive/v${SIMGRID_VERSION}/simgrid-v${SIMGRID_VERSION}.tar.gz" \
        | tar xz --strip-components=1

# SMPI and the model checker are not used by ENIGMA: disabling them roughly
# halves the build time and the installed size.
RUN cmake -S . -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/opt/simgrid-${SIMGRID_VERSION} \
        -Denable_python=ON \
        -DSIMGRID_PYTHON_LIBDIR=/opt/simgrid-${SIMGRID_VERSION}/lib/python3/dist-packages \
        -Denable_smpi=OFF \
        -Denable_model-checking=OFF \
        -Denable_documentation=OFF \
    && cmake --build build -j"$(nproc)" \
    && cmake --install build

# ===========================================================================
# Stage 2 — runtime image: SimGrid + ENIGMA pre-built, toolchain kept so users
# can edit use_cases/*/*.cpp and recompile inside the container.
# ===========================================================================
FROM ubuntu:${UBUNTU_VERSION}

ARG SIMGRID_VERSION=4.1
ARG DEBIAN_FRONTEND=noninteractive

LABEL org.opencontainers.image.title="ENIGMA" \
      org.opencontainers.image.description="gENeric Iot edGe siMulAtor — Edge/Fog/Cloud simulation on SimGrid ${SIMGRID_VERSION}, with MQTT, mobility and energy modules" \
      org.opencontainers.image.source="https://github.com/enigma-sim/ENIGMA" \
      org.opencontainers.image.licenses="GPL-3.0"

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake \
        libboost-dev libboost-context-dev libboost-stacktrace-dev \
        python3 python3-pip \
    && pip3 install --no-cache-dir --break-system-packages folium xyzservices \
    && rm -rf /var/lib/apt/lists/*

COPY --from=simgrid-build /opt/simgrid-${SIMGRID_VERSION} /opt/simgrid-${SIMGRID_VERSION}

ENV SIMGRID_ROOT=/opt/simgrid-${SIMGRID_VERSION} \
    LD_LIBRARY_PATH=/opt/simgrid-${SIMGRID_VERSION}/lib \
    PYTHONPATH=/opt/simgrid-${SIMGRID_VERSION}/lib/python3/dist-packages:/enigma/src/python

# Fail the build early if the SimGrid library or its Python module is broken.
RUN ! ldd ${SIMGRID_ROOT}/lib/libsimgrid.so | grep -q "not found" \
    && python3 -c "import simgrid; print('SimGrid Python bindings OK')"

WORKDIR /enigma
COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=${SIMGRID_ROOT} \
    && cmake --build build -j"$(nproc)" \
    && chmod +x use_cases/*/run.sh build.sh run_examples.sh

VOLUME ["/out"]
CMD ["bash"]
