#!/usr/bin/sh

if [ $UID -ne 0 ];then
#    print_warning "非root用户没有权限安装"
    exit 1
fi

sudo rm -f /usr/bin/ukui-settings-daemon
sudo rm -fr /usr/local/lib/ukui-settings-daemon/*
make clean
/opt/Qt5.14.0/5.14.0/gcc_64/bin/qmake \
       /home/dingjing/code/ukui-settings-daemon/ukui-settings-daemon.pro \
       -spec linux-g++ CONFIG+=debug CONFIG+=qml_debug

sudo /usr/bin/make qmake_all

sudo make install
