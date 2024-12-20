#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u
SPATH=`pwd`
OUTDIR=/tmp/aeld1
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    #make xconfig
    echo "mrproper 1\n"
    make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- mrproper
    echo "defconfig\n"
    make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- defconfig
    echo "make all\n"
    make -j30 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- all
    make -j30 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- modules
    make -j20 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- dtbs
fi

echo "Adding the Image in outdir"
cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}/

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make distclean
    make defconfig
else
    cd busybox
fi

# TODO: Make and install busybox

make -j10 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make -j10 CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

echo "Library dependencies"
${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs/bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs/bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs
SYSROOT=$(${CROSS_COMPILE}gcc -print-sysroot)
echo "sysroot is $SYSROOT"
mkdir -p "${OUTDIR}/rootfs/lib"
mkdir -p "${OUTDIR}/rootfs/lib64"
cp -a $SYSROOT/lib/ld-linux-aarch64.so.1 "${OUTDIR}/rootfs/lib/"
cp -a $SYSROOT/lib64/libm.so.6 "${OUTDIR}/rootfs/lib64/"
cp -a $SYSROOT/lib64/libresolv.so.2 "${OUTDIR}/rootfs/lib64/"
cp -a $SYSROOT/lib64/libc.so.6 "${OUTDIR}/rootfs/lib64/"

# TODO: Make device nodes
mkdir -p "${OUTDIR}/rootfs/dev"
cd "${OUTDIR}/rootfs"
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 600 dev/console c 5 1

# TODO: Clean and build the writer utility
cd ${SPATH}
make CROSS_COMPILE=${CROSS_COMPILE}
mkdir -p ${OUTDIR}/rootfs/home/conf
cp writer "${OUTDIR}/rootfs/usr/bin/."
cp finder.sh "${OUTDIR}/rootfs/usr/bin/."
cp finder-test.sh "${OUTDIR}/rootfs/home/."
cp conf/* ${OUTDIR}/rootfs/home/conf/.
cp autorun-qemu.sh ${OUTDIR}/rootfs/home/


# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs

# TODO: Chown the root directory
cd "${OUTDIR}/rootfs"
sudo chown -R root:root *
# TODO: Create initramfs.cpio.gz
find . | cpio -H newc -ov --owner root:root > "${OUTDIR}/initramfs.cpio"
gzip -f "${OUTDIR}/initramfs.cpio"
