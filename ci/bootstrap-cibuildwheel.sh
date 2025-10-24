#!/bin/sh

yum install -y flex ca-certificates || exit 1

echo "$ICS_IPA_CA_CRT" >/etc/pki/ca-trust/source/anchors/ica-ipa-ca.crt

update-ca-trust || exit 1
