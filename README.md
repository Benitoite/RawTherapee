# ART: Another RawTherapee

![Another RawTherapee](https://bitbucket.org/kd6kxr/art/raw/ce7537c3f720bb23ed794bd841d30252c75365d7/rtdata/images/png/ART-logo-64.png)


## Docker Image Instructions:
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

<hr>

# A(nother) RT

ART is a fork of RawTherapee, a powerful cross-platform raw photo processing program. **This is not an official RawTherapee repository!**, and there is **no support whatsoever** for this version. [ART Code Repository](https:/bitbucket.org/agriggio/ART)

If you are looking for the RawTherapee code, please go to the official repo at [GitHub](https://github.com/Beep6581/RawTherapee).
