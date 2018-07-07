MacOS:

1. In first of two terminals:

`open -a XQuartz`

`socat TCP-LISTEN:6000,reuseaddr,fork UNIX-CLIENT:\"$DISPLAY\"`

2. In second terminal (*Replace `<your ip address>` with your ip address, for example 192.168.0.100):

`docker run -e DISPLAY=<your ip address>:0 -e QT_XKB_CONFIG_ROOT=/usr/share/X11/xkb -v /tmp/X11-unix=/tmp/X11-unix -v ~:/hi:private kd6kxr/rawtherapee-dev`

Linux:
* `docker run -it --rm -e DISPLAY=$DISPLAY  -v /tmp/.X11-unix:/tmp/.X11-unix -v ~:/hi:private kd6kxr/rawtherapee-dev`

Windows:
1. `"c:\Program Files (x86)\Xming\Xming.exe" :0 -ac -clipboard -multiwindow`
2. `docker run -it -e DISPLAY=192.168.0.3:0 -v ~:/hi:private kd6kxr/rawtherapee-dev`
*Replace 192.168.0.3 with your ip address.

* Mounts your home directory to `/hi` and starts up rawtherapee-dev application

![logo](https://kd6kxr.us/rawtherapee-dev.png)

RawTherapee is a powerful, cross-platform raw photo processing program, released as [libre software](https://en.wikipedia.org/wiki/Free_software) under the [GNU General Public License Version 3](https://opensource.org/licenses/gpl-3.0.html). It is written mostly in C++ using a [GTK+](http://www.gtk.org/) front-end. It uses a patched version of [dcraw](http://www.cybercom.net/~dcoffin/dcraw/) for reading raw files, with an in-house solution which adds the highest quality support for certain camera models unsupported by dcraw and enhances the accuracy of certain raw files already supported by dcraw. It is notable for the advanced control it gives the user over the demosaicing and development process.

## Target audience

RawTherapee is designed for developing raw files from a broad range of digital cameras, as well as [HDR DNG](https://helpx.adobe.com/photoshop/digital-negative.html) files and non-raw image formats ([JPEG](https://en.wikipedia.org/wiki/JPEG), [TIFF](https://en.wikipedia.org/wiki/Tagged_Image_File_Format) and [PNG](https://en.wikipedia.org/wiki/Portable_Network_Graphics)). The target audience ranges from enthusiast newcomers who wish to broaden their understanding of how digital imaging works to semi-professional photographers. Knowledge in color science is not compulsory, but it is recommended that you are eager to learn and ready to read our documentation ([RawPedia](http://rawpedia.rawtherapee.com/)) as well as look up basic concepts which lie outside the scope of RawPedia, such as [color balance](https://en.wikipedia.org/wiki/Color_balance), elsewhere.

Of course, professionals may use RawTherapee too while enjoying complete freedom, but will probably lack some peripheral features such as [Digital Asset Management](https://en.wikipedia.org/wiki/Digital_asset_management), printing, uploading, etc. RawTherapee is not aimed at being an inclusive all-in-one program, and the [open-source community](https://en.wikipedia.org/wiki/Open-source_movement) is sufficiently developed by now to offer all those peripheral features in other specialized software.

## Links

Website:
http://rawtherapee.com/

Forum:
https://discuss.pixls.us/c/software/rawtherapee

Features:
http://rawpedia.rawtherapee.com/Features

Official documentation:
http://rawpedia.rawtherapee.com/

Download RawTherapee:
http://rawtherapee.com/downloads

Download source code tarballs:
http://rawtherapee.com/shared/source/

## Compilation, branches and Git
Refer to RawPedia for a detailed explanation of how to get the necessary dependencies and how to compile RawTherapee.

Linux:
http://rawpedia.rawtherapee.com/Linux

Windows:
http://rawpedia.rawtherapee.com/Windows

macOS:
http://rawpedia.rawtherapee.com/macOS
