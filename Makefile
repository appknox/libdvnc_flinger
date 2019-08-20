all: build
this_dir = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SOURCES := Android.mk include/flinger.h include/screenFormat.h
BASE := $(this_dir)aosp

build: \
	libs/android-25/armeabi-v7a/libdvnc_flinger_sdk.so \
	libs/android-25/arm64-v8a/libdvnc_flinger_sdk.so \
	libs/android-26/armeabi-v7a/libdvnc_flinger_sdk.so \
	libs/android-26/arm64-v8a/libdvnc_flinger_sdk.so \
	libs/android-27/armeabi-v7a/libdvnc_flinger_sdk.so \
	libs/android-27/arm64-v8a/libdvnc_flinger_sdk.so \
	libs/android-28/armeabi-v7a/libdvnc_flinger_sdk.so \
	libs/android-28/arm64-v8a/libdvnc_flinger_sdk.so \

libs/android-25/armeabi-v7a/libdvnc_flinger_sdk.so: $(SOURCES) src/flinger.cpp
	mkdir -p $(@D)
	docker run --rm \
		-a stdout -a stderr \
		-v $(BASE)/android-7.1.0_r1:/aosp \
		-v $(this_dir):/app \
		-v $(this_dir)$(@D):/artifacts \
		openstf/aosp:jdk8 /aosp.sh build aosp_arm-eng libdvnc_flinger_sdk

libs/android-25/arm64-v8a/libdvnc_flinger_sdk.so: $(SOURCES) src/flinger.cpp
	mkdir -p $(@D)
	docker run --rm \
		-a stdout -a stderr \
		-v $(BASE)/android-7.1.0_r1:/aosp \
		-v $(this_dir):/app \
		-v $(this_dir)$(@D):/artifacts \
		openstf/aosp:jdk8 /aosp.sh build aosp_arm64-eng libdvnc_flinger_sdk

libs/android-26/armeabi-v7a/libdvnc_flinger_sdk.so: $(SOURCES) src/flinger.cpp
	mkdir -p $(@D)
	docker run --rm \
		-a stdout -a stderr \
		-v $(BASE)/android-8.0.0_r1:/aosp \
		-v $(this_dir):/app \
		-v $(this_dir)$(@D):/artifacts \
		openstf/aosp:jdk8 /aosp.sh build aosp_arm-eng libdvnc_flinger_sdk

libs/android-26/arm64-v8a/libdvnc_flinger_sdk.so: $(SOURCES) src/flinger.cpp
	mkdir -p $(@D)
	docker run --rm \
		-a stdout -a stderr \
		-v $(BASE)/android-8.0.0_r1:/aosp \
		-v $(this_dir):/app \
		-v $(this_dir)$(@D):/artifacts \
		openstf/aosp:jdk8 /aosp.sh build aosp_arm64-eng libdvnc_flinger_sdk

libs/android-27/armeabi-v7a/libdvnc_flinger_sdk.so: $(SOURCES) src/flinger.cpp
	mkdir -p $(@D)
	docker run --rm \
		-a stdout -a stderr \
		-v $(BASE)/android-8.1.0_r1:/aosp \
		-v $(this_dir):/app \
		-v $(this_dir)$(@D):/artifacts \
		openstf/aosp:jdk8 /aosp.sh build aosp_arm-eng libdvnc_flinger_sdk

libs/android-27/arm64-v8a/libdvnc_flinger_sdk.so: $(SOURCES) src/flinger.cpp
	mkdir -p $(@D)
	docker run --rm \
		-a stdout -a stderr \
		-v $(BASE)/android-8.1.0_r1:/aosp \
		-v $(this_dir):/app \
		-v $(this_dir)$(@D):/artifacts \
		openstf/aosp:jdk8 /aosp.sh build aosp_arm64-eng libdvnc_flinger_sdk

libs/android-28/armeabi-v7a/libdvnc_flinger_sdk.so: $(SOURCES) src/flinger.cpp
	mkdir -p $(@D)
	docker run --rm \
		-a stdout -a stderr \
		-v $(BASE)/android-9.0.0_r1:/aosp \
		-v $(this_dir):/app \
		-v $(this_dir)$(@D):/artifacts \
		openstf/aosp:jdk8 /aosp.sh build aosp_arm-eng libdvnc_flinger_sdk

libs/android-28/arm64-v8a/libdvnc_flinger_sdk.so: $(SOURCES) src/flinger.cpp
	mkdir -p $(@D)
	docker run --rm \
		-a stdout -a stderr \
		-v $(BASE)/android-9.0.0_r1:/aosp \
		-v $(this_dir):/app \
		-v $(this_dir)$(@D):/artifacts \
		openstf/aosp:jdk8 /aosp.sh build aosp_arm64-eng libdvnc_flinger_sdk

setup:
	mkdir -p aosp/mirror
	docker run -ti --rm -v $(BASE)/mirror:/mirror openstf/aosp:jdk8 /aosp.sh create-mirror
	mkdir -p aosp/android-7.1.0_r1
	docker run -ti -v $(BASE)/mirror:/mirror -v $(BASE)/android-7.1.0_r1:/aosp openstf/aosp:jdk8 /aosp.sh checkout-branch android-7.1.0_r1
	mkdir -p aosp/android-8.0.0_r1
	docker run -ti -v $(BASE)/mirror:/mirror -v $(BASE)/android-8.0.0_r1:/aosp openstf/aosp:jdk8 /aosp.sh checkout-branch android-8.0.0_r1
	mkdir -p aosp/android-8.1.0_r1
	docker run -ti -v $(BASE)/mirror:/mirror -v $(BASE)/android-8.1.0_r1:/aosp openstf/aosp:jdk8 /aosp.sh checkout-branch android-8.1.0_r1
	mkdir -p aosp/android-9.0.0_r1
	docker run -ti -v $(BASE)/mirror:/mirror -v $(BASE)/android-9.0.0_r1:/aosp openstf/aosp:jdk8 /aosp.sh checkout-branch android-9.0.0_r1