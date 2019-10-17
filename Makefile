builder:
	docker build -t aguegu/chibios:17.6.4 .

push:
	docker push aguegu/chibios:17.6.4

.PHONY: builder push
