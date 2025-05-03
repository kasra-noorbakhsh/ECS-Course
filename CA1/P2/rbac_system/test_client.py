from ssh_client import connect_ssh, run_command

HOST = "127.0.0.1"
PORT = 22
USERNAME = "admin_user"  # or "normal_user"
KEY_PATH = "/home/admin_user/.ssh/id_rsa"  # Path to private key

COMMAND = "df -h"

client = connect_ssh(HOST, PORT, USERNAME, KEY_PATH)

if client:
    run_command(client, USERNAME, COMMAND)
    client.close()
