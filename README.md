
# Unrar64j

## 初めに

[unrar32.dll](http://www.madobe.net/archiver/lib/unrar32.html)をx64/ユニコード対応させた私家版です。
32bit版は[本家](http://www.csdinc.co.jp/archiver/lib/unrar32.html)に取り込まれました。

動作確認は基本的に Windows7/64bit + [あふw](http://www.h5.dion.ne.jp/~akt/)x64版で行なっています。

unrar.dllの64bit版がunrar64.dllで名前が被っていますので、
[unarj32j](http://www.csdinc.co.jp/archiver/lib/unarj32.html)に倣ってunrar64j.dllになっています。

バイナリーは[ここ](https://github.com/rururutan/unrar32/releases)に置いてあります。

## 動作環境

unrar64jを動作させるには以下の条件を満たしている必要があります。

* 64bit版Windows Vista以上
* [unrar64.dll](http://www.rarlab.com/)

## 追加API関数

従来の物と比較して以下の関数を追加しています。

* UnrarSetUnicodeMode
* UnrarGetArcFileSizeEx
* UnrarGetArcOriginalSizeEx
* UnrarGetArcCompressedSizeEx
* UnrarGetOriginalSizeEx
* UnrarGetCompressedSizeEx

##対応アプリケーション

此方で確認できているUnicodeモード/unrar64j対応アプリケーションは以下の通りです。

* [あふw](http://www.h5.dion.ne.jp/~akt/)
* [SASF](http://homepage1.nifty.com/Ayakawa/)
* [Paper Plane xUI](http://homepage1.nifty.com/toro/)
* [reces](http://www16.atpages.jp/rayna/reces/index.html)
* [LhaForge](http://claybird.sakura.ne.jp/garage/lhaforge/nosupport/index.html)
* [だいなファイラー](http://hp.vector.co.jp/authors/VA004117/dyna.html)

## ライセンス

オリジナルに従います。

このソフトを使用または使用できないことによって生じた結果等について、
作者は責任を負わないこととさせていただきます。

## サポート

本DLLに関し、オリジナル作者の亀井さんに問い合わせないようお願いします。
GitHubのIssues又はTwitterのRuRuRuTanに報告してください。

## 変更履歴

    version 0.17 2015/6/8 (by Claybird)
      ・RuRuRu氏によるversion 0.16の変更点を取り込み
      ・開発環境をVC2005に戻した
      ・パスワード間違いによるCRCエラーのメッセージを変更した
      ・CHECKARCHIVE_NOT_ASK_PASSWORDを指定したUnrarCheckArchive()でヘッダ暗号化された
        ファイルが破損扱いになっていた不具合を修正

    version 0.16 2013/10/18 (by RuRuRu) [変更点抜粋 by Claybird]
      ・unrar.dllのAPIバージョン6(RAR5対応版)で互換性の無い変更が行われたので対応。
        これまでのバージョンでは展開に失敗する場合があります。
      ・パスワード入力時に何も入力しない場合はキャンセル扱いにした。(7-zip32準拠)
      ・環境によってはパスワード入力に失敗する不具合を修正。
      ・展開キャンセル時に即時終了する様に変更。
      ・プログレスダイアログを親ウインドウ中央に表示するように修正。
      ・128文字以上のパスワードの書庫に対応。
      ・UnrarSetUnicodeModeを実装。
      ・内部的にUTF-16で動作する様に変更。
      ・ユニコード文字を含むパスワードの書庫が開けない不具合を修正。
      ・x64でビルド可能な様に修正(unrar64j.dll)
      ・以下の64ビット整数値の取得API関数を追加。
        UnrarGetArcFileSizeEx
        UnrarGetArcOriginalSizeEx
        UnrarGetArcCompressedSizeEx
        UnrarGetOriginalSizeEx
        UnrarGetCompressedSizeEx

