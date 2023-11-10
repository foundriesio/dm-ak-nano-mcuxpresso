# Aktualizr-nano client for MCUXpresso SDK

Proof-of-Concept sample application for Aktualizr-nano, a device management and
OTA update client for MCUs based on FoundriesFactory.
It runs on top of MCUXpresso SDK.

This guide documents the complete flow for using the current solution on a `MIMXRT1060-EVKB` board, including:
* Fetching the required software
* Building and loading the booloader
* Building, signing and setting the version of an application with OTA support
* Loading a first build of the application into the board using an USB cable
* Provisioning device keys and certificates, with and without the use of EdgeLock 2GO and a SE050 secure element
* Onboarding of the device to the EdgeLock 2GO service
* Viewing the device status using FoundriesFactory tools
* Publishing signed binaries of new application versions to be fetched by the device

*The current code is not intended for production use, and is currently a mere
prototype illustrating the possibility of using Foundries.io services in MCUs.*

The first sections of the following instructions were strongly based on
https://github.com/FreeRTOS/iot-reference-nxp-rt1060

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
* MCUXpresso SDK version 2.13.0 for MIMXRT1060-EVKB.
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

### 1.3 Credentials Requirements

* A FoundriesFactory account, that can be created at https://app.foundries.io/

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

Create a new folder, for example, *SDK_2_13_0_MIMXRT1060-EVKB* side-by-side
with the folder created on the previous step,
and unpack the content of the SDK_2_13_0_MIMXRT1060-EVKB.zip file inside it.

By the end of this step, if using the recommended folder names, you will have
the following base tree:

~~~
├── foundriesio-mcu-ota
│   ├── foundriesio
│   ├── core
│   ├── middleware
│   ├── poc
│   └── rtos
└── SDK_2_13_0_MIMXRT1060-EVKB
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


### 3.1 Creating Signing Keys for the Bootloader

The project uses the MCUBoot python script called `imgtool.py` to create signing keys,
and sign and verify the image locally. You can use the Command Prompt in Windows or the Terminal
in MAC or Linux machines.

1. Install the necessary requirements for the image tool. From the repo root folder, run:
     ~~~
     python -m pip install -r SDK_2_13_0_MIMXRT1060-EVKB\middleware\mcuboot_opensource\scripts\requirements.txt
     ~~~

2. Adjust the `sign-rsa2048-pub.c` file content

For the sake of simplicity, the default MCUBoot signing keys are used during on these instructions.
However, the `sign-rsa2048-pub.c` needs to be updated in order for it to match the `root-rsa-2048.pem` key, used during signing.
Adjust the content of the `SDK_2_13_0_MIMXRT1060-EVKB/boards/evkbmimxrt1060/mcuboot_opensource/secure/sign-rsa2048-pub.c` file to:

