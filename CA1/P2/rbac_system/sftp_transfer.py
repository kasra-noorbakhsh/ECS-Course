import paramiko
import os
from logger_config import setup_logger

def connect_sftp(hostname, port, username, key_path):
    try:
        key = paramiko.RSAKey.from_private_key_file(key_path)
        transport = paramiko.Transport((hostname, port))
        transport.connect(username=username, pkey=key)
        sftp = paramiko.SFTPClient.from_transport(transport)
        return sftp
    except Exception as e:
        print(f"[!] SFTP Connection failed: {e}")
        return None

sftp_logger = setup_logger('SFTPLogger', 'rbac_system/client.log')

def upload_file(sftp, local_path, remote_path):
    try:
        sftp.put(local_path, remote_path)
        sftp_logger.info(f"Uploaded {local_path} to {remote_path}")
    except Exception as e:
        sftp_logger.error(f"Upload failed from {local_path} to {remote_path} | Error: {e}")

def download_file(sftp, remote_path, local_path):
    try:
        sftp.get(remote_path, local_path)
        print(f"[✓] Downloaded {remote_path} to {local_path}")
    except Exception as e:
        print(f"[!] Download failed: {e}")

def handle_file_transfer(command_parts, hostname, port, username, key_path):
    if len(command_parts) != 4:
        print("[!] Invalid command format.")
        return

    action = command_parts[0].lower()
    local_path = command_parts[2]
    remote_path = command_parts[3]

    sftp = connect_sftp(hostname, port, username, key_path)
    if not sftp:
        return

    if action == "upload_file":
        upload_file(sftp, local_path, remote_path)
    elif action == "download_file":
        download_file(sftp, remote_path, local_path)
    else:
        print("[!] Unknown action.")

    sftp.close()
