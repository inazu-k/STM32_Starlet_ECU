# 同期デコーダのホスト試験

ホスト用C11コンパイラを使い、ルートのARM用CMake設定と分離して実行する。

```powershell
cmake -S tests -B build/sync-host -G Ninja
cmake --build build/sync-host
ctest --test-dir build/sync-host --output-on-failure
```

コンパイラやNinjaがPATHにない場合は、インストール済みの実体を`CMAKE_C_COMPILER`と`CMAKE_MAKE_PROGRAM`で指定する。テストの周期範囲100〜2000 tick、停止3000 tickは模擬試験専用であり、実機用設定値ではない。

検証対象は全24歯からの取得開始、G2先着、同期確定までの待機、周期変動、Gの誤位置・誤種類・重複・欠落・不正入力、NEの短周期・欠落・時刻逆転、32 bit時刻周回、固定停止時間、ノイズによる停止時刻の非延長、停止後の再取得、キャプチャ欠落通知、設定値の検証。

入力のGはNE立下りに対応したレベル取得結果である。NE/Gの実際の取得遅延や電気波形の成立は、このホスト試験では検証しない。

`engine_sync_service`試験では64件のキュー境界、満杯時の非上書き、欠落通知中の結果無効化、世代更新後の再取得、イベント取出し中の割り込み相当の入力追加、継続入力中の処理件数制限、停止前後のイベントの一括処理、時刻周回、空確認の直前の入力到来、割り込みマスク復元を検証する。プラットフォーム関数を模擬して競合するタイミングに入力を挿入するもので、実CPUの割り込み遅延の測定ではない。



## CMakeを使わない実行と追加試験

```sh
python3 tests/run_host_tests.py
```

`CC`でホストCコンパイラを指定できる。すべてC11、警告をエラーとしてコンパイルする。
追加したecu_model試験は不等間隔の補間・独立軸クランプ・不正軸・非有限値、λ/VE/dead timeの独立参照、500rpm境界と燃料分配、TDC校正・停止時の無効化、12周期待機、方式切替、予定終了保持・重複／遅刻見送り・時刻周回・停止時の予約破棄を検証する。
sensor試験はHALをスタブ化し、4チャネルのハンドル・保存先の対応、設定／開始／変換待ち／停止の各失敗時の無効化、TPS未取得を水温で代用しないことを検証する。
実波形・実センサ・タイマIRQの精度は検証範囲に含まない。


`ecu_regression`は追加コードのレビューで判明した条件を再現する。初回位相未確定の500rpm以上→未満で待機をリセットすること、無効入力／位相喪失の追加確認状態を保持すること、燃料計算の有限入力による中間overflowと最終underflowの扱い、NULL設定時の結果消去、微小TDCオフセットで角度が720度にならないことを確認する。


`model_boundary`はテーブルの軸／格子数のバイト数overflowを配列走査より前に拒否することと、予定終了の遅刻通知、同じ呼出しに含まれる正常開始との共存、時刻周回時の終了遅刻を検証する。
検査付きの実行例（GCC、Linux）:

```sh
ASAN_OPTIONS=detect_leaks=0 CC='cc -O2 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all' python3 tests/run_host_tests.py
```

`detect_leaks=0`はptrace下でLeakSanitizerが動作しない環境向け。AddressSanitizerの境界外アクセス検査と未定義動作の検査は有効のまま。これはリーク検査の実施を意味しない。
