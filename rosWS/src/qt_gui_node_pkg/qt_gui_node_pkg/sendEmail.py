import argparse
import smtplib
import time
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
from email.mime.base import MIMEBase
from email import encoders
import os

try:
    from .read_device_info import get_device_info 
    from .read_device_info import get_store_folder_file_list
except ImportError:
    from read_device_info import get_device_info
    from read_device_info import get_store_folder_file_list


# Do not import `gui_node` here to avoid circular imports. Accept versions via
# optional parameters to `send_email` instead.

    
def send_email(
    smtp_server="sslout.de",
    smtp_port=465,
    sender_email="ica-tm1@ica-service.de",
    sender_password="Start%123",
    receiver_email="",
    subject="info Mail",
    body="",
    attachment_path=None,
    attachments=None,
    use_ssl=True,
    gui_version=None,
    tm_version=None,
):
    """
    Send an email with optional attachment.
    
    Args:
        smtp_server (str): SMTP server address (e.g., 'sslout.de')
        smtp_port (int): SMTP server port (e.g., 465 for SSL, 587 for TLS)
        sender_email (str): Sender's email address
        sender_password (str): Sender's email password
        receiver_email (str): Receiver's email address
        subject (str): Email subject
        body (str): Email body text
        attachment_path (str, optional): Path to file to attach. Defaults to None.
        use_ssl (bool, optional): Use SSL (True) or TLS with starttls (False). Defaults to True.
    
    Returns:
        bool: True if email sent successfully, False otherwise
    """
    if not receiver_email:
        print("Error: Receiver email address is required.")
        return False
    try:
        device_info = get_device_info()
        # prefer values passed in; fall back to device_info or 'unknown'
        gv = gui_version if gui_version is not None else device_info.get('gui_version') if device_info else None
        tv = tm_version if tm_version is not None else (device_info.get('tm_version') if device_info else None)
        if device_info:
            device_info_text = (
                "\n\nDevice info:\n"
                f"Machine Number: {device_info.get('machine_number', 'Unknown')}\n"
                f"Interface: {device_info.get('interface', 'Unknown')}\n"
                f"IP Address: {device_info.get('ip') or 'Not assigned'}\n"
                f"MAC Address: {device_info.get('mac') or 'Not found'}\n"
                f"GUI Version: {gv or 'unknown'}\n"
                f"TM Version: {tv or 'unknown'}"
            )
        else:
            device_info_text = "\n\nDevice info: Not available"
        folderList = ""
        if not body:
            folderList = get_store_folder_file_list() or "No store folder list found."
        if folderList:
            folderList = f"\n\nResult Folder:\n{folderList}"
        body_with_device_info = f"{body}{device_info_text}{folderList}"

        # Create message
        msg = MIMEMultipart()
        msg["From"] = sender_email
        msg["To"] = receiver_email
        msg["Subject"] = subject
        msg.attach(MIMEText(body_with_device_info, "plain"))

        # Attach files if provided (single path or list)
        attach_list = []
        if attachments:
            attach_list = list(attachments)
        elif attachment_path:
            attach_list = [attachment_path]

        for ap in attach_list:
            if not ap:
                continue
            if os.path.exists(ap):
                try:
                    with open(ap, "rb") as attachment:
                        part = MIMEBase("application", "octet-stream")
                        part.set_payload(attachment.read())
                    encoders.encode_base64(part)
                    part.add_header(
                        "Content-Disposition",
                        f"attachment; filename= {os.path.basename(ap)}",
                    )
                    msg.attach(part)
                    print(f"Attached file: {ap}")
                except OSError as e:
                    print(f"Warning: Could not attach {ap}: {e}")
            else:
                print(f"Warning: File {ap} not found. Skipping attachment.")

        # Connect to server and send email
        if use_ssl:
            with smtplib.SMTP_SSL(smtp_server, smtp_port) as server:
                server.login(sender_email, sender_password)
                server.send_message(msg)
        else:
            with smtplib.SMTP(smtp_server, smtp_port) as server:
                server.starttls()
                server.login(sender_email, sender_password)
                server.send_message(msg)

        print("Email sent successfully!")
        return True
    
    except Exception as e:
        print(f"Failed to send email to {receiver_email}: {e}")
        return False


def send_email_with_retry(
    retry_interval_seconds=60,
    max_retries=None,
    stop_event=None,
    log_func=None,
    **send_kwargs,
):
    """Send email and retry every `retry_interval_seconds` until success.

    Args:
        retry_interval_seconds (int): Wait time between retries in seconds.
        max_retries (int|None): Optional retry limit. None means retry forever.
        stop_event (threading.Event|None): Optional stop signal.
        log_func (callable|None): Optional logger callback accepting one string.
        **send_kwargs: Arguments forwarded to `send_email`.

    Returns:
        tuple[bool, int]: (success, attempts)
    """
    attempts = 0
    interval = max(1, int(retry_interval_seconds or 60))

    if not (send_kwargs.get("receiver_email") or "").strip():
        msg = "Email retry aborted: receiver_email is empty."
        print(msg)
        if callable(log_func):
            log_func(msg)
        return False, attempts

    while True:
        if stop_event is not None and stop_event.is_set():
            return False, attempts

        attempts += 1
        if send_email(**send_kwargs):
            if attempts > 1:
                msg = f"Email sent successfully after {attempts} attempts."
                print(msg)
                if callable(log_func):
                    log_func(msg)
            return True, attempts

        if max_retries is not None and attempts >= int(max_retries):
            msg = f"Email send failed after {attempts} attempts (max retries reached)."
            print(msg)
            if callable(log_func):
                log_func(msg)
            return False, attempts

        msg = (
            f"Email send failed on attempt {attempts}. "
            f"Retrying in {interval} seconds..."
        )
        print(msg)
        if callable(log_func):
            log_func(msg)

        if stop_event is not None:
            if stop_event.wait(interval):
                return False, attempts
        else:
            time.sleep(interval)


def _build_arg_parser():
    parser = argparse.ArgumentParser(description="Send an email with an optional attachment.")
    parser.add_argument("--smtp-server", required=False, default="sslout.de", help="SMTP server address")
    parser.add_argument("--smtp-port", required=False, default=465, type=int, help="SMTP server port")
    parser.add_argument("--sender-email", required=False, default="ica-tm1@ica-service.de" , help="Sender email address")
    parser.add_argument("--sender-password", required=False,default="Start%123", help="Sender email password")
    parser.add_argument("--receiver-email", required=False,default="sasan.safaei@ica.de", help="Receiver email address")
    parser.add_argument("--subject", required=False,default="info Mail", help="Email subject")
    parser.add_argument("--body", required=False,default="", help="Email body")
    parser.add_argument("--attachment", dest="attachment_path", default=None, help="File to attach")
    parser.add_argument("--no-ssl", action="store_true", help="Use TLS instead of SSL")
    return parser


if __name__ == "__main__":
    args = _build_arg_parser().parse_args()
    send_email(
        smtp_server=args.smtp_server,
        smtp_port=args.smtp_port,
        sender_email=args.sender_email,
        sender_password=args.sender_password,
        receiver_email=args.receiver_email,
        subject=args.subject,
        body=args.body,
        attachment_path=args.attachment_path,
        use_ssl=not args.no_ssl,
    )

