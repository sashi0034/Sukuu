# Sukuu

![image](https://github.com/sashi0034/Sukuu/assets/82739042/e412a320-106f-4471-8537-75701a368077)

## About

このゲームは、[バンダイナムコスタジオ杯 2023](https://siv3d.github.io/ja-jp/event/gamejam2023/)
に提出したゲームのリポジトリです。本作は最優秀賞を受賞しました。

最新版については、ゲームジャムで提出した内容に改良を加えたものを『Mutable 50』というタイトルで Steam でリリースしています。(https://store.steampowered.com/app/3147480/Mutable_50/)


## Build

1. 本プロジェクトは [Siv3D 0.6.15](https://github.com/Siv3D/OpenSiv3D) を利用しています。ビルドには、適切なバージョンの
   Visual Studio 2022 をインストールする必要があります。

1. コルーチン機能 [boost/coroutine2](https://github.com/boostorg/coroutine2) やスタックトレース取得機能 boost/stacktrace
   などを利用するために、 Boost 1.79.0 を使用しています。
   `BOOST_INCLUDEDIR` に `include` ディレクトリ、 `BOOST_LIBRARYDIR` に `lib` ディレクトリを環境変数に設定してください。

1. 本作を Steam でリリースするに伴い、Steamworks SDK を利用しています。ビルドするには、`STEAMWORKS_SDK` に Steamworks SDK
   のパスを環境変数に設定する必要があります。

1. [Live++](https://www.liveplusplus.tech/jpn/features.html) を利用しています。プロジェクト直下に LivePP 以下を配置してください。

- 本作の開発には Jetbrains Rider を使用しました。

## Credits

- [`Sukuu/App/credit.html`](Sukuu/App/credit.html)

## Licenses

1. ソースコードは GNU General Public License (GPL) で配布します。

1. フォント及びその他の第三者素材は各素材のオリジナルライセンスに従うものとします。

## Bug Reports

[Issues](https://github.com/sashi0034/Sukuu) でバグ報告等を歓迎しています。
