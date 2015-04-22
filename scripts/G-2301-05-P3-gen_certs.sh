#!/bin/sh

openssl genrsa -out rootkey.pem 2048
openssl req -new -x509 -key rootkey.pem -out rootcert.pem
cat rootcert.pem rootkey.pem > root.pem
openssl x509 -subject -issuer -noout -in root.pem


openssl x509 -subject -issuer -noout -in client.pem
