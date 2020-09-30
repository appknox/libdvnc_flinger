.PHONY: all
all: build
this_dir = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SOURCES := Android.mk include/flinger.h include/screenFormat.h include/ScreenFrame.h src/flinger.cpp
BASE := $(this_dir)aosp

.PHONY: build
build: \
	libs/android-26/armeabi-v7a/libdvnc_flinger.so \
	libs/android-26/arm64-v8a/libdvnc_flinger.so \
	libs/android-27/armeabi-v7a/libdvnc_flinger.so \
	libs/android-27/arm64-v8a/libdvnc_flinger.so \
	libs/android-28/armeabi-v7a/libdvnc_flinger.so \
	libs/android-28/arm64-v8a/libdvnc_flinger.so \
	libs/android-29/arm64-v8a/libdvnc_flinger.so \

libs/android-26/armeabi-v7a/libdvnc_flinger.so: $(SOURCES) src/ScreenFrame26.cpp
	mkdir -p $(@D)
	docker run --rm \
		-a stdout -a stderr \
		-v $(BASE)/android-8.0.0_r1:/aosp \
		-v $(this_dir):/app \
		-v $(this_dir)$(@D):/artifacts \
		openstf/aosp:jdk8 /aosp.sh build aosp_arm-eng libdvnc_flinger

libs/android-26/arm64-v8a/libdvnc_flinger.so: $(SOURCES) src/ScreenFrame26.cpp
	mkdir -p $(@D)
	docker run --rm \
		-a stdout -a stderr \
		-v $(BASE)/android-8.0.0_r1:/aosp \
		-v $(this_dir):/app \
		-v $(this_dir)$(@D):/artifacts \
		openstf/aosp:jdk8 /aosp.sh build aosp_arm64-eng libdvnc_flinger

libs/android-27/armeabi-v7a/libdvnc_flinger.so: $(SOURCES) src/ScreenFrame27.cpp
	mkdir -p $(@D)
	docker run --rm \
		-a stdout -a stderr \
		-v $(BASE)/android-8.1.0_r1:/aosp \
		-v $(this_dir):/app \
		-v $(this_dir)$(@D):/artifacts \
		openstf/aosp:jdk8 /aosp.sh build aosp_arm-eng libdvnc_flinger

libs/android-27/arm64-v8a/libdvnc_flinger.so: $(SOURCES) src/ScreenFrame27.cpp
	mkdir -p $(@D)
	docker run --rm \
		-a stdout -a stderr \
		-v $(BASE)/android-8.1.0_r1:/aosp \
		-v $(this_dir):/app \
		-v $(this_dir)$(@D):/artifacts \
		openstf/aosp:jdk8 /aosp.sh build aosp_arm64-eng libdvnc_flinger

libs/android-28/armeabi-v7a/libdvnc_flinger.so: $(SOURCES) src/ScreenFrame28.cpp
	mkdir -p $(@D)
	docker run --rm \
		-a stdout -a stderr \
		-v $(BASE)/android-9.0.0_r1:/aosp \
		-v $(this_dir):/app \
		-v $(this_dir)$(@D):/artifacts \
		openstf/aosp:jdk8 /aosp.sh build aosp_arm-eng libdvnc_flinger

libs/android-28/arm64-v8a/libdvnc_flinger.so: $(SOURCES) src/ScreenFrame28.cpp
	mkdir -p $(@D)
	docker run --rm \
		-a stdout -a stderr \
		-v $(BASE)/android-9.0.0_r1:/aosp \
		-v $(this_dir):/app \
		-v $(this_dir)$(@D):/artifacts \
		openstf/aosp:jdk8 /aosp.sh build aosp_arm64-eng libdvnc_flinger

libs/android-29/arm64-v8a/libdvnc_flinger.so: $(SOURCES) src/ScreenFrame29.cpp
	mkdir -p $(@D)
	docker run --rm \
		-a stdout -a stderr \
		-v $(BASE)/android10-release:/aosp \
		-v $(this_dir):/app \
		-v $(this_dir)$(@D):/artifacts \
		openstf/aosp:jdk8 /aosp.sh build aosp_arm64-eng libdvnc_flinger

.PHONY: setup
setup:
	mkdir -p aosp/mirror
	docker run -ti --rm -v $(BASE)/mirror:/mirror openstf/aosp:jdk8 /aosp.sh create-mirror
	mkdir -p aosp/android-8.0.0_r1
	docker run -ti -v $(BASE)/mirror:/mirror -v $(BASE)/android-8.0.0_r1:/aosp openstf/aosp:jdk8 /aosp.sh checkout-branch android-8.0.0_r1
	mkdir -p aosp/android-8.1.0_r1
	docker run -ti -v $(BASE)/mirror:/mirror -v $(BASE)/android-8.1.0_r1:/aosp openstf/aosp:jdk8 /aosp.sh checkout-branch android-8.1.0_r1
	mkdir -p aosp/android-9.0.0_r1
	docker run -ti -v $(BASE)/mirror:/mirror -v $(BASE)/android-9.0.0_r1:/aosp openstf/aosp:jdk8 /aosp.sh checkout-branch android-9.0.0_r1
	mkdir -p aosp/android10-release
	docker run -ti -v $(BASE)/mirror:/mirror -v $(BASE)/android10-release:/aosp openstf/aosp:jdk8 /aosp.sh checkout-branch android10-release
.PHONY: copy
copy: \
	upload_assets/libdvnc_flinger_sdk26_armabi-v7a.so \
	upload_assets/libdvnc_flinger_sdk26_arm64-v8a.so \
	upload_assets/libdvnc_flinger_sdk27_armabi-v7a.so \
	upload_assets/libdvnc_flinger_sdk27_arm64-v8a.so \
	upload_assets/libdvnc_flinger_sdk28_armabi-v7a.so \
	upload_assets/libdvnc_flinger_sdk28_arm64-v8a.so \
	upload_assets/libdvnc_flinger_sdk29_arm64-v8a.so \

upload_assets/libdvnc_flinger_sdk%_armabi-v7a.so: libs/android-%/armeabi-v7a/libdvnc_flinger.so
	mkdir -p upload_assets
	cp -r $< $@

upload_assets/libdvnc_flinger_sdk%_arm64-v8a.so: libs/android-%/arm64-v8a/libdvnc_flinger.so
	mkdir -p upload_assets
	cp -r $< $@
