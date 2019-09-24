## Line thins starterを利用した備品管理アプリのサンプル

### LINE側を自分で用意する場合（Forkして使うには）
1. GitHubのプロジェクトをForkする。
   ForkしたものをGitHubPagesとして公開する。（masterブランチを公開）
1. LINEの開発者アカウントを取得する
1. 新規プロバイダを作成する。
    新規プロバイダ…プロダクトのIDのようなもの
1. 作成したプロバイダで「MessagingAPI」を作成する。
1. LIFFを追加する。
   （MessagingAPIのページに「LIFF」のタブがある）
   - エンドポイントURL…公開したgithubpagesのURLを設定する。
      https://「yourID」.github.io/line-things-starter/liff-app/
   - サイズ：LINE上で表示されるアプリの大きさ
   - BLE feature:使うのでON
   （すべて後から変更可能）
1. トライアルプロダクト情報を作成する。
   https://developers.line.biz/ja/reference/line-things/#post-trial-products
   URL先のコマンドラインを実行して、以下の情報を取得する。
    - サービス探索用UUID
    - デバイス特定用UUID※
    - デバイス特定用CharacteristicUUID※
    ※のUUIDはトライアルプロダクトであれば、
    　誰が作成しても同じ固定値
1. サービス探索用UUIDをソースコードに反映する。
   （IoTデバイスとLiffアプリ(js)の両方）
　 （トライアルプロダクトとしてはここだけでよい）

### kintone連携までやる場合
1. GoogleCloudFunctionsにCloudFunctionsフォルダ配下の
    jsを関数として登録する。
    (package.json記載の依存ライブラリの登録も忘れず）
1. kintone側でアプリを作成する。
   （readme.mdにアプリの詳細記載）
1. 作成したkintoneアプリのapi Tokenなどを
   jsに設定する。
   （readme.mdに設定箇所の詳細記載）

### License

LINE Things Starter に含まれるコードは、クリエイティブ・コモンズ [CC0](http://creativecommons.org/publicdomain/zero/1.0/) の下で利用可能です。
