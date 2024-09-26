
# Multi2Dec (B25Decoder)

## このリポジトリについて

このリポジトリは、http://www2.wazoku.net/2sen/dtvup/ で公開されている、  
[Multi2Dec v2.10](http://www2.wazoku.net/2sen/dtvup/source/up0001.zip) に同梱されているソースコードのミラーです。

Axfc で公開されている [Multi2Dec_0210_Patch.tar.gz](https://www.axfc.net/u/4083028) (Pass: DTV) 内の B25Decoder 向けのパッチを適用し、オリジナルから下記の改良を加えています。  

- **B25Decoder.dll 単体のみを VS2019 でビルドできる [B25Decoder_VC17.sln](B25Decoder_VC17.sln) を追加**
  - 大元のパッチのビルドターゲット指定は VS2022 (v143) だが、個人的に VS2019 に統一していることもあり、意図的に VS2019 (v142) にダウングレードしている
- **[Linux 版 EDCB](https://github.com/xtne6f/EDCB/blob/1d0000e0670de51e1d27e672dcac9dee567eed06/Document/HowToBuild.txt#%E3%83%93%E3%83%AB%E3%83%89linux) 向けの B25Decoder.so (Linux 版 B25Decoder) をビルドできるように改良**
  - `/usr/local/lib/edcb/B25Decoder.so` に配置すると、EDCB の B25 デコード機能が有効化される 
- B25Decoder には不要な処理を減らしてコンパクトにビルドできる `MINIMIZE_FOR_B25DECODER` マクロを追加
  - `Release-MIN` でビルドするとファイルサイズを削減できる
- B25Decoder 関連ソースコードの文字コードを Shift-JIS から UTF-8 with BOM に変換
- 常に JST (UTC+9) 基準で処理を行うように修正
- オリジナルの開発時期 (最終更新: 2009年) 由来の古い記述のメンテナンス・モダナイズ

適用したパッチは [Patches](Patches/) 以下に配置しています。

> [!NOTE]
> 上記のパッチは、主に Linux 版 EDCB で直接 B25 デコード処理を行うためのものです。  
> そのためオリジナルの B25Decoder 実装（[libaribb25 の B25Decoder インターフェイス実装](https://github.com/tsukumijima/libaribb25/blob/master/aribb25/libaribb25.cpp) とは異なる）の維持のみが行われており、Multi2Dec のソースコードに含まれるそのほかのツール群はメンテナンスされていません。

> [!TIP]
> 2024年現在では「ARIB STD-B25 仕様確認テストプログラム」を源流とする [libaribb25](https://github.com/tsukumijima/libaribb25) / [libarib25](https://github.com/stz2012/libarib25) を、B25Decoder インターフェイスに部分的に適合させた B25Decoder.dll を使う手法が主流です。  
> 他方 libaribb25 は TS ファイル全体を完璧にデコードするために設計されており、デコード失敗時などのエラーハンドリングや機能の豊富さでは、Multi2Dec 内のオリジナルの B25Decoder 実装の方が優れているようです。  
> C++ で記述されており、libaribb25 よりコードが読みやすい点もメリット。

## ビルド

ビルドを実行すると、B25Decoder.dll / B25Decoder.so が生成されます。  

> [!NOTE]
> Multi2Dec に含まれるツール群全体のビルドを行う [Multi2Dec.sln](Multi2Dec.sln) はオリジナルの最終バージョンから一切メンテナンスされておらず、約20年前に公開された VS2005 向けの sln ファイルのままです。

### Windows

```powershell
msbuild B25Decoder_VC17.sln /t:Build /p:Configuration=Release /p:Platform=Win32 /p:PlatformToolset=v142
msbuild B25Decoder_VC17.sln /t:Build /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v142
```

## Windows (コンパクト版)

```powershell
msbuild B25Decoder_VC17.sln /t:Build /p:Configuration=Release-MIN /p:Platform=Win32 /p:PlatformToolset=v142
msbuild B25Decoder_VC17.sln /t:Build /p:Configuration=Release-MIN /p:Platform=x64 /p:PlatformToolset=v142
```

## Linux

```bash
sudo apt install g++ libpcsclite-dev make pkg-config
cd B25Decoder
make USE_SIMD=y
```

## 補遺

以下のオリジナルの Readme も参照してください。

- [Readme.txt](Readme.txt) : [Multi2Dec v2.10](http://www2.wazoku.net/2sen/dtvup/source/up0001.zip) 内に含まれている、オリジナルの Multi2Dec の Readme
- [Readme-B25Decoder.txt](Readme-B25Decoder.txt) : [Multi2Dec v2.10](http://www2.wazoku.net/2sen/dtvup/source/up0001.zip) 内に含まれている、オリジナルの B25Decoder の Readme
- [Readme-Patches.txt](Readme-Patches.txt) : [Multi2Dec_0210_Patch.tar.gz](https://www.axfc.net/u/4083028) 内に含まれている、パッチの Readme

> [!NOTE]
> 上記 Readme の内容は公開当時から一切更新されておらず、情報が古くなっている箇所があります。


Multi2Dec やその改良パッチの作者の方々に感謝します。
