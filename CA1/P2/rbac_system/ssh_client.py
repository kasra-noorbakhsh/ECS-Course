import paramiko
from permissions import is_command_allowed
from sftp_transfer import handle_file_transfer
from logger_config import setup_logger

def connect_ssh(hostname, port, username, key_path):
    try:
        key = paramiko.RSAKey.from_private_key_file(key_path)

        client = paramiko.SSHClient()
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())  # Accept unknown host keys
        client.connect(hostname=hostname, port=port, username=username, pkey=key)

        print(f"[+] Connected to {hostname} as {username}")
        return client

    except paramiko.AuthenticationException:
        print("[-] Authentication failed. Check the SSH key or username.")
    except paramiko.SSHException as e:
        print(f"[-] SSH error: {e}")
    except Exception as e:
        print(f"[-] Unknown error: {e}")
    
    return None

client_logger = setup_logger('ClientLogger', 'rbac_system/client.log')

def run_command(client, user, command):
    client_logger.info(f"{user} attempting to run: {command}")
    if not is_command_allowed(user, command):
        print(f"[-] User '{user}' is not allowed to run the command: {command}")
        return
    
    if command.startswith("Upload_file") or command.startswith("Download_file"):
        handle_file_transfer(
            command.split(), 
            hostname="127.0.0.1", 
            port=22,
            username="admin_user", 
            key_path="/home/admin_user/.ssh/id_rsa"
        )

    try:
        stdin, stdout, stderr = client.exec_command(command)
        output = stdout.read().decode()
        error = stderr.read().decode()

        if output:
            print(f"[Output]\n{output}")
        if error:
            print(f"[Error]\n{error}")

    except Exception as e:
        client_logger.error(f"{user} failed to run: {command} | Error: {e}")
