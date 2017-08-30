# wiolte-driver

Wio LTE��Arduino IDE�p���C�u�����ł��B

## Wio LTE

Wio LTE�́ASeeed���J�����Ă���}�C�R�����W���[���ł��B

![1](img/1.png)

Grove�R�l�N�^�[��STM32F4�}�C�R���ALTE���W���[�����ڂ��Ă���AArduino IDE�őf�����v���g�^�C�s���O���邱�Ƃ��ł��܂��B

## �@�\

|�J�e�S���[|�@�\|�T���v���R�[�h|�֐���|���L|
|:--|:--|:--|:--|:--|
|�d������|LTE���W���[���d��||PowerSupplyLTE||
||Grove�R�l�N�^�[�d��||PowerSypplyGrove||
|�\��|�t���J���[LED�\��|basic/LedSetRGB|LedSetRGB||
|LTE|��M���x|basic/GetRSSI|GetReceivedSignalStrength||
||NTP��������|basic/GetTime|SyncTime||
||SMS���M|sms/SendSMS|SendSMS|���{�ꖢ�Ή�|
||SMS��M|sms/ReceiveSMS|ReceiveSMS|���{�ꖢ�Ή�|
||UDP/TCP�N���C�A���g���M||SocketSend||
||UDP/TCP�N���C�A���g��M||SocketReceive||
||HTTP�N���C�A���gGET||HttpGet|ContentType�Œ�[^1]/https���Ή�|
||HTTP�N���C�A���gPOST||HttpPost|ContentType�Œ�[^1]/https���Ή�|

[^1]: application/x-www-form-urlencoded

## �J�����̃Z�b�g�A�b�v

### Arduino IDE�̊m�F

Wio LTE�̃v���O�����́AArduino IDE�Ńv���O���~���O�A�R���p�C���A�������݂��邱�Ƃ��ł��܂��B

> �p�\�R����Arduino IDE�������Ă��Ȃ��ꍇ�́A�C���X�g�[�����Ă��������B

Arduino IDE���N�����āA�c�[�� > �{�[�h �́AWio Tracker LTE ���\������Ă��邱�Ƃ��m�F���܂��B

> Wio Tracker LTE��������Ȃ��ꍇ�́AWio Tracker LTE�{�[�h�̒ǉ����K�v�ł��B

![12](img/12.png)

### Wio LTE�̃��[�h�ƃf�o�C�X�h���C�o�[�̓���m�F

Wio LTE�̃}�C�R���́A**DFU���[�h**��**�ʏ탂�[�h**��2�̃��[�h������܂��B

**DFU���[�h**��Arduino IDE�ō쐬�����v���O�������}�C�R���֏������ނƂ��Ɏg�p���郂�[�h�ŁA**�ʏ탂�[�h**�̓}�C�R���֏������񂾃v���O���������s���郂�[�h�ł��B

���ꂼ��̃��[�h�����������삵�Ă��邩�A�K�v�ȃ\�t�g�E�F�A�������Ă��邩�A���m�F���܂��傤�B

#### DFU���[�h

**BOOT�{�^��**�������Ȃ���**USB�R�l�N�^�[**��USB�P�[�u����ڑ�����ƁA�}�C�R����DFU(Device Firmware Upgrade)���[�h�ŋN�����܂��B

���ł�USB�P�[�u�����ڑ�����Ă���ꍇ�́A**BOOT�{�^��**�������Ȃ���**RST�{�^��**���N���b�N���āADFU���[�h�ōċN�������邱�Ƃ��ł��܂��B

![5](img/5.png) ![6](img/6.png)

�}�C�R����DFU���[�h�ŋN������ƁA�f�o�C�X�}�l�[�W���[��"���j�o�[�T���V���A���o�X�f�o�C�X"�z����"STM32 BOOTLOADER"���\������܂��B

![3](img/3.png)

> "�ق��̃f�o�C�X"�z����"STM32 BOOTLOADER"���\������Ă���ꍇ�́A�f�o�C�X�h���C�o�[�̃C���X�g�[�����K�v�ł��B
> 
> ![8](img/8.png)

> "���j�o�[�T���V���A���o�X�R���g���[���["�z����"STM Device in DFU Mode"���\������Ă���ꍇ�́AWinUSB�f�o�C�X�h���C�o�[�ɐ؂�ւ����K�v�ł��B
> 
> ![10](img/10.png)

#### �ʏ탂�[�h

�}�C�R�����ʏ탂�[�h�ŋN�����Ă���Ƃ���USB�P�[�u����ڑ�����ƁA�f�o�C�X�}�l�[�W���[��"�|�[�g"�z����"STMicroelectronics Virtual COM Port"���\������܂��B

![4](img/4.png)

> "�ق��̃f�o�C�X"�z����"STM32 Virtual ComPort in FS Mode"�ƕ\������Ă���ꍇ�́AVirtual COM Port�f�o�C�X�h���C�o�[�̃C���X�g�[�����K�v�ł��B
> 
> ![7](img/7.png)

## �⑫����

### Arduino IDE�̃C���X�g�[��

1. [Arduino��Software�T�C�g](https://www.arduino.cc/en/Main/Software)��Download the Arduino IDE�ɂ���Windows Installer���N���b�N���āAarduino-1.8.4-windows.exe����肵�܂��B
1. arduino-1.8.4-windows.exe�����s���܂��B

### Wio Tracker LTE�{�[�h�̒ǉ�

1. Arduino IDE���N�����܂��B
1. �t�@�C�� > ���ݒ� �� �ݒ�^�u �ɂ���"�ǉ��̃{�[�h�}�l�[�W����URL:"�ɁAhttps://raw.githubusercontent.com/Seeed-Studio/Seeed_Platform/master/package_seeeduino_boards_index.json ����͂��܂��B
![9](img/9.png)
1. �c�[�� > �{�[�h > �{�[�h�}�l�[�W�� �ŁASeeed STM32F4 Boards by Seeed Studio ��I�����A�C���X�g�[�����N���b�N���܂��B
![2](img/2.png)

### WinUSB�f�o�C�X�h���C�o�[�ɐ؂�ւ�

1. [Zadig�T�C�g](http://zadig.akeo.ie/)��Download�ɂ���Zadig 2.3���N���b�N���āAzadig-2.3.exe����肵�܂��B
1. Wio LTE��DFU���[�h�Őڑ����܂��B
1. zadig-2.3.exe���N�����܂��B
1. Options > List All Devices ��I�����܂��B
1. STM32 BOOTLOADER ��I��ŁADriver���̍���STTub30�A�E��WinUSB�ɕύX���Ă���AReplace Driver���N���b�N���܂��B
![11](img/11.png)

### Virtual COM Port�f�o�C�X�h���C�o�[�̃C���X�g�[��

1. [ST�}�C�N���G���N�g���j�N�X��STSW-STM32102�T�C�g](http://www.st.com/content/st_com/ja/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-utilities/stsw-stm32102.html)����STSW-STM32102(en.stsw-stm32102.zip)����肵�܂��B
1. en.stsw-stm32102.zip���𓀂��āAVCP_V1.4.0_Setup.exe�����s���܂��B����ƁAC:\Program Files (x86)\STMicroelectronics\Software\Virtual comport driver �Ƀf�o�C�X�h���C�o�[�̃C���X�g�[���t�@�C�����z�u����܂��B
1. C:\Program Files (x86)\STMicroelectronics\Software\Virtual comport driver\Win8\dpinst_amd64.exe �����s���܂��B����ŁAVisual COM Port�f�o�C�X�h���C�o�[���C���X�g�[������܂��B
