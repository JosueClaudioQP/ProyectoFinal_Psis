#!/bin/bash

echo "======================================="
echo " Instalando dependencias..."
echo "======================================="

sudo apt update

sudo apt install -y \
build-essential \
gcc \
make \
gdb \
git \
tree \
procps \
psmisc \
rsync \
curl \
wget \
zip \
unzip

echo
echo "======================================="
echo " Dependencias instaladas correctamente."
echo "======================================="