```
/* Autogenerated by imgtool.py, do not edit. */
const unsigned char rsa_pub_key[] = {
    0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01,
    0x00, 0xd1, 0x06, 0x08, 0x1a, 0x18, 0x44, 0x2c,
    0x18, 0xe8, 0xfb, 0xfd, 0xf7, 0x0d, 0xa3, 0x4f,
    0x1f, 0xbb, 0xee, 0x5e, 0xf9, 0xaa, 0xd2, 0x4b,
    0x18, 0xd3, 0x5a, 0xe9, 0x6d, 0x18, 0x80, 0x19,
    0xf9, 0xf0, 0x9c, 0x34, 0x1b, 0xcb, 0xf3, 0xbc,
    0x74, 0xdb, 0x42, 0xe7, 0x8c, 0x7f, 0x10, 0x53,
    0x7e, 0x43, 0x5e, 0x0d, 0x57, 0x2c, 0x44, 0xd1,
    0x67, 0x08, 0x0f, 0x0d, 0xbb, 0x5c, 0xee, 0xec,
    0xb3, 0x99, 0xdf, 0xe0, 0x4d, 0x84, 0x0b, 0xaa,
    0x77, 0x41, 0x60, 0xed, 0x15, 0x28, 0x49, 0xa7,
    0x01, 0xb4, 0x3c, 0x10, 0xe6, 0x69, 0x8c, 0x2f,
    0x5f, 0xac, 0x41, 0x4d, 0x9e, 0x5c, 0x14, 0xdf,
    0xf2, 0xf8, 0xcf, 0x3d, 0x1e, 0x6f, 0xe7, 0x5b,
    0xba, 0xb4, 0xa9, 0xc8, 0x88, 0x7e, 0x47, 0x3c,
    0x94, 0xc3, 0x77, 0x67, 0x54, 0x4b, 0xaa, 0x8d,
    0x38, 0x35, 0xca, 0x62, 0x61, 0x7e, 0xb7, 0xe1,
    0x15, 0xdb, 0x77, 0x73, 0xd4, 0xbe, 0x7b, 0x72,
    0x21, 0x89, 0x69, 0x24, 0xfb, 0xf8, 0x65, 0x6e,
    0x64, 0x3e, 0xc8, 0x0e, 0xd7, 0x85, 0xd5, 0x5c,
    0x4a, 0xe4, 0x53, 0x0d, 0x2f, 0xff, 0xb7, 0xfd,
    0xf3, 0x13, 0x39, 0x83, 0x3f, 0xa3, 0xae, 0xd2,
    0x0f, 0xa7, 0x6a, 0x9d, 0xf9, 0xfe, 0xb8, 0xce,
    0xfa, 0x2a, 0xbe, 0xaf, 0xb8, 0xe0, 0xfa, 0x82,
    0x37, 0x54, 0xf4, 0x3e, 0xe1, 0x2b, 0xd0, 0xd3,
    0x08, 0x58, 0x18, 0xf6, 0x5e, 0x4c, 0xc8, 0x88,
    0x81, 0x31, 0xad, 0x5f, 0xb0, 0x82, 0x17, 0xf2,
    0x8a, 0x69, 0x27, 0x23, 0xf3, 0xab, 0x87, 0x3e,
    0x93, 0x1a, 0x1d, 0xfe, 0xe8, 0xf8, 0x1a, 0x24,
    0x66, 0x59, 0xf8, 0x1c, 0xab, 0xdc, 0xce, 0x68,
    0x1b, 0x66, 0x64, 0x35, 0xec, 0xfa, 0x0d, 0x11,
    0x9d, 0xaf, 0x5c, 0x3a, 0xa7, 0xd1, 0x67, 0xc6,
    0x47, 0xef, 0xb1, 0x4b, 0x2c, 0x62, 0xe1, 0xd1,
    0xc9, 0x02, 0x03, 0x01, 0x00, 0x01,
};
const unsigned int rsa_pub_key_len = 270;
```

### 3.2 Building and Running the Bootloader

~~~
cd SDK_2_13_0_MIMXRT1060-EVKB\boards\evkbmimxrt1060\mcuboot_opensource\armgcc
build_flexspi_nor_release.bat
~~~

- Load resulting elf image into board
~~~
pyocd load flexspi_nor_release\mcuboot_opensource.elf -t MIMXRT1060
~~~

## 4 Setting up usage of a new factory

### 4.1 Create a new factory

The current MCU OTA PoC leverages Foundries.io infrastructure used for MPUs.
There is no official support for MCUs, so a generic "RaspberryPi 4 64-bit"
factory can be used, even though no actual hardware of that type will be used.

In order to create a factory, follow the instructions available at
https://docs.foundries.io/latest/getting-started/signup/index.html#create-a-factory

### 4.2 Setting up factory PKI

Once the factory is created, you need to set up the factory's PKI. This is done
by running the following command:

```
fioctl keys ca create </absolute/path/to/certs/>
```

The command can only be executed once, so store the generate certificates and
keys in a safe place.

As it will be described in Section 5.2, the content of some of the generated files
is be used inside the aknano_secret.h file and, if EdgeLock 2GO is not used, inside aknano_provisioning_secret.h file as well.

More details about setting up the Factory PKI are available at
https://docs.foundries.io/latest/reference-manual/security/device-gateway.html

### 4.3 Setting up EdgeLock 2GO Managed integration *(`EdgeLock 2GO Managed` mode only)*

FoundriesFactory has a built-in integration with EdgeLock 2GO, which allows
devices to be manages without the need to interact with the EdgeLock 2GO
API explicitly. If that integration is to be used, the factory need to be
configured first.

