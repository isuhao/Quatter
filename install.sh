#!/bin/sh

./.installreq.sh
./.buildurho.sh

git pull
cd ..
mkdir Quatter-build
cd Quatter-build
qmake ../Quatter/Quatter.pro
sudo make install
sudo chown -R $USER ~/.local/share/luckey/quatter/
sudo chown $USER ~/.local/share/icons/quatter.svg
update-icon-caches ~/.local/share/icons/
cd ..
rm -rf Quatter-build
