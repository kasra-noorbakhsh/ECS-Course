import os
import time
import tarfile
import json
from datetime import datetime

BACKUP_TARGETS = ['/etc', '/home']

def load_config():
    base_dir = os.path.dirname(os.path.abspath(__file__))
    config_path = os.path.join(base_dir, 'config.json')
    with open('config.json', 'r') as f:
        return json.load(f)

# Create a compressed archive of the given directories
def create_backup(output_dir):
    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
    backup_name = f"backup_{timestamp}.tar.gz"
    backup_path = os.path.join(output_dir, backup_name)

    os.makedirs(output_dir, exist_ok=True)

    print(f"[+] Creating backup at {backup_path}")
    with tarfile.open(backup_path, "w:gz") as tar:
        for target in BACKUP_TARGETS:
            tar.add(target, arcname=os.path.basename(target))

    print(f"[✓] Backup created successfully.")

def main():
    config = load_config()
    run_interval = config.get("run_interval", 3600)
    output_dir = config.get("output_dir", "/tmp/backups")

    print(f"[*] Backup script started. Run interval: {run_interval}s. Output dir: {output_dir}")
    
    while True:
        try:
            create_backup(output_dir)
        except Exception as e:
            print(f"[!] Backup failed: {e}")
        time.sleep(run_interval)

if __name__ == "__main__":
    main()
