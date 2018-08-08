
Debian
====================
This directory contains files used to package velesd/veles-qt
for Debian-based Linux systems. If you compile velesd/veles-qt yourself, there are some useful files here.

## veles: URI support ##


veles-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install veles-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your velesqt binary to `/usr/bin`
and the `../../share/pixmaps/veles128.png` to `/usr/share/pixmaps`

veles-qt.protocol (KDE)

