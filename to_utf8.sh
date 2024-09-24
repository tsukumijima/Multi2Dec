#!/bin/bash

echo "Convert from CP932 to UTF-8(BOM) ..."

for f in BonSDK/Include/*.h B25Decoder/BonProject/*.{cpp,h} B25Decoder/B25Decoder.{cpp,h} B25Decoder/IB25Decoder.h B25Decoder/stdafx.{cpp,h}; do
  if [ "$(head -c 3 $f)" != "$(echo -e "\xef\xbb\xbf")" ]; then
    echo "$f"
    echo -en "\xef\xbb\xbf" > $f.tmp
    iconv -f CP932 -t UTF-8 $f >> $f.tmp
    mv $f.tmp $f
  else
    echo "Skip $f"
  fi
done

echo "Done."
