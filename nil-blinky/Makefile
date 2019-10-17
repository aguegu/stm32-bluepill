UID = $(shell id -u)
GID = $(shell id -g)
CHIBIOS_VERSION = 19.1.3

dist:
	docker run -t --rm -v ${CURDIR}:/root/ChibiOS/demos/various/workspace aguegu/chibios:${CHIBIOS_VERSION} sh -c "\
		make -f chibios.Makefile \
		&& chown $(UID):$(GID) -R build .dep \
	"

clean:
	docker run -t --rm -v ${CURDIR}:/root/ChibiOS/demos/various/workspace aguegu/chibios:${CHIBIOS_VERSION} make -f chibios.Makefile clean

flash:
	st-flash --format ihex write build/ch.hex

all:
	make dist && make flash

.PHONY: dist clean flash all
