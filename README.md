# geodata
To use the various weather/geo data services, for glglobe use a lib

For now uses: https://re-d.ssec.wisc.edu/ for weather map display.

To build use any (lin)ux:
<pre>
autoreconf -fis
./configure ...
make
</pre>
For windows (get msys2 https://www.msys2.org/) the files shoud adapt use e.g.<br>
<pre>
  ./configure --prefix=/mingw64
</pre>
For use needs to be installed:
<pre>
  make install
</pre>
