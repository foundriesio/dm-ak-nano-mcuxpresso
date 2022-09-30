#!/bin/bash

#
# Helper script to build, sign and publish OTA binaries
#

set -e # exit on errors

mcuboot_path="${HOME}/nxp_sdk/SDK_2_11_1_MIMXRT1170-EVK/middleware/mcuboot_opensource/"
fioctl_path="${HOME}/git_foundries/fioctl/bin"

[ -n "${MCUBOOT_PATH}" ] && mcuboot_path=${MCUBOOT_PATH}
[ -n "${FIOCTL_PATH}" ] && fioctl_path=${FIOCTL_PATH}

COLOR_RED="\e[31m"
COLOR_GREEN="\e[32m"
COLOR_YELLOW="\e[33m"
COLOR_BLUE="\e[34m"
COLOR_MAGENTA="\e[35m"
COLOR_CYAN="\e[36m"
COLOR_RESET="\e[0m"

DO_FLASH=0
DO_FLASH_PRIMARY_SLOT=0
DO_FLASH_SECONDARY_SLOT=0
DO_FORCE_CLEANUP=0 # not effective yet

# Default RT1170
BUILD_RT1060=0
FLASH_SLOT1_ADDRESS=0x30040000
FLASH_SLOT2_ADDRESS=0x30240000
BUILD_HWID="MIMXRT1170-EVK"
export BOARD_MODEL="rt1170"

DO_PUBLISH=1
PUBLISH_TAGS="devel"

DO_BREAK_SIGNATURE=0
DO_TEST_BUILD=0

for i in "$@"; do
  case $i in
    -f1|--flash1)
      DO_FLASH=1
      DO_FLASH_PRIMARY_SLOT=1
      shift # past argument
      ;;
    -f2|--flash2)
      DO_FLASH=1
      DO_FLASH_SECONDARY_SLOT=1
      shift # past argument
      ;;
    -P|--nopublish)
      DO_PUBLISH=0
      shift # past argument
      ;;
    --tags=*)
      PUBLISH_TAGS="${i#*=}"
      shift # past argument=value
      ;;
    --rt1060)
      BUILD_RT1060=1
      FLASH_SLOT1_ADDRESS=0x60040000
      FLASH_SLOT2_ADDRESS=0x60240000
      export BOARD_MODEL="rt1060"
      BUILD_HWID="MIMXRT1060-EVK"
      shift # past argument
      ;;
    --force-cleanjup)
      DO_FORCE_CLEANUP=1
      shift # past argument
      ;;
    -t|--test)
      DO_TEST_BUILD=1
      shift # past argument
      ;;
    --breaksignature)
      DO_BREAK_SIGNATURE=1
      shift # past argument
      ;;
    -*|--*)
      echo "Unknown option $i"
      exit 1
      ;;
    *)
      ;;
  esac
done

if [ -f last_revision ]; then
  last_revision=`cat last_revision`
  revision=$[ $last_revision + 1 ]
else
  revision=1300
fi

build_full_path="flexspi_nor_debug"
unsigned_file="${build_full_path}/ota_demo.bin"
rm -f "${unsigned_file}"

if [ $DO_TEST_BUILD -eq 1 ]; then
  # integration test build
  ./build_flexspi_nor_test.sh
else
  # regular build
  ./build_flexspi_nor_debug.sh
fi

signed_file="${build_full_path}/ota_demo.${revision}.signed.bin"

python3 ${mcuboot_path}/scripts/imgtool.py sign \
        --key ${mcuboot_path}/root-rsa-2048.pem  \
        --align 4 \
        --header-size 0x400 \
        --pad-header \
        --slot-size 0x200000 \
        --version 2.7.0+${revision} \
        ${build_full_path}/ota_demo.bin ${signed_file}

if [ ${DO_BREAK_SIGNATURE} -eq 1 ]; then
  echo -e "${COLOR_RED}BREAKING SIGNATURE${COLOR_RESET}"
  printf "$(printf '\\x%02X\\x%02X\\x%02X\\x%02X' 99 98 19 53)" \
    | dd of=${signed_file} bs=1 seek=25000 count=4 conv=notrunc
fi

if [ ${DO_PUBLISH} -eq 1 -a ${DO_TEST_BUILD} -ne 1 ]; then
  echo -e "${COLOR_YELLOW}"
  echo -e "Publishing..."
  ${fioctl_path}/fioctl-linux-amd64 --verbose targets create-file "${signed_file}" "${revision}" "${BUILD_HWID}" "${PUBLISH_TAGS}"
  echo -e "Publishing Done"
  echo -e "${COLOR_RESET}"
fi

if [ ${DO_FLASH} -eq 1 ]; then
  echo -e "${COLOR_CYAN}"
  echo -e "Flashing..."
  [ ${DO_FLASH_PRIMARY_SLOT} -eq 1 ]   && pyocd flash ${signed_file} -a $FLASH_SLOT1_ADDRESS
  [ ${DO_FLASH_SECONDARY_SLOT} -eq 1 ] && pyocd flash ${signed_file} -a $FLASH_SLOT2_ADDRESS
  echo -e "Flashing Done"
  echo -e "${COLOR_RESET}"
fi

echo ${revision} > last_revision

echo -e "${COLOR_BLUE}"
echo -e "======================================="
echo -e "DO_FLASH                = ${DO_FLASH}"
echo -e "DO_FLASH_PRIMARY_SLOT   = ${DO_FLASH_PRIMARY_SLOT}"
echo -e "DO_FLASH_SECONDARY_SLOT = ${DO_FLASH_SECONDARY_SLOT}"
echo -e "DO_BREAK_SIGNATURE      = ${DO_BREAK_SIGNATURE}"
echo -e "DO_TEST_BUILD           = ${DO_TEST_BUILD}"
echo -e "DO_FORCE_CLEANUP        = ${DO_FORCE_CLEANUP}"
echo -e "DO_PUBLISH              = ${DO_PUBLISH}"
echo -e "PUBLISH_TAGS            = ${PUBLISH_TAGS}"
echo -e "Revision                = ${revision}"
echo -e "======================================="
echo -e "${COLOR_RESET}"
