# Aktualizr-nano client for MCUXpresso SDK

Proof-of-Concept code for a Foundries.io OTA client (Aktualizr-nano) running on top of MCUXpresso SDK.

The first sections of the following instructions were strongly based on https://github.com/FreeRTOS/iot-reference-nxp-rt1060

## 1 Prerequisites

### 1.1 Hardware Requirements

* Mini/micro USB cable.
* MIMXRT1060-EVKB board.
* Personal Computer with the Windows platform.
* Network cable RJ45 standard (Network with Internet access).
* (Optional) OM-SE050ARD Development kit.

### 1.2 Software Requirements

* [CMake](https://cmake.org/download/)
* [Make](https://gnuwin32.sourceforge.net/packages/make.htm)
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

1. Connect the USB cable between the personal computer and the open SDA USB port (J1) on the
     board. The serial COM port setting is as below:

     ![Image](https://user-images.githubusercontent.com/45887168/161103634-0855c3b3-cfe4-439e-8e5b-0ce1c0c54b6d.png)

1. Connect the RJ45 cable to the ethernet port.

1. If a SE05X is to be used, Plug in the OM-SE050ARD development kit to the arduino headers on the MIMXRT1060-EVKB board.
     Make sure all the jumpers are in the correct positions as shown in the figure below.

     ![Image](https://user-images.githubusercontent.com/45887168/161103567-f1046d5d-447b-4fc4-ad69-2a81d848b001.png)

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

By the end of this step, if using the recommended folder names, you will have
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
patch -p1 --binary < ..\foundriesio-mcu-ota\patches\mcuboot_sdk_2_12_1_mimxrt1060_evkb_crlf.patch
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


### 4.1 Choose provisioning mode

The application supports 3 provisioning modes:

* No Secure Element: Device Key and Certificate are generated locally and stored
in flash

* Standalone SE05X: Device Key and Certificate are generated locally and
securely stored in SE05X

* EdgeLock 2GO Managed: Device Key and Certificate are generated remotely,
transferred and stored securely in SE05X

*No Secure Element* is the default mode. In order to change the mode to be used,
the `CMakeLists.txt` file has to be edited, setting the correct values for
`AKNANO_ENABLE_SE05X` and `AKNANO_ENABLE_EL2GO`.

* For `Standalone SE05X`:
~~~
# Use SE05X for device key and certificate provisioning
SET (AKNANO_ENABLE_SE05X 1)

# Enable EdgeLock 2GO Managed. Requires SE05X
SET (AKNANO_ENABLE_EL2GO 0)
~~~

* For `EdgeLock 2GO Managed`:
~~~
# Use SE05X for device key and certificate provisioning
SET (AKNANO_ENABLE_SE05X 1)

# Enable EdgeLock 2GO Managed. Requires SE05X
SET (AKNANO_ENABLE_EL2GO 1)
~~~

### 4.2 Fill factory-specific information

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

**This file does not need to be filled when `EdgeLock 2GO Managed` mode is used**
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

Make sure the `gcc-arm-none-eabi-10.3-2021.10` binaries are included in the PATH,
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

### 4.6 Adding device to EdgeLock 2GO *(`EdgeLock 2GO Managed` mode only)*

Foundries.io has an
[integration to with EdgeLock 2GO](https://docs.foundries.io/latest/user-guide/el2g.html)
that facilitates the management of factory devices.

The status of the factory integration can be verified using:
~~~
$ fioctl el2g status
# Subdomain: zhpgunao9lc7zgo4.device-link.edgelock2go.com

# Product IDs
        ID            NAME
        --            ----
        935389312472  SE050C2HQ1/Z01V3

# Secure Objects
        TYPE         NAME                        OBJECT ID
        ----         ----                        ---------
        CERTIFICATE  fio-device-gateway-ca-prod  83000043
        CERTIFICATE  fio-device-gateway-ca       83000043
        KEYPAIR      fio-device-gateway-ca-kp    83000042

# Intermediate CAs
Name: fio-device-gateway-ca
Algorithm: NIST_P256
ID: 3334
-----BEGIN CERTIFICATE-----
MIIBnjCCAUSgAwIBAgIUSkLxEtR+3ez1GLtY2jR0kwytQ4YwCgYIKoZIzj0EAwIw
KzETMBEGA1UEAwwKRmFjdG9yeS1DQTEUMBIGA1UECwwLbnhwLWhidC1wb2MwHhcN
MjIxMTA4MTcyNjUxWhcNMzIxMTA1MTcyNjUxWjBLMSUwIwYDVQQDDBxmaW8tNTlk
YjljOWExYzg1MDEwMDE5ZTAyM2NjMRQwEgYDVQQLDAtueHAtaGJ0LXBvYzEMMAoG
A1UECgwDbnhwMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEOdDstGx5VQ39YgIM
3MTrFUBlxl2aL+wfdbKtA7Q9oc0CqsqswXl8sieGiJ+f2MrWa+jQzljnbiPi5zaE
4Sj94KMmMCQwDgYDVR0PAQH/BAQDAgIEMBIGA1UdEwEB/wQIMAYBAf8CAQAwCgYI
KoZIzj0EAwIDSAAwRQIgfCqiayeiRty/kY/Te6Gm5wpALoyBPYOxUvz0MVQ1G64C
IQCKZuR2IM4exjY9tDhIUD+Iywf0vw4MqlG9vinfAJIo7w==
-----END CERTIFICATE-----

~~~

Each device can be added to be added with a command like:
~~~
fioctl el2g devices add 935389312472 <UUID>
~~~

Where UUID should be passed in decimal format, and is unique to the secure
element being used. The value can be obtained from the PoC application serial
output, and looks like this:
~~~
UID in hex format: 040050013FE3A6988FABD6046389DA0F6880
UID in decimal format: 348555488454828795258771919000334924474496
~~~

The configured devices can be listed with:
~~~
$ fioctl el2g devices list
GROUP             ID                                          LAST CONNECTION
-----             --                                          ---------------
fio-935389312472  348555488454828795258771919000334924474496
fio-935389312472  348555487759198980128543317891042222499968
~~~

And the status for a specific device can be checked with:
~~~
$ fioctl el2g devices show 348555488454828795258771919000334924474496
Hardware Type: SE050C2HQ1/Z01V3
Hardware 12NC: 935389312472
Secure Objects:
        NAME                      TYPE         STATUS
        ----                      ----         ------
        fio-device-gateway-ca     CERTIFICATE  PROVISIONING_COMPLETED
        fio-device-gateway-ca-kp  KEYPAIR      PROVISIONING_COMPLETED
Certificates:
# fio-device-gateway-ca
-----BEGIN CERTIFICATE-----
MIIBoTCCAUegAwIBAgIHNzNvAAAAATAKBggqhkjOPQQDAjBLMSUwIwYDVQQDDBxm
aW8tNTlkYjljOWExYzg1MDEwMDE5ZTAyM2NjMRQwEgYDVQQLDAtueHAtaGJ0LXBv
YzEMMAoGA1UECgwDbnhwMB4XDTIyMTExODE4MzUwMFoXDTMyMTExODE4MzUwMFow
OjEiMCAGA1UEAwwZbnhwLTAwMDAwMDAwMDAzNzMzNmYtMDAwMTEUMBIGA1UECwwL
bnhwLWhidC1wb2MwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAASfb9cWHvRPIIIR
Axuk5lq0kcbLu/XtxIC+z7ecmVj5KeoqRWF2vW27I8qaARTDwaiwlEvZ3kK4YfFh
M5P92GaaoycwJTAOBgNVHQ8BAf8EBAMCB4AwEwYDVR0lBAwwCgYIKwYBBQUHAwIw
CgYIKoZIzj0EAwIDSAAwRQIhAOjNZoB0NNhCbRXgcGRZttEbY46l7Y1PjKW6NPe3
oJhQAiA/6OgyCRaqpMg4yKUlbmFwTQPkq3lCmvH2sewWqHK0hw==
-----END CERTIFICATE-----
~~~


## 5 Publishing new versions and performing OTA

### 5.1 Get `fioctl` tool

To generate new versions to test the OTA functionality, an yet unreleased version
of fioctl is required. It can be fetched from: https://github.com/foundriesio/fioctl/tree/mcu-api.
Notice that the feature is only available in the mcu-api branch.

After compiling fioctl, make sure it is on the environment PATH, and log in with
`fioctl login` using your client ID, which must have access to the factory.

### 5.2 Disable provisioning code

By default, the PoC code has the provisioning logic enabled. Although nothing is
done if the device already has a provisioned certificate, it is highly recommended
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
dynamically, as described bellow.

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

Save this file to a local file, for example, `device_settings.json`, and upload it
with a command such as:

~~~
fioctl devices config set "0A557380-4808-125C-6C0F-5689E6462ED8"  --raw device_settings.json
~~~

Replace "0A557380-4808-125C-6C0F-5689E6462ED8" with the name of the
device that will be affected. The name can be verified in the `app.foundries.io`
factory web dashboard, or by looking fo the device UUID in the serial output.

The factory user needs to have `Owner` role in the factory, and the `fioctl` token
requires the `devices:read-update` permission.
