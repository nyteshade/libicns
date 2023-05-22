#!/bin/sh

S="\x1b[1m"
R="\x1b[31m"
G="\x1b[32m"
Y="\x1b[33m"
B="\x1b[34m"
O="\x1b[0m"


if [[ "configure" = "${1}" ]]; then
  printf "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
  printf "${G}Autoconfiguring${O}...\x1b[s"
  printf "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
  autoconf -i -f 2>&1 >/dev/null
  printf "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n"
  printf "\x1b[u\x1b[7A${S}done${O}\n\x1b[5B"

  debugdef=""
  if [[ "${DEBUG}" = "true" ]]; then
    debugdef="-DICNS_DEBUG=1"
    printf "${G}Configuring${O} (${Y}debug${O})...\x1b[s"
  else
    printf "${G}Configuring${O}...\x1b[s"
  fi
  printf "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"

  CFLAGS="-std=c11 ${debugdef} -I/usr/local/include -I/opt/homebrew/include" ./configure --prefix=/usr/local >/dev/null
  printf "\x1b[u\x1b[1A${S}done${O}\n\n\n"
fi


function updateIconStamper() {
  cp -rpv /usr/local/include/icns.h /Volumes/Code/Personal/macOS/icon-stamper/NEIconTools/NEIconTools/libicns/include/
  cp -rpv /usr/local/lib/libicns.a /Volumes/Code/Personal/macOS/icon-stamper/NEIconTools/NEIconTools/libicns/lib/
}

make && make install && updateIconStamper

