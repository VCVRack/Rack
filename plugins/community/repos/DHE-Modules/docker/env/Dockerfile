FROM ubuntu:xenial
RUN apt-get update \
  && apt-get install -y --no-install-recommends software-properties-common \
  && add-apt-repository -y ppa:ubuntu-toolchain-r/test \
  && apt-get update \
  && apt-get install -y --no-install-recommends \
  autoconf \
  automake \
  cmake \
  curl \
  g++ \
  git \
  libasound2-dev \
  libgl1-mesa-dev \
  libglu1-mesa-dev \
  libgtk2.0-dev \
  libtool \
  libudev-dev \
  libxcursor-dev \
  libxinerama-dev\
  libxrandr-dev \
  make \
  tar \
  unzip \
  wget \
  zip \
  zlib1g-dev \
  && add-apt-repository -yr ppa:ubuntu-toolchain-r/test \
  && apt-get autoremove --purge \
  && apt-get clean