First the FoundriesFactory needs to be registered with EdgeLock 2GO. This should
be requested to the [Foundries.io support team](https://foundriesio.atlassian.net/servicedesk/customer/portal/1/group/1/create/3).
The 12NC of the SE05X model should to be used be informed in the request.

Once that initial configuration is done, this is the expected output of the
`fioctl el2g status` command:
```
$ fioctl el2g status
# Subdomain: s3q86u6rppswnpjy.device-link.edgelock2go.com

# Product IDs
        ID            NAME
        --            ----
        935389312472  SE050C2HQ1/Z01V3

# Secure Objects
        TYPE  NAME  OBJECT ID
        ----  ----  ---------

# Intermediate CAs
```

Then, configure the device gateway credentials with the following command.
The factory PKI (Section 4.2) should already be set up.

```
fioctl el2g config-device-gateway --pki-dir </absolute/path/to/certs/>
```

The expected result for `fioctl el2g status` will be something like:
```
$ fioctl el2g status -f mcu-test-1
# Subdomain: s3q86u6rppswnphy.device-link.edgelock2go.com

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
ID: 3467
-----BEGIN CERTIFICATE-----
MIIB3TCCAYKgAwIBAgIUG0ckz3TwkypMb2WqDKT+gNkr9OEwCgYIKoZIzj0EAwIw
KjETMBEGA1UEAwwKRmFjdG9yeS1DQTETMBEGA1UECwwKbWN1LXRlc3QtMTAeFw0y
MzAxMjUxNjQ0MDhaFw0zMzAxMjIxNjQ0MDhaMEoxJTAjBgNVBAMMHGZpby02MTI2
M2M4NjRkZTM0Mzk0ZWQ4YjY2ZTQxEzARBgNVBAsMCm1jdS10ZXN0LTExDDAKBhNV
BAoMA254cDBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABFUe47cegb8KaeonTvWD
HfISWSQwDoscNZOIgEE1gz7Oluxl2pgJT3G50SAk4AR94PLiQ3HITsBt/M75h7VX
fMmjZjBkMA4GA1UdDwEB/wQEAwICBDASBgNVHRMBAf8ECDAGAQH/AgEAMB0GA1Ud
DgQWBBTCX8TNoXoXexIy080uMuQ7TcXbODAfBgNVHSMEGDAWgBS+qWI8LILusexB
wM2TjlearF5BczAKBggqhkjOPQQDAgNJADBGAiEA2wX2Yz26fNW6+ZPftf9Fjb1T
fRMBa5XPEB6M3lcDx3YCIQDJdkwKYHdlIheA1jd+WstqY4uTarT2PlA04fStmO05
Sg==
-----END CERTIFICATE-----
```

More details are available at
https://docs.foundries.io/latest/user-guide/el2g.html


## 5 Build and load Proof-of-Concept application


### 5.1 Choose provisioning mode

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

### 5.2 Fill factory-specific information

The PoC application has 2 header files that must be filled manually with
specific data for the factory in use.
The process for obtaining the required content is described in Section 4.2.
The comments inside the code explain which files should be used for each of
the required macros, and are replicated bellow.

- `foundriesio-mcu-ota\foundriesio\aktualizr-nano\src\aknano_secret.h`
~~~
/*
 * The factory UUID is used as subdomain name to access the factory-specific
 * device gateway.
 * The value can be obtained from the console output of the
 * "fioctl keys ca create" command. There will be multiple occurrences of
 * {AKNANO_FACTORY_UUID}.ota-lite.foundries.io (such as
 * c321d157-5953-42ad-bca0-9c44ea420ce1.ota-lite.foundries.io).
 * Just place the {UUID} value bellow.
 * The UUID can also be obtained by dumping information from the "tls-crt" file
 * generated by the "fioctl keys ca create" command. That can be done using
 * a openssl command such as "openssl x509 -in tls-crt -text", where
 * {AKNANO_FACTORY_UUID}.ota-lite.foundries.io URL will be listed as an DNS
 * option inside the "X509v3 Subject Alternative Name" extension.
 */
#define AKNANO_FACTORY_UUID "00000000-0000-0000-0000-000000000000"

/*
 * TCP port used to access the device gateway.
 * This can usually be left unchanged.
 */
#define AKNANO_DEVICE_GATEWAY_PORT 8443

/*
 * This is the server side certificate chain used during TLS connections to the
 * device gateway.
 * It corresponds to the concatenation of "factory_ca.pem" and "tls-crt" files
 * obtained as output of the "fioctl keys ca create" command
 */
#define  AKNANO_DEVICE_GATEWAY_CERTIFICATE \
"-----BEGIN CERTIFICATE-----\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"000000000000\n" \
"-----END CERTIFICATE-----\n" \
"-----BEGIN CERTIFICATE-----\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"000000000000000000000000000000000000000000000000000000000000\n" \
"-----END CERTIFICATE-----\n"
~~~

- `foundriesio-mcu-ota\foundriesio\aktualizr-nano\src\provisioning\aknano_provisioning_secret.h`

**This file does not need to be filled when `EdgeLock 2GO Managed` mode is used**
~~~
/*
 * Foundries.io factory name
 */
#define AKNANO_FACTORY_NAME ""

/*
 * Factory certification authority private key
 * Content of "local-ca.key" file generated by "fioctl keys ca create" command
 */
#define AKNANO_ISSUER_KEY \
"-----BEGIN EC PRIVATE KEY-----\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"000000000000000000000000000000000000\n" \
"-----END EC PRIVATE KEY-----\n"

/*
 * Factory certification authority certificate
 * Content of "local-ca.pem" file generated by "fioctl keys ca create" command
 */
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
"0000000000000000000000000000000000000000000000000000000000000000\n" \
"000000000000000000000000000000000000000000000000\n" \
"-----END CERTIFICATE-----\n"
~~~

### 5.3 Build Proof-of-Concept application

Make sure the `gcc-arm-none-eabi-10.3-2021.10` binaries are included in the PATH,
and that ARMGCC_DIR is set.

~~~
cd foundriesio-mcu-ota\foundriesio\dm-ak-nano-mcuxpresso\armgcc\
build_flexspi_nor_release.bat
~~~

### 5.4 Sign Proof-of-Concept application

~~~
cd foundriesio-mcu-ota\foundriesio\dm-ak-nano-mcuxpresso\armgcc\
python ..\..\..\..\SDK_2_13_0_MIMXRT1060-EVKB\middleware\mcuboot_opensource\scripts\imgtool.py sign --key ..\..\..\..\SDK_2_13_0_MIMXRT1060-EVKB\middleware\mcuboot_opensource\root-rsa-2048.pem --align 4 --header-size 0x400 --pad-header --slot-size 0x200000 --version 1.0.0+1000 --pad --confirm flexspi_nor_release\ota_demo.bin flexspi_nor_release\ota_demo.signed.bin
~~~

Notice that we use the `--pad --confirm` options, that are only required when the
image is going to be loaded for the first time.

We also set the version to 1.0.0 and revision value to 1000.

### 5.5 Load Proof-of-Concept application to board
~~~
pyocd flash flexspi_nor_release\ota_demo.signed.bin -a 0x60040000 -t MIMXRT1060
~~~

### 5.6 Checking execution

After flashing, the board will reboot and start executing the PoC application.
What happens next depends on the selected provisioning mode.

#### 5.6.1 `No Secure Element` and `Standalone SE05X`
The device will register itself in the factory, and will proceed polling the
update server trying to fetch updates.

A new device with the randomly generated UUID should appear in
https://app.foundries.io/factories/FACTORY_NAME
and in the list returned by `fioctl devices list`.

#### 5.6.2 `EdgeLock 2GO Managed`
The device first tries to fetch the secure objects from EdgeLock 2GO endpoint.
If the device was not explicitly added yet (see the next section), an error
message will be printed to the serial output:
~~~
ERROR: iot_agent_update_device_configuration L#537 iot_agent_update_device_configuration_from_datastore failed with 0xffffffd8
Update status report:
  The device update FAILED (0x0035: ERR_DEVICE_NOT_WHITELISTED)
~~~
After performing the step described in the next session, the device should be able
to fetch the secure objects, and it will register itself in the factory,
with a name and UUID staring with `nxp-0000`, such as `nxp-00000000003932ce-0001`.

The new device will be listed in https://app.foundries.io/factories/FACTORY_NAME
and in the list returned by `fioctl devices list`.

### 5.7 Adding device to EdgeLock 2GO *(`EdgeLock 2GO Managed` mode only)*

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

Each device can be added with a command like:
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

The `935389312472` value is the `Hardware 12NC` corresponding to the `SE050C2HQ1/Z01V3`
model, used in the current PoC. Other SE models have a different 12NC values:

| Product Type | 12NC# |
|---| --- |
| SE050A1HQ1/Z01SGZ | 935386722472 |
| SE050A2HQ1/Z01SHZ | 935386984472 |
| SE050B1HQ1/Z01SEZ | 935386985472 |
| SE050B2HQ1/Z01SFZ | 935386986472 |
| SE050C1HQ1/Z01SCZ | 935386987472 |
| SE050C2HQ1/Z01SDZ | 935386988472 |

The 12NC value can also be obtained from [EdgeLock 2GO website](https://edgelock2go.com)

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


## 6 Publishing new versions and performing OTA

### 6.1 Get `fioctl` tool

To generate new versions to test the OTA functionality, an yet unreleased version
of fioctl is required. It can be fetched from: https://github.com/foundriesio/fioctl/tree/mcu-api.
Notice that the feature is only available in the mcu-api branch.

After compiling fioctl, make sure it is on the environment PATH, and log in with
`fioctl login` using your client ID, which must have access to the factory.

### 6.2 Disable provisioning code

By default, the PoC code has the provisioning logic enabled. Although nothing is
done if the device already has a provisioned certificate, it is highly recommended
to disable the provisioning code on the binaries that will be applied using OTA.

To do so, edit the `CMakeLists.txt` file, and set the
`AKNANO_ALLOW_PROVISIONING` value to 0 instead of 1:
~~~
# Enable provisioning code
SET (AKNANO_ALLOW_PROVISIONING 0)
~~~

### 6.3 Build new version

Build the new version using the same command used when building the original
flashed binary.

~~~
cd foundriesio-mcu-ota\foundriesio\dm-ak-nano-mcuxpresso\armgcc\
build_flexspi_nor_release.bat
~~~

### 6.4 Sign new version

~~~
cd foundriesio-mcu-ota\foundriesio\dm-ak-nano-mcuxpresso\armgcc\
python ..\..\..\..\SDK_2_13_0_MIMXRT1060-EVKB\middleware\mcuboot_opensource\scripts\imgtool.py sign --key ..\..\..\..\SDK_2_13_0_MIMXRT1060-EVKB\middleware\mcuboot_opensource\root-rsa-2048.pem --align 4 --header-size 0x400 --pad-header --slot-size 0x200000 --version 1.0.0+1001 flexspi_nor_release\ota_demo.bin flexspi_nor_release\ota_demo.signed.bin
~~~

Notice that the `--pad --confirm` options should not be used.

We also keep the version as 1.0.0, but incremented the revision value to 1001.
Any value higher then the original flashed revision, 1000, can be used.

### 6.5 Publish new version

In order to upload and publish the new version, run the following command.
Notice that the "1001" revision matches the value used during signing.
The "new_tag" tag is used, and it can be any arbitrary string.

~~~
fioctl --verbose targets create-file "ota_demo.signed.bin" "1001" "MIMXRT1060-EVK" "new_tag"
~~~

The configuration of which tag should be followed by each device can be set
dynamically, as described bellow.

### 6.6 Changing the tag tracked by the device

In order to change the tag that is being tracked by a specific device, the command is the same as on MPUs:

```
fioctl device config updates <device_name> --tag <tag> --force
```
For example:
```
fioctl device config updates "0A557380-4808-125C-6C0F-5689E6462ED8" --tag "new_tag" --force
```

### 6.7 Changing the generic config file

The current PoC uses two configurations that can be adjusted by changing the generic JSON config file for the device:
- *polling_interval* sets the interval between aknano check-ins with the device gateway
- *btn_polling_interval* sets the interval between reads of the board user button done by the sample task

Here is an example:
~~~
{
    "reason": "",
    "files": [
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

### 6.7 Removing published version

Published binaries can be removed from the targets list by using the `fioctl targets prune` command:

~~~
fioctl targets prune <target> [<target>...]
~~~

Example:
~~~
fioctl targets prune "MIMXRT1060-EVK-v1001"
~~~

## 7 Enabling AWS IoT MQTT demo task

The same device credentials used for establishing a secure mTLS channel between the device and
Foundries.io device gateway can be used to securely communicate with AWS IoT cloud infrastructure. The integrated
AWS IoT MQTT demo task exemplifies that use.

In order to run the MQTT sample, you need to:
- Have an active [AWS IoT](https://aws.amazon.com/iot-core/) account
- Set the factory Certificate Authority as a trusted signer for devices certificates in AWS IoT, and setup the required
policies. The steps required to do so are organized in a [Python script](https://foundries.io/media/aws-iot-setup.py).
After editing the `factory_cas` with the factory PKI files path (Section 4.2), you can run the script and the account
will be configured. Please notice that this script uses the aws command line application. More information can be found
at [this blog post](https://foundries.io/insights/blog/aws-iot-jitp/)
- Change the AKNANO_ENABLE_AWS_MQTT_DEMO_TASK option in the CMakeLists.txt file from `0` to `1`
- Set the values for `democonfigROOT_CA_PEM` and `democonfigMQTT_BROKER_ENDPOINT` inside
`foundriesio/dm-ak-nano-mcuxpresso/config_files/mqtt_agent_demo_config.h` with the access information for your
AWS IoT account.

After configuring and running the PoC application with the MQTT task, the device will be listed in:
```
aws iot list-things
```
And you can see the published messages in the AWS IoT web interface, in `MQTT test client`, after subscribing to the
`/filter/Publisher0` topic.

## 8 Advanced topics
### 8.1 Device re-provisioning

Specially during testing, it may be useful to re-provision the device with a new set of keys and certificate.
When "No Secure Element" or "Standalone SE05X" provisioning modes are used, all that has to be done is
erase the board flash. Although in "Standalone SE05X" mode secure objects will stay inside the SE, they will
be automatically overwritten once the provisioning software detects that the device does not have serial and
UUID set.

When in "EdgeLock 2GO Managed" mode, additional steps are required. The secure objects need to be erased from
the Secure Element, and the device has to removed and re-added to the EdgeLock 2GO database.
Erasing the secure objects from the Secure Element can be achieved by compiling the firmware with
AKNANO_RESET_DEVICE_ID set (check CMakeLists.txt), loading the binary and waiting for it to run for a few seconds, until the execution is halted.
To remove the device from the EdgeLock 2GO database, run a command like:
```
fioctl el2g devices delete 935389312472 <UUID>
```
And then re-add the device, like described in Section 5.7.
After those steps, the regular firmware (without AKNANO_RESET_DEVICE_ID enabled) should be loaded to the board.

### 8.2 Enabling High Assurance Boot (HAB)

In the process described in the document, the OTA update images signatures validated by the bootloader,
MCU Boot.
In order to allow the bootloader image to be validated as well, the board boot ROM code has to provide
some authentication mechanism.
The MIMXRT1060-EVKB board provides the High Assurance Boot (HAB) functionality, that can
be enabled with this objective.

In order to enable HAB, the [NXP MCUXpresso Secure Provisioning](https://www.nxp.com/design/software/development-software/mcuxpresso-software-and-tools-/mcuxpresso-secure-provisioning-tool:MCUXPRESSO-SECURE-PROVISIONING) tool has to be used. A walkthrough
of the HAB enabling process can be found is also [available](https://www.nxp.com/pages/secure-boot-on-the-i-mx-rt10xx-crossover-mcus:TIP-SECURE-BOOT-ON-THE-I.MX-RT10XX-CROSSOVER-MCUS?SAMLart=ST-AAF0E3YJDJej%2BJVBprc7Vu5rkUdez2IREF5agwakysTZKo6kjKUWEzSa).
HAB documentation usually covers a simpler scenario, where no bootloader is used.
But not much changes when a bootloader is used. It is just a matter of signing the bootloader image
itself (Sections 3.1 and 3.2) instead of a standalone application image.
After fusing the board making it only accept a signed bootloader, the remaining of the OTA process is
still the same. The OTA images are still only signed by the private key corresponding to the public key
selected during MCU Boot build.

### 8.3 Configuring a production device

FoundriesFactory differentiates between [test and production devices](https://docs.foundries.io/latest/reference-manual/ota/production-targets.html).
For `No Secure Element` and `Standalone SE05X` provisioning modes, the certificate for a production
device requires an additional attribute. It is enabled by setting the `AKNANO_PRODUCTION_DEVICE`
value inside `CMakeLists.txt` to `1` during build. This option only affects the device during provisioning.

For `EdgeLock 2GO Managed` mode this is done by using the `--production` option on the
`fioctl el2g device add` command. No changes are needed on the device firmware.

The update process for production devices, MCUs and MPUs,
[involves the use of `waves`](https://docs.foundries.io/latest/reference-manual/ota/production-targets.html).
