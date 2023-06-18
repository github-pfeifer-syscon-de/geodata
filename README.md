# geodata
To use the various weather/geo data services, for glglobe use this lib.

At the moment the following satellite image services are supported:
<ul>
  <li>RealEarth, with a excellent global coverage (we only use the global products)</li>
  <li>EumetSat, with some close to real time images</li>
  <li>Deutscher Wetterdienst, with a surprisingly wide coverage (but for some we lack the coordinate transform magic)</li>
</ul>
Additional services supporting WebMapService shoud be easy to add.<br>
Requirs genericImg&genericGlm so build&install there first.<br>
To build use any (lin)ux:
<pre>
autoreconf -fis
./configure ...
make
</pre>
For use needs to be installed:
<pre>
  make install
</pre>
For windows (get msys2 https://www.msys2.org/) the files shoud adapt use e.g.<br>
<pre>
  ./configure --prefix=/mingw64
</pre>

