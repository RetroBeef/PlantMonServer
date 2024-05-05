#!/bin/bash

openssl req -x509 -out server.crt -keyout private.key \
  -newkey rsa:2048 -nodes -sha256 \
  -subj '/CN=localhost' -extensions EXT -config <( \
   printf "[dn]\nCN=localhost\n[req]\ndistinguished_name = dn\n[EXT]\nsubjectAltName=DNS:localhost\nkeyUsage=digitalSignature\nextendedKeyUsage=serverAuth")

mv -f server.crt ../build/server.crt
mv -f private.key ../build/private.key

echo "Certificate and key generated successfully."

