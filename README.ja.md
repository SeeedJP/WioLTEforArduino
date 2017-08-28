# wiolte-driver

Wio LTEのArduino IDE用ライブラリです。

## Wio LTE

Wio LTEは、Seeedが開発しているマイコンモジュールです。

![1](img/1.png)

GroveコネクターとSTM32F4マイコン、LTEモジュールが載っており、Arduino IDEで素早くプロトタイピングすることができます。

## 機能

|カテゴリー|機能|関数名|注記|
|:--|:--|:--|:--|
|電源制御|LTEモジュール電源|PowerSupplyLTE||
||Groveコネクター電源|PowerSypplyGrove||
|表示|フルカラーLED表示|LedSetRGB||
|LTE|受信強度|GetReceivedSignalStrength||
||SMS送信|SendSMS|日本語未対応|
||SMS受信|ReceiveSMS|日本語未対応|
||NTP時刻同期|SyncTime||
||UDP/TCPクライアント送信|SocketSend||
||UDP/TCPクライアント受信|SocketReceive||
||HTTPクライアントGET|HttpGet|ContentType固定[^1]/https未対応|
||HTTPクライアントPOST|HttpPost|ContentType固定[^1]/https未対応|

[^1]: application/x-www-form-urlencoded

## 開発環境

追加のボードマネージャのURL:

https://raw.githubusercontent.com/Seeed-Studio/Seeed_Platform/master/package_seeeduino_boards_index.json

ボードマネージャ:

![2](img/2.png)

プログラム書き換えモード:

![3](img/3.png)

通常:

![4](img/4.png)

