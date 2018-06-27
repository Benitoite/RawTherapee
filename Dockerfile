FROM debian:testing

#   add the dependencies

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends build-essential cmake git curl libcanberra-gtk3-dev libexiv2-dev libexpat-dev libfftw3-dev libglibmm-2.4-dev libgtk-3-dev libgtkmm-3.0-dev libiptcdata0-dev libjpeg-dev liblcms2-dev liblensfun-dev libpng-dev libsigc++-2.0-dev libtiff5-dev zlib1g-dev -y

#   set the entrypoint command

LABEL maintainer="kd6kxr@gmail.com"
CMD echo "This is a test..." && sh && echo "THATS ALL FOLKS!!!"
