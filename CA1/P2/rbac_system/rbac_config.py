RBAC = {
    "admin_user": {
        "role": "admin",
        "allowed_commands": ["*"]  # admin can run everything
    },
    "normal_user": {
        "role": "user",
        "allowed_commands": [
            "uptime",
            "df -h",
            "free -m",
            "ps aux",
            "top -b -n1",
	    "whoami",
	    "hostname",
        ]
    }
}

