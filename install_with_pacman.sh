#!/bin/sh

sudo pacman -S cmake libx11 libxrandr alsa-lib
./.buildurho.sh

git pull
cd ..
mkdir Quatter-build
cd Quatter-build
qmake ../Quatter/Quatter.pro
sudo make install
sudo chown -R $USER ~/.local/share/luckey/quatter/
sudo chown $USER ~/.local/share/icons/quatter.svg
cd ..
rm -rf Quatter-build
