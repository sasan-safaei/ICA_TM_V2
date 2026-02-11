import io
import os
import csv

import pandas as pd
import qrcode
from reportlab.lib.pagesizes import A4
from reportlab.lib.units import mm
from reportlab.lib.utils import ImageReader
from reportlab.pdfgen import canvas
from PIL import Image

EXPECTED_COLUMNS = ["Num", "EUI-M", "EUI", "Device", "Date", "Time", "Status", "Description"]


def csv_to_pdf_with_qr(csv_path, pdf_path, dateFilter="", statusFilter=""):
    main_df = _read_csv_trim_extra(csv_path, EXPECTED_COLUMNS)
    print("Current Filter:" + dateFilter)
    if dateFilter != "":
        df = main_df[main_df['Date'] == dateFilter]
    else:
        df = main_df
    columns_to_hide = ['EUI-M', 'Status', 'Description']
    visible_columns = [col for col in df.columns if col not in columns_to_hide]
    c = canvas.Canvas(pdf_path, pagesize=A4)
    width, height = A4
    row_height = 30 * mm
    top_margin = height - 30 * mm
    bottom_margin = 30 * mm
    rows_per_page = int((height - top_margin + bottom_margin) // row_height)

    y = top_margin
    itemCnt = 0
    for idx, row in df.iterrows():
        if row['Status'] != 'Defected':
            if y < bottom_margin:
                c.showPage()
                y = top_margin

            row_text = " | ".join(str(row[col]) for col in visible_columns)
            text_width = c.stringWidth(row_text, "Helvetica", 12)
            text_x = (width - text_width) / 2

            qr_size = 30 * mm
            qr_y = y - qr_size / 2
            if itemCnt % 2 == 0:
                qr_x = 20 * mm  # Left
            else:
                qr_x = width - 20 * mm - qr_size  # Right
            itemCnt += 1
            qr_value = str(row["EUI"])
            qr_img = qrcode.make(qr_value)
            qr_img = qr_img.resize((int(qr_size), int(qr_size)), Image.LANCZOS)
            qr_bytes = io.BytesIO()
            qr_img.save(qr_bytes, format="PNG")
            qr_bytes.seek(0)
            c.drawImage(ImageReader(qr_bytes), qr_x, qr_y, qr_size, qr_size)

            c.setFont("Helvetica", 12)
            c.drawString(text_x, y, row_text)

            y -= row_height

    c.save()


def _read_csv_trim_extra(file_path, expected_columns):
    with open(file_path, newline='', encoding='utf-8-sig') as csv_file:
        reader = csv.reader(csv_file)
        rows = list(reader)
    if not rows:
        raise ValueError("CSV is empty")
    header = [cell.strip() for cell in rows[0]]
    if len(header) < len(expected_columns):
        raise ValueError("CSV header has too few columns")
    header = header[:len(expected_columns)]
    if header != expected_columns:
        raise ValueError("CSV header does not match expected columns")
    data_rows = []
    for row in rows[1:]:
        if not row:
            continue
        if len(row) < len(expected_columns):
            row += [""] * (len(expected_columns) - len(row))
        else:
            row = row[:len(expected_columns)]
        data_rows.append(row)
    return pd.DataFrame(data_rows, columns=expected_columns)


def _build_arg_parser():
    import argparse

    parser = argparse.ArgumentParser(description="Convert CSV rows to a PDF with QR codes.")
    parser.add_argument("csv_path", help="Path to the input CSV file")
    parser.add_argument("pdf_path", help="Path to the output PDF file")
    parser.add_argument("--date", default="", dest="dateFilter", help="Filter by Date column")
    parser.add_argument("--status", default="", dest="statusFilter", help="Filter by Status")
    return parser


if __name__ == "__main__":
    args = _build_arg_parser().parse_args()
    csv_to_pdf_with_qr(args.csv_path, args.pdf_path, args.dateFilter, args.statusFilter)
    