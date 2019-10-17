UID=$(shell id -u)
GID=$(shell id -g)

dist:
	docker run -t --rm -v ${CURDIR}:/root/ChibiOS/demos/various/workspace aguegu/chibios sh -c "\
		make -f chibios.makefile \
		&& chown $(UID):$(GID) -R build .dep \
	"

clean:
	docker run -t --rm -v ${CURDIR}:/root/ChibiOS/demos/various/workspace aguegu/chibios make -f chibios.makefile clean

flash:
	st-flash --format ihex write build/ch.hex

all:
	make dist && make flash

.PHONY: dist clean flash all
