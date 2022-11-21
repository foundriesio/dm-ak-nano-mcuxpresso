# Aktualizr-nano client for MCUXpresso SDK

Proof-of-Concept code for a Foundries.io OTA client (Aktualizr-nano) running on top of MCUXpresso SDK.

The following instructions were inspired by the https://github.com/FreeRTOS/iot-reference-nxp-rt1060

## 1 Prerequisites

### 1.1 Hardware Requirements

* Mini/micro USB cable.
* MIMXRT1060-EVKB board.
* Personal Computer with the Windows platform.
* Network cable RJ45 standard (Network with Internet access).
* (Optional) OM-SE050ARD Development kit.

### 1.2 Software Requirements

* [CMake](https://cmake.org/download/)
* MCUXpresso SDK version 2.12.1 for MIMXRT1060-EVKB.
    To download, visit the
    [MCUXpresso Software Development Kit (SDK) page](https://www.nxp.com/design/software/development-software/mcuxpresso-software-and-tools-/mcuxpresso-software-development-kit-sdk:MCUXpresso-SDK).
    (A user account is required to download and download with the default option selected.)
    The projects use board support libraries directly from the NXP github repository, however,
    the SDK is required to be able to get the correct board configuration and for the build and
    flash tool to work correctly.
* [Python3](https://www.python.org/downloads/)
* A serial terminal application, such as [Tera Term](https://ttssh2.osdn.jp/index.html.en).
* [GCC compiler for ARM](https://developer.arm.com/downloads/-/gnu-rm), gcc-arm-none-eabi-10.3-2021.10, or compatible
* [PyOCD](https://pyocd.io/)
* [West](https://pypi.org/project/west/)
* [Patch](https://gnuwin32.sourceforge.net/packages/patch.htm)


## 2 Hardware and Software Setup

### 2.1 Setting up Device

1. Plug in the OM-SE050ARD development kit to the arduino headers on the MIMXRT1060-EVKB board.
     Make sure all the jumpers are in the correct positions as shown in the figure below.

     ![Image](https://user-images.githubusercontent.com/45887168/161103567-f1046d5d-447b-4fc4-ad69-2a81d848b001.png)

1. Connect the USB cable between the personal computer and the open SDA USB port (J1) on the
     board. The serial COM port setting is as below:

     ![Image](https://user-images.githubusercontent.com/45887168/161103634-0855c3b3-cfe4-439e-8e5b-0ce1c0c54b6d.png)

1. Connect the RJ45 cable to the ethernet port.


## 2.2 Fetching Proof-of-Concept code

Create a new folder, for example, *foundriesio-mcu-ota*, and run the following
commands inside it:

~~~
west init -m https://github.com/foundriesio/dm-ak-nano-mcuxpresso-manifest.git
west update
~~~

## 2.3 Unpack MCUXpresso SDK

Create a new folder, for example, *SDK_2_12_1_MIMXRT1060-EVKB* side-by-side
with the folder created on the previous step,
and unpack the content of the SDK_2_12_1_MIMXRT1060-EVKB.zip file inside it.

By the end of this step, if using the recomended folder names, you will have
the following base tree:

~~~
├── foundriesio-mcu-ota
│   ├── foundriesio
│   ├── mcuxsdk
│   ├── middleware
│   ├── poc
│   └── rtos
└── SDK_2_12_1_MIMXRT1060-EVKB
    ├── boards
    ├── CMSIS
    ├── components
    ├── COPYING-BSD-3
    ├── devices
    ├── docs
    ├── LA_OPT_NXP_Software_License.txt
    ├── middleware
    ├── MIMXRT1060-EVKB_manifest_v3_10.xml
    ├── rtos
    ├── SW-Content-Register.txt
    └── tools
~~~

## 2.3 Board flash cleanup

- Start preferentially with an empty board, erasing original content if needed:

~~~
pyocd erase --mass -t MIMXRT1060
~~~

## 3 Prepare and Run the Bootloader


### 3.1 Patch the bootloader

Some adjustments are required on top of MCUBoot available from MCUXpresso 2.12.1.

Apply the patch by running the following command inside the `SDK_2_12_1_MIMXRT1060-EVKB`
directory:
~~~
patch -P1 < ..\patches\mcuboot_sdk_2_12_1_mimxrt1060_evkb.patch
~~~


### 3.2 Creating Signing Keys for the Bootloader

The project uses the MCUBoot python script called `imgtool.py` to create signing keys,
and sign and verify the image locally. You can use the Command Prompt in Windows or the Terminal
in MAC or Linux machines.

1. Install the necessary requirements for the image tool. From the repo root folder, run:
     ~~~
     python -m pip install -r SDK_2_12_1_MIMXRT1060-EVKB\middleware\mcuboot_opensource\scripts\requirements.txt
     ~~~

For the sake of simplicity, the default MCUBoot signing keys are used during on these instructions.
The patch mcuboot_sdk_2_12_1_mimxrt1060_evkb.patch contains an update to the sign-rsa2048-pub.c,
making it match the root-rsa-2048.pem key, used during signing.


### 3.3 Building and Running the Bootloader

~~~
cd SDK_2_12_1_MIMXRT1060-EVKB\boards\evkmimxrt1060\mcuboot_opensource\armgcc
flexspi_nor_release.bat
~~~

- Load resulting elf image into board
~~~
pyocd load flexspi_nor_release\mcuboot_opensource.elf
~~~

## 4 Build and load Proof-of-Concept application

### 4.1 Fill factory-specific information

The PoC application has 2 header files that must be filled manually with
private data for the factory in use.

Contact the Foundries.io team to get the expected content for the files. Notice
that the `AKNANO_API_TOKEN` is not required by default anymore.

- `foundriesio-mcu-ota\foundriesio\dm-ak-nano-mcuxpresso\aknano_secret.h`
~~~
#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
/* set API acces token */
#define AKNANO_API_TOKEN "0000000000000000000000000000000000000000"
#endif

#define AKNANO_DEVICE_GATEWAY_PORT 8443
#define AKNANO_FACTORY_UUID "00000000-0000-0000-0000-000000000000"
#define AKNANO_DEVICE_GATEWAY_CERTIFICATE \
"-----BEGIN CERTIFICATE-----\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"000000000000000000000000000000000000000==\n" \
"-----END CERTIFICATE-----\n"
~~~

- `foundriesio-mcu-ota\foundriesio\dm-ak-nano-mcuxpresso\provisioning\aknano_provisioning_secret.h`
~~~
#define AKNANO_FACTORY_NAME ""

/* local-ca.key */
#define AKNANO_ISSUER_KEY \
"-----BEGIN EC PRIVATE KEY-----\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"000000000000000000000000000000000000\n" \
"-----END EC PRIVATE KEY-----\n"

/* local-ca.pem */
#define AKNANO_ISSUER_CERTIFICATE \
"-----BEGIN CERTIFICATE-----\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"000000000000000000000000000000000000000000000000\n" \
"-----END CERTIFICATE-----\n"
~~~

### 4.2 Build Proof-of-Concept application

Make sure the gcc-arm-none-eabi-10.3-2021.10 binaries are included in the PATH,
and that ARMGCC_DIR is set.

~~~
cd foundriesio-mcu-ota\foundriesio\dm-ak-nano-mcuxpresso\armgcc\
build_flexspi_nor_release.bat
~~~

### 4.3 Sign Proof-of-Concept application

~~~
cd foundriesio-mcu-ota\foundriesio\dm-ak-nano-mcuxpresso\armgcc\
python ..\..\..\..\SDK_2_12_1_MIMXRT1060-EVKB\middleware\mcuboot_opensource\scripts\imgtool.py sign \
        --key ..\..\..\..\SDK_2_12_1_MIMXRT1060-EVKB-EVKB\middleware\mcuboot_opensource\root-rsa-2048.pem  \
        --align 4 \
        --header-size 0x400 \
        --pad-header \
        --slot-size 0x200000 \
        --version 1.0.0+1000 \
        --pad --confirm \
        flexspi_nor_release\ota_demo.bin \
        flexspi_nor_release\ota_demo.signed.bin \
~~~

Notice that we use the `--pad --confirm` options, that are only required when the
image is going ot be loaded fo the first time.

We also set the version to 1.0.0 and revision value to 1000. 

### 4.4 Load Proof-of-Concept application to board

~~~
pyocd flash flexspi_nor_release\ota_demo.signed.bin -a 0x60040000 -t MIMXRT1060
~~~

### 4.5 Checking execution
After flashing, the board will reboot and start executing the PoC application.
The device will register itself in the factory, and will proceed polling the 
update server trying to fetch updates.

A new device with the randomly generated UUID should appear in
https://app.foundries.io/factories/FACTORY_NAME


## 5 Publishing new versions and performing OTA

### 5.1 Get `fioctl` tool

To generate new versions to test the OTA functionality, an yet unreleased version
of fioctl is required. It can be fetched from: https://github.com/foundriesio/fioctl/tree/mcu-api.
Notice that the feature is only available in the mcu-api branch.

After compiling fioctl, make sure it is on the enviroment PATH, and log in with
`fioctl login` using your client ID, which must have access to the factory.

### 5.2 Disable provisioning code

By default, the PoC code has the provisioning logic enabled. Although nothing is
done if the device already has a provisioned certificate, it is higly recomended
to disable the provisioning code on the binaries that will be applied using OTA.

To do so, edit the `CMakeLists.txt` file, and set the 
`AKNANO_ALLOW_PROVISIONING` value to 0 instead of 1:
~~~
# Enable provisioning code
SET (AKNANO_ALLOW_PROVISIONING 0)
~~~

### 5.3 Build new version

Build the new version using the same command used when building the original 
flashed binary.

~~~
cd foundriesio-mcu-ota\foundriesio\dm-ak-nano-mcuxpresso\armgcc\
build_flexspi_nor_release.bat
~~~

### 5.4 Sign new version

~~~
cd foundriesio-mcu-ota\foundriesio\dm-ak-nano-mcuxpresso\armgcc\
python ..\..\..\..\SDK_2_12_1_MIMXRT1060-EVKB\middleware\mcuboot_opensource\scripts\imgtool.py sign \
        --key ..\..\..\..\SDK_2_12_1_MIMXRT1060-EVKB-EVKB\middleware\mcuboot_opensource\root-rsa-2048.pem  \
        --align 4 \
        --header-size 0x400 \
        --pad-header \
        --slot-size 0x200000 \
        --version 1.0.0+1001 \
        flexspi_nor_release\ota_demo.bin \
        flexspi_nor_release\ota_demo.signed.bin \
~~~

Notice that the `--pad --confirm` options should not be used.

We also keep the version as 1.0.0, but incremented the revision value to 1001.
Any value higher then the original flashed revision, 1000, can be used.

### 5.5 Publish new version

In order to upload and publish the new version, run the following command.
Notice that the "1001" revision matches the value used during signing.
The "nxp" tag is used, and it can be any arbitrary string. 

~~~
fioctl --verbose targets create-file "ota_demo.signed.bin" "1001" "MIMXRT1060-EVK" "nxp"
~~~

The configuration of which tag should be followed by each device can be set
dinamically, as described bellow.

### 5.6 Changing the tag tracked by the device

The *tag* that is being tracked by the device, as long as the *polling_interval* and
the *btn_polling_interval* can be adjusted by uploading a settings JSON file to the
factory. Here is an example:

~~~
{
    "reason": "",
    "files": [
        {
            "name": "tag",
            "value": "nxp",
            "unencrypted": true
        },
        {
            "name": "polling_interval",
            "value": "10",
            "unencrypted": true
        },
        {
            "name": "btn_polling_interval",
            "value": "2000",
            "unencrypted": true
        }
    ]
}
~~~

Save this file to a local file, for exemple, `device_settings.json`, and upload it
with a command such as:

~~~
fioctl devices config set "0A557380-4808-125C-6C0F-5689E6462ED8"  --raw device_settings.json
~~~

Replace "0A557380-4808-125C-6C0F-5689E6462ED8" with the name of the
device that will be affected. The name can be verified in the `app.foundries.io` 
factory web dashboard, or by looking fo the device UUID in the serial output.

The factory user needs to have `Owner` role in the factory, and the `fioct` token
requires the `devices:read-update` permission.
