#!/usr/bin/env python3
import csv
import argparse
import shutil
from collections import defaultdict
import sys


def read_last_eui_desc(csv_path, eui_col='EUI', desc_col='Desc'):
    # return set of EUI values whose last occurrence has Desc == 'NoError'
    last = {}
    with open(csv_path, newline='') as f:
        reader = csv.DictReader(f)
        for row in reader:
            e = row.get(eui_col, '').strip()
            last[e] = row.get(desc_col, '').strip()
    return {e for e, d in last.items() if d == 'NoError'}


def update_second_csv(src_path, dst_path, target_euis, eui_col='EUI', status_col='Status'):
    # Read all rows
    with open(src_path, newline='') as f:
        reader = csv.reader(f)
        rows = list(reader)
    if not rows:
        return False
    header = rows[0]
    # find columns
    try:
        eui_idx = header.index(eui_col)
    except ValueError:
        print(f"EUI column '{eui_col}' not found in {src_path}")
        return False
    try:
        status_idx = header.index(status_col)
    except ValueError:
        print(f"Status column '{status_col}' not found in {src_path}")
        return False

    # Ensure there is a next column to store previous status; if not, add one named 'PrevStatus'
    if status_idx + 1 >= len(header):
        header.append('PrevStatus')
        for i in range(1, len(rows)):
            rows[i].append('')

    # Update rows
    updated_count = 0
    for i in range(1, len(rows)):
        row = rows[i]
        # pad short rows
        if len(row) < len(header):
            row += [''] * (len(header) - len(row))
        eui = row[eui_idx].strip()
        if eui in target_euis:
            cur_status = row[status_idx].strip()
            if cur_status != 'Ok' and cur_status != 'Repaired*':
                # move current status into next column (append if existing)
                next_val = row[status_idx + 1].strip()
                if next_val:
                    row[status_idx + 1] = next_val + ';' + cur_status
                else:
                    row[status_idx + 1] = cur_status
                # set status to Repaired*
                row[status_idx] = 'Repaired*'
                updated_count += 1

    # Write to dst_path
    with open(dst_path, 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerows(rows)
    return updated_count


def main():
    parser = argparse.ArgumentParser(description='Fix save bug between two CSV files.')
    parser.add_argument('--first', default='./file/AllTest.csv', help='Path to first CSV (AllTest.csv)')
    parser.add_argument('--second', default='./file/ICA2510R12.csv', help='Path to second CSV to update(EUI-SAVED.csv)')
    parser.add_argument('--out', default=None, help='Output path for updated second CSV (defaults to overwrite)')
    parser.add_argument('--eui-col', default='EUI', help='Column name for EUI')
    parser.add_argument('--desc-col', default='Desc', help='Column name for description in first CSV')
    parser.add_argument('--status-col', default='Status', help='Column name for status in second CSV')
    args = parser.parse_args()

    # If no arguments passed (only script name), show help
    if len(sys.argv) == 1:
        parser.print_help()
        return

    first = args.first
    second = args.second
    out = args.out or second

    target_euis = read_last_eui_desc(first, eui_col=args.eui_col, desc_col=args.desc_col)
    if not target_euis:
        print('No matching EUI entries with Desc == "NoError" found in', first)
        return

    # backup
    shutil.copy2(second, second + '.bak')

    updated = update_second_csv(second, out, target_euis, eui_col=args.eui_col, status_col=args.status_col)
    if updated is False:
        print('Update failed')
    else:
        print(f'Updated {out} — modified rows: {updated}')


if __name__ == '__main__':
    main()
