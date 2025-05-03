import os
import subprocess
from logger_config import setup_logger

server_logger = setup_logger('ServerLogger', 'rbac_system/server.log')

def handle_command(command, user):
    try:
        server_logger.info(f"User '{user}' executed: {command}")
        
        # Run the command and capture output
        result = subprocess.run(command, shell=True, capture_output=True, text=True)
        output = result.stdout
        error = result.stderr

        if output:
            print(f"[Output]\n{output}")
        if error:
            print(f"[Error]\n{error}")

        return output, error

    except Exception as e:
        server_logger.error(f"Command execution error from user '{user}': {e}")
        return None, str(e)
