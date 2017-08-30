# Wio LTE for Arduino

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

## 使い方

[wikiページ](https://github.com/SeeedJP/WioLTEforArduino/wiki)をご参照ください。

[^1]: application/x-www-form-urlencoded
