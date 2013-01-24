
# Unrar64j

## 初めに

unrar32.dllをx64/ユニコード対応させた私家版です。

動作確認には Windows7/64bit + あふw x64のみで行なっています。

unrar.dllの64bit版がunrar64.dllで名前が被っていますので、
unarj32jに倣ってunrar64j.dllになっています。

## 動作環境

unrar64jを動作させるには以下の条件を満たしている必要があります。

* 64bit版Windows Vista以上
* [unrar64.dll](http://www.rarlab.com/)


unrar32を動作させるには以下の条件を満たしている必要があります。

* WindowsXP SP3以上
* [unrar.dll](http://www.rarlab.com/)

## 追加API関数

従来の物と比較して以下の関数を追加しています。

* UnrarSetUnicodeMode
* UnrarGetArcFileSizeEx
* UnrarGetArcOriginalSizeEx
* UnrarGetArcCompressedSizeEx
* UnrarGetOriginalSizeEx
* UnrarGetCompressedSizeEx

##対応アプリケーション

此方で確認できているUnicodeモード対応アプリケーションは以下の通りです。

* [あふw](http://www.h5.dion.ne.jp/~akt/)
* [SASF](http://homepage1.nifty.com/Ayakawa/)
* [Paper Plane xUI](http://homepage1.nifty.com/toro/)

## ライセンス

オリジナルに従います。

このソフトを使用または使用できないことによって生じた結果等について、
作者は責任を負わないこととさせていただきます。

## サポート

本DLLに関し、オリジナル作者の亀井さんに問い合わせないようお願いします。
GitHubのIssues又はTwitterのRuRuRuTanに報告してください。

## 変更履歴

    2012/11/7 (by RuRuRu)
      ・パスワード入力時に何も入力しない場合はキャンセル扱いにした。(7-zip32準拠)
      ・環境によってはパスワード入力に失敗する不具合を修正。
      ・VS2010ランタイムを不要にした。

    2012/9/6 (by RuRuRu)
      ・展開キャンセル時に即時終了する様に変更。
      ・プログレスダイアログを親ウインドウ中央に表示するように修正。

    2012/8/7 (by RuRuRu)
      ・解凍ダイアログでユニコード文字が表示できていなかった。
      ・パスワードダイアログをキャンセル時の挙動を改善。

    2012/8/5 (by RuRuRu)
      ・128文字以上のパスワードの書庫に対応。
      ・ファイル検索のバグを修正(12/8/1verでデグレード)

    2012/8/1 (by RuRuRu)
      ・UnrarSetUnicodeModeを実装。
      ・内部的にUTF-16で動作する様に変更。
      ・ユニコード文字を含むパスワードの書庫が開けない不具合を修正。

    2012/7/27 (by RuRuRu)
      ・x64でビルド可能な様に修正。
      ・x64でのファイル名はunrar64j.dll
      ・以下の64ビット整数値の取得API関数を追加。
        UnrarGetArcFileSizeEx
        UnrarGetArcOriginalSizeEx
        UnrarGetArcCompressedSizeEx
        UnrarGetOriginalSizeEx
        UnrarGetCompressedSizeEx

