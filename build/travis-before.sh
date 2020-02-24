#!/bin/sh -xe

cd $(dirname $0)/..

apt-get update
apt-get dist-upgrade -y
apt-get install -y automake gettext libpopt-dev libglib2.0-dev libncursesw5-dev tcl8.6-dev libxml2-dev libconfig-dev screen tmux xxd
