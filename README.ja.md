# wiolte-driver

Wio LTEのArduino IDE用ライブラリです。

## Wio LTE

Wio LTEは、Seeedが開発しているマイコンモジュールです。

![1](img/1.png)

GroveコネクターとSTM32F4マイコン、LTEモジュールが載っており、Arduino IDEで素早くプロトタイピングすることができます。

## 機能

|カテゴリー|機能|サンプルコード|関数名|注記|
|:--|:--|:--|:--|:--|
|電源制御|LTEモジュール電源||PowerSupplyLTE||
||Groveコネクター電源||PowerSypplyGrove||
|表示|フルカラーLED表示|basic/LedSetRGB|LedSetRGB||
|LTE|受信強度|basic/GetRSSI|GetReceivedSignalStrength||
||NTP時刻同期|basic/GetTime|SyncTime||
||SMS送信|sms/SendSMS|SendSMS|日本語未対応|
||SMS受信|sms/ReceiveSMS|ReceiveSMS|日本語未対応|
||UDP/TCPクライアント送信||SocketSend||
||UDP/TCPクライアント受信||SocketReceive||
||HTTPクライアントGET||HttpGet|ContentType固定[^1]/https未対応|
||HTTPクライアントPOST||HttpPost|ContentType固定[^1]/https未対応|

[^1]: application/x-www-form-urlencoded

## 開発環境のセットアップ

### Arduino IDEの確認

Wio LTEのプログラムは、Arduino IDEでプログラミング、コンパイル、書き込みすることができます。

> パソコンにArduino IDEが入っていない場合は、インストールしてください。

Arduino IDEを起動して、ツール > ボード の、Wio Tracker LTE が表示されていることを確認します。

> Wio Tracker LTEが見つからない場合は、Wio Tracker LTEボードの追加が必要です。

![12](img/12.png)

### Wio LTEのモードとデバイスドライバーの動作確認

Wio LTEのマイコンは、**DFUモード**と**通常モード**の2つのモードがあります。

**DFUモード**はArduino IDEで作成したプログラムをマイコンへ書き込むときに使用するモードで、**通常モード**はマイコンへ書き込んだプログラムを実行するモードです。

それぞれのモードが正しく動作しているか、必要なソフトウェアが入っているか、を確認しましょう。

#### DFUモード

**BOOTボタン**を押しながら**USBコネクター**にUSBケーブルを接続すると、マイコンがDFU(Device Firmware Upgrade)モードで起動します。

すでにUSBケーブルが接続されている場合は、**BOOTボタン**を押しながら**RSTボタン**をクリックして、DFUモードで再起動させることができます。

![5](img/5.png) ![6](img/6.png)

マイコンがDFUモードで起動すると、デバイスマネージャーの"ユニバーサルシリアルバスデバイス"配下に"STM32 BOOTLOADER"が表示されます。

![3](img/3.png)

> "ほかのデバイス"配下に"STM32 BOOTLOADER"が表示されている場合は、デバイスドライバーのインストールが必要です。
> 
> ![8](img/8.png)

> "ユニバーサルシリアルバスコントローラー"配下に"STM Device in DFU Mode"が表示されている場合は、WinUSBデバイスドライバーに切り替えが必要です。
> 
> ![10](img/10.png)

#### 通常モード

マイコンが通常モードで起動しているときにUSBケーブルを接続すると、デバイスマネージャーの"ポート"配下に"STMicroelectronics Virtual COM Port"が表示されます。

![4](img/4.png)

> "ほかのデバイス"配下に"STM32 Virtual ComPort in FS Mode"と表示されている場合は、Virtual COM Portデバイスドライバーのインストールが必要です。
> 
> ![7](img/7.png)

## 補足説明

### Arduino IDEのインストール

1. [ArduinoのSoftwareサイト](https://www.arduino.cc/en/Main/Software)のDownload the Arduino IDEにあるWindows Installerをクリックして、arduino-1.8.4-windows.exeを入手します。
1. arduino-1.8.4-windows.exeを実行します。

### Wio Tracker LTEボードの追加

1. Arduino IDEを起動します。
1. ファイル > 環境設定 の 設定タブ にある"追加のボードマネージャのURL:"に、https://raw.githubusercontent.com/Seeed-Studio/Seeed_Platform/master/package_seeeduino_boards_index.json を入力します。
![9](img/9.png)
1. ツール > ボード > ボードマネージャ で、Seeed STM32F4 Boards by Seeed Studio を選択し、インストールをクリックします。
![2](img/2.png)

### WinUSBデバイスドライバーに切り替え

1. [Zadigサイト](http://zadig.akeo.ie/)のDownloadにあるZadig 2.3をクリックして、zadig-2.3.exeを入手します。
1. Wio LTEをDFUモードで接続します。
1. zadig-2.3.exeを起動します。
1. Options > List All Devices を選択します。
1. STM32 BOOTLOADER を選んで、Driver欄の左をSTTub30、右をWinUSBに変更してから、Replace Driverをクリックします。
![11](img/11.png)

### Virtual COM Portデバイスドライバーのインストール

1. [STマイクロエレクトロニクスのSTSW-STM32102サイト](http://www.st.com/content/st_com/ja/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-utilities/stsw-stm32102.html)からSTSW-STM32102(en.stsw-stm32102.zip)を入手します。
1. en.stsw-stm32102.zipを解凍して、VCP_V1.4.0_Setup.exeを実行します。すると、C:\Program Files (x86)\STMicroelectronics\Software\Virtual comport driver にデバイスドライバーのインストールファイルが配置されます。
1. C:\Program Files (x86)\STMicroelectronics\Software\Virtual comport driver\Win8\dpinst_amd64.exe を実行します。これで、Visual COM Portデバイスドライバーがインストールされます。
