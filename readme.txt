
はじめに

    unrar32.dllをx64/ユニコード対応させた私家版です。

    動作確認には Windows7/64bit + あふw x64のみで行なっています。

    x64用のファイル名はunrar64j.dllです。(あふwでそのまま使える為)
    'j'のついた背景にはx64になってunrar.dllがunrar64.dllと改名され、
    unrar32はunrar64と安易に変更出来なくなった為です。


必須要件

    unrar64jを動作させるには

    64bit版Windows Vista/7
    Microsoft Visual C++ 2010 SP1 再頒布可能パッケージ (x64)
    unrar64.dll (http://www.rarlab.com/)


    unrar32を動作させるには

    WindowsXP SP3以上
    Microsoft Visual C++ 2010 SP1 再頒布可能パッケージ (x86)
    unrar.dll (http://www.rarlab.com/)

    以上が必要です。


ライセンス

    オリジナルに従います。

    このソフトを使用または使用できないことによって生じた結果等について、
    作者は責任を負わないこととさせていただきます。


サポート

    本DLLに関し、オリジナル作者の亀井さんに問い合わせないようお願いします。
    GitHubのIssues又はTwitterのRuRuRuTanに報告してください。


変更履歴

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

