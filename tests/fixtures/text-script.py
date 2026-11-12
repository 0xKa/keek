import json
from pathlib import Path


def rate_nap(spot: str, minutes: int) -> dict:
    sunny = "window" in spot
    return {"spot": spot, "minutes": minutes, "score": minutes * (2 if sunny else 1)}


if __name__ == "__main__":
    naps = [rate_nap("window sill", 90), rate_nap("keyboard", 45), rate_nap("laundry basket", 120)]
    best = max(naps, key=lambda nap: nap["score"])
    print(json.dumps(best, indent=2))
    Path("best-nap.json").write_text(json.dumps(naps))
