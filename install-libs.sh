set -e

WIICURL='https://github.com/AndrewPiroli/wii-curl/releases/download/c8.12.1%2Bm3.6.2/wii-curl-8.12.1-1-any.pkg.tar.gz'
WIISOCKET='https://github.com/AndrewPiroli/wii-curl/releases/download/c8.12.1%2Bm3.6.2/libwiisocket-0.1-1-any.pkg.tar.gz'
WIIMBEDTLS='https://github.com/AndrewPiroli/wii-curl/releases/download/c8.12.1%2Bm3.6.2/wii-mbedtls-3.6.2-2-any.pkg.tar.gz'

mkdir /tmp/libs-temp-install
cd /tmp/libs-temp-install

wget $WIICURL
wget $WIISOCKET
wget $WIIMBEDTLS

tar -xvf wii-curl-8.12.1-1-any.pkg.tar.gz
tar -xvf wii-mbedtls-3.6.2-2-any.pkg.tar.gz
tar -xvf libwiisocket-0.1-1-any.pkg.tar.gz

rsync -av --remove-source-files opt/devkitpro/ /opt/devkitpro/
