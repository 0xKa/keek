import json
from pathlib import Path


def describe(path: Path) -> dict:
    stat = path.stat()
    return {"name": path.name, "size": stat.st_size, "is_dir": path.is_dir()}


if __name__ == "__main__":
    entries = [describe(entry) for entry in sorted(Path(".").iterdir())]
    print(json.dumps(entries, indent=2))
