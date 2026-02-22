# geodata
To use the various weather/geo data services, for glglobe use this lib.

At the moment the following satellite image services are supported:
<ul>
  <li>RealEarth, with a excellent global coverage (we only use the global products)</li>
  <li>EumetSat, with some close to real time images</li>
  <li>Deutscher Wetterdienst, with a surprisingly wide coverage (but for some we lack the coordinate transform magic)</li>
</ul>
Additional services supporting WebMapService shoud be easy to add.

## Build
Requirs genericImg&genericGlm so build&install these first.

### Linux
Example for Debian install prerequisites
<pre>
apt-get install libsoup-3.0-dev
</pre>
To build use any (lin)ux:
<pre>
meson setup build -Dprefix=/usr
cd build
meson compile
</pre>

### Windows
For windows (get msys2 https://www.msys2.org/) the files shoud adapt use e.g.
<pre>
  pacman -S ${MINGW_PACKAGE_PREFIX}-libsoup3
  meson setup build -Dprefix=${MINGW_PREFIX}
  ...
</pre>

### Any platform
For use needs to be installed (for linux run as root):
<pre>
meson install
</pre>
