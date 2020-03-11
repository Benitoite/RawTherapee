MacOS:

1. In first of two terminals:

`open -a XQuartz`

`socat TCP-LISTEN:6000,bind=$(ifconfig -a|tail +9|grep 'inet '|cut -d ' ' -f 2),reuseaddr,fork UNIX-CLIENT:\"$DISPLAY\"`

2. In second terminal:

`docker run -e DISPLAY=$(ifconfig -a|tail +9|grep 'inet '|tail -c +6|cut -d ' ' -f 2):0 -e QT_XKB_CONFIG_ROOT=/usr/share/X11/xkb -v /tmp/X11-unix=/tmp/X11-unix -v ~:/hi:private kd6kxr/art1-master`

Linux:
* `docker run -it --rm -e DISPLAY=$DISPLAY  -v /tmp/.X11-unix:/tmp/.X11-unix -v ~:/hi:private kd6kxr/art1-master`

Windows:
1. `"c:\Program Files (x86)\Xming\Xming.exe" :0 -ac -clipboard -multiwindow`
2. `docker run -it -e DISPLAY=192.168.0.3:0 -v ~:/hi:private kd6kxr/art1-master`
*Replace 192.168.0.3 with your ip address.

* Mounts your home directory to `/hi` and starts up the ART application
