# 概要
本プログラムは，MC Softomax探索を用いたコンピュータ将棋ソフトです．USIプロトコルに基づいています．  
コンピュータ将棋の研究を目的として開発されています．  
2020年5月に世界コンピュータ将棋オンライン大会 (WCSOC) に芝浦将棋Softmaxとして参加しました．  
芝浦将棋Jr.を改造して作られていた旧芝浦将棋Softmaxを，1から書き直したプログラムです．

# 評価関数
利用手順は以下の通りです．評価関数ファイルはAperyのものを使用してください．
1. 実行ファイルをダウンロードあるいはコンパイルする
2. 実行ファイルのあるディレクトリ内に，data ディレクトリを作成．その中に kppt_apery ディレクトリを作成する．
3. data/kppt_apery/ 内に，[Apery](https://hiraokatakuya.github.io/apery/)から入手したKPP.bin，KKP.binファイルを置く
4. [将棋所](http://shogidokoro.starfree.jp/index.html)などの将棋GUIソフトに将棋エンジンとして登録すると，対局を行えます

# ライセンス
本プログラムのKPPT型評価関数は[Apery](https://hiraokatakuya.github.io/apery/)を移植したものであり，GPLv3に従います．

# 関連ソフト
[芝浦少将](https://github.com/tanuki12hiromasa/MCSS_55Shogi)：本ソフトの5五将棋版です．評価関数の学習実験用．  
[連続対局ソフト](https://github.com/tanuki12hiromasa/USI_MultipleMatcher)：USI対応ソフトを連続対局させるソフトです．ソフト同士の勝率を測る際などに用いています．