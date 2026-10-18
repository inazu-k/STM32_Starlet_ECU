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
