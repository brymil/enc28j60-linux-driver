#!/bin/bash

function mk_overlay {
	name=${1}
	INC="/usr/src/linux-headers-$(uname -r)/include/"
	$(cpp -nostdinc -I ${INC} -undef -x assembler-with-cpp ${name}.dts ${name}.dts.preprocessed)
	dtc -i ${INC} -I dts "${name}.dts.preprocessed" -O dtb -o "${name}.dtbo"
	chmod +x "${name}.dtbo"
	rm "/boot/overlays/${name}.dtbo"
	cp "${name}.dtbo" "/boot/overlays"
}
mk_overlay ${1}
