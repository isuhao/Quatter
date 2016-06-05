sudo apt-get install libx11-dev libxrandr-dev libasound2-dev git cmake make qmake

cd `dirname $0`
if [ ! -d Urho3D ]
then
  git clone https://github.com/Urho3D/Urho3D
fi

cd Urho3D
git pull
cmake .
make
cd ..

git pull
qmake Quatter.pro
sudo make uninstall
sudo make install