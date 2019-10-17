VERSION := 17.6.4

image:
	docker build -t aguegu/chibios:${VERSION} -f ${VERSION}.Dockerfile .

push:
	docker push aguegu/chibios:${VERSION}

.PHONY: image push
