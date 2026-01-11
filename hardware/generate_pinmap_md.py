#!/usr/bin/env python3
"""
Generate hardware/pinmap.md from hardware/pinmap.csv

Usage:
  python hardware/generate_pinmap_md.py
"""
import csv
from datetime import datetime
from pathlib import Path

ROOT = Path(__file__).resolve().parent
CSV_PATH = ROOT / "pinmap.csv"
MD_PATH = ROOT / "pinmap.md"


def escape_cell(s: str) -> str:
    if s is None:
        return ""
    return str(s).replace("|", "\\|").strip()


def generate_md(rows, fieldnames):
    now = datetime.utcnow().strftime("%Y-%m-%d %H:%M UTC")
    lines = []
    lines.append("# Pin map — Starlet ECU")
    lines.append("")
    lines.append(f"_自動生成: {now}_")
    lines.append("")
    lines.append("以下は `hardware/pinmap.csv` から生成したボードのピン一覧です。")
    lines.append("")
    # header
    header = " | ".join([h for h in fieldnames])
    sep = " | ".join(["---" for _ in fieldnames])
    lines.append(f"| {header} |")
    lines.append(f"| {sep} |")
    for r in rows:
        cells = [escape_cell(r.get(h, "")) for h in fieldnames]
        lines.append(f"| {' | '.join(cells)} |")
    lines.append("")
    return "\n".join(lines)


def main():
    if not CSV_PATH.exists():
        print(f"CSV not found: {CSV_PATH}")
        return 1
    with CSV_PATH.open(newline='', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        rows = list(reader)
        if not rows:
            print("No rows in CSV")
            return 1
        md = generate_md(rows, reader.fieldnames)
    MD_PATH.write_text(md, encoding='utf-8')
    print(f"Wrote {MD_PATH}")
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
