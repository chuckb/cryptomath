# ───────────────────────────────────────────────────────────
# LINUX DISTRO TARBALL
# ───────────────────────────────────────────────────────────

# Override on the command line, e.g.
#   make dist-linux LINUX_DISTRO=alpine:3.18 \
#                   LINUX_DEPS="build-base autoconf openssl-dev"
LINUX_DISTRO    ?= ubuntu:24.04
LINUX_DEPS      ?= build-essential autoconf libssl-dev

# turn "ubuntu:24.04" → "ubuntu-24.04"
LINUX_SAFE_TAG  := $(subst :,-,$(LINUX_DISTRO))
LINUX_IMG       := myproj-dist-$(LINUX_SAFE_TAG)

docker-image-linux:
	docker build \
	  --build-arg BASE_IMAGE=$(LINUX_DISTRO) \
	  --build-arg DEPS="$(LINUX_DEPS)" \
	  -t $(LINUX_IMG) \
	  -f Dockerfile.dist .

dist-linux: docker-image-linux
	docker run --rm \
	  -v "$(PWD)":/src \
	  -w /src \
	  $(LINUX_IMG)
