#!/usr/bin/env python3
"""CMakeなしでも同一のホスト試験を実行する。生成物は一時ディレクトリへ置く。"""
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
cases = {
    'engine_sync': ['tests/engine_sync_test.c', 'Core/Src/engine_sync.c'],
    'engine_sync_service': ['tests/engine_sync_service_test.c', 'Core/Src/engine_sync.c', 'Core/Src/engine_sync_service.c'],
    'ecu_model': ['tests/ecu_model_test.c', 'Core/Src/ecu_model.c', 'Core/Src/injection_plan.c', 'Core/Src/engine_sync.c'],
    'ecu_regression': ['tests/ecu_regression_test.c', 'Core/Src/ecu_model.c', 'Core/Src/injection_plan.c', 'Core/Src/engine_sync.c'],
    'sensors': ['tests/sensor_test.c', 'Core/Src/efi_calc.c'],
}
with tempfile.TemporaryDirectory(prefix='starlet-tests-') as tmp:
    for name, sources in cases.items():
        binary = str(Path(tmp) / name)
        command = shlex.split(os.environ.get('CC', 'cc')) + [
            '-std=c11', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
            '-UNDEBUG', '-Itests/stubs', '-ICore/Inc', *sources, '-lm', '-o', binary]
        subprocess.run(command, cwd=root, check=True)
        subprocess.run([binary], check=True)
