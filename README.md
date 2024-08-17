# Sukuu

![image](https://github.com/sashi0034/Sukuu/assets/82739042/e412a320-106f-4471-8537-75701a368077)

## About

本リポジトリは Windows
向けに開発されたゲームのソースコードです。このゲームは、[バンダイナムコスタジオ杯 2023](https://siv3d.github.io/ja-jp/event/gamejam2023/)
に提出する目的を持って開発されたものです。本作は最優秀賞を受賞しています。

最新版については、ゲームジャムで提出した内容から発展させたものが『Mutable 50』というタイトルで Steam にてリリースされています。

ストアページは以下のリンクからアクセスできます。

https://store.steampowered.com/app/3147480/Mutable_50/

## Build

1. 本プロジェクトは [Siv3D 0.6.15](https://github.com/Siv3D/OpenSiv3D) を利用しています。ビルドには、適切なバージョンの
   Visual Studio 2022 をインストールする必要があります。

1. コルーチン機能 [boost/coroutine2](https://github.com/boostorg/coroutine2) やスタックトレース取得機能 boost/stacktrace
   などを利用するために、 Boost 1.79.0 を使用しています。
   `BOOST_INCLUDEDIR` に `include` ディレクトリ、 `BOOST_LIBRARYDIR` に `lib` ディレクトリを環境変数に設定してください。

1. 本作を Steam でリリースするに伴い、Steamworks SDK を利用しています。ビルドするには、`STEAMWORKS_SDK` に Steamworks SDK
   のパスを環境変数に設定する必要があります。

- 本作の開発には Jetbrains Rider を使用しています。`.idea` に本プロジェクト用の設定ファイルが含まれています。

## Credits

以下のファイルに本作に関わるクレジット情報が含まれています。

- [`Sukuu/App/credit.html`](Sukuu/App/credit.html)

## License
本プロジェクトに使用されている素材及びコードは、以下のライセンス条件に従います。

1. ソースコードは GNU General Public License (GPL) で配布します。

2. 画像及びサウンドアセットは Creative Commons Attribution-NoDerivs (CC BY-ND) で公開します。

3. フォント及びその他の第三者素材は各素材のオリジナルライセンスに従うものとします。

## Bug Report

[Issues](https://github.com/sashi0034/Sukuu) にて、本作に関するバグ報告等を歓迎しています。
