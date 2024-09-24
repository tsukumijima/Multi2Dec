
－ Multi2Dec Ver.2.10に対するB25DecoderのLinux対応と最適化パッチ －


【必要なもの】
・up0001.zip (Multi2Dec Ver.2.10)
　・MD5ハッシュ値 abd0b827b90d78c48f7b4c91405e85df
　・中身のMulti2Dec_0210_Src.zipだけ必要
・新しめのLinux環境
　・パッチ当てだけならMSYS2などでも可能


【手順】
・あらかじめup0001.zipをこのReadmeと同じ場所に置いておく
・パッチに必要なものをinstall
$ sudo apt install patch unzip
$ unzip up0001.zip
$ cd Multi2Dec
$ unzip Multi2Dec_0210_Src.zip
$ cd Multi2Dec
・ソースの文字コードがSJISなのでUTF-8に変換
$ ../../to_utf8.sh
・パッチを当てる ($ git am --keep-cr ../../*.patch でもよい)
$ ../../apply_patch.sh
$ cd B25Decoder
・ビルドに必要なものをinstall。PC/SC関係はlibarib25と同様なので各自解決のこと
$ sudo apt install g++ libpcsclite-dev make pkg-config
・libarib25と同様にWITH_PCSC_PACKAGEやWITH_PCSC_LIBRARYを指定可能
$ make USE_SIMD=y
・成功するとB25Decoder.soができる。Linux版のEDCBで使える


【最適化について】
SIMD最適化はSSE2とARMのNeonのみ。並列化度はそんなに高くないがオリジナルよりはまずまず速い


【おまけ】
B25Decoder_VC17.slnはVisualStudio2022のソリューションファイル
ソリューション構成でRelease-MINを選ぶと、B25Decoderに不要なコードを減らしてコンパクトにビルドできる
