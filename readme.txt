
はじめに

    unrar32.dllをx64対応させた私家版です。

    動作確認には Windows7/64bit + あふw x64のみで行なっています。

    x64用のファイル名はunrar64j.dllです。(あふwでそのまま使える為)
    'j'のついた背景にはx64になってunrar.dllがunrar64.dllと改名され、
    unrar32はunrar64と安易に変更出来なくなった為です。


ライセンス

    オリジナルに従います。


変更履歴

    version 0.13 2012/7/27 (by RuRuRu)
      ・x64でビルド可能な様に修正。
      ・x64でのファイル名はunrar64j.dll
      ・以下の64ビット整数値の取得関数を追加。
        UnrarGetArcFileSizeEx
        UnrarGetArcOriginalSizeEx
        UnrarGetArcCompressedSizeEx
        UnrarGetOriginalSizeEx
        UnrarGetCompressedSizeEx

