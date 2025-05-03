FROM raetro/quartus:13.1 AS intelFPGA

FROM mcr.microsoft.com/devcontainers/base:debian AS base

COPY --from=intelFPGA /opt/intelFPGA/ /opt/intelFPGA/

ENV PATH=/opt/intelFPGA/nios2eds/bin/gnu/H-i686-pc-linux-gnu/bin:$PATH

RUN dpkg --add-architecture i386

RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
     && apt-get -y install --no-install-recommends \
     ca-certificates \
     make \
     git \
     qemu-system-nios2 \
     qemu-user-static \
     libc6:i386 \
     libstdc++6:i386 \
     libsm6:i386 \
     libxext6:i386 \
     libxft2:i386

RUN apt-get clean && rm -rf /var/cache/apt/* && rm -rf /var/lib/apt/lists/* && rm -rf /tmp/*