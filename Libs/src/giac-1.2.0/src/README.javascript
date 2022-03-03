Running javscript programs with giac CAS support under linux
1/ Install giac-1.1: add the line
deb http://www-fourier.ujf-grenoble.fr/~parisse/debian/ testing main
in the file /etc/apt/sources.list. Then run
  sudo apt-get update
Now you can install or upgrade by running
  sudo apt-get install giac 

2/ Install webkit
  sudo apt-get install libwebkit-dev

3/ Copy libgiacjs.so in /usr/lib and run sudo ldconfig or run 
  export LD_LIBRARY_PATH=.

4/ Run
  ./javascript -l giacjs giacjs.js
(replace giacjs.js with your javascript)
If it does not work, run ldd javascript and make a link in /usr/lib like this
cd /usr/lib ; sudo ln -s libwebkitgtk-1.0.so.0 libwebkit-1.0.so.2

For details how to make the interface, have a look at giacjs.i
