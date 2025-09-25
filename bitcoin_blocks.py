import requests
import json
import time
import sys
from datetime import datetime
import argparse
import os

# --- Parse command line arguments ---
parser = argparse.ArgumentParser(description="Fetch Bitcoin block data from Blockstream API")
parser.add_argument(
    "limit",
    type=int,
    help="Number of blocks to fetch (starting from height 0)"
)
args = parser.parse_args()
limit = args.limit

# Output JSON file (saved next to the script)
output_file = os.path.join(os.path.dirname(__file__), "blockchain.info.json")

# Start from Genesis block
current_height = 0
all_blocks = {}

try:
    while True:
        # Stop after reaching the requested limit
        if current_height >= limit:
            break

        print(f"Fetching block {current_height}...")

        # Step 1: Get block hash by height
        block_hash_url = f"https://blockstream.info/api/block-height/{current_height}"
        response = requests.get(block_hash_url)

        if response.status_code != 200:
            print(f"Error fetching block {current_height}, stopping.")
            break

        block_hash = response.text.strip()

        # Step 2: Get block data in JSON
        block_url = f"https://blockstream.info/api/block/{block_hash}"
        block_data = requests.get(block_url).json()

        # Convert timestamp to readable UTC time
        timestamp_utc = block_data["timestamp"]
        timestamp_human_readable = datetime.utcfromtimestamp(timestamp_utc).strftime('%Y-%m-%d %H:%M:%S')

        # Prepare block info
        block_info = {
            "version": block_data.get("version", "N/A"),
            "previousHash": str(block_data.get("previousblockhash", None)),
            "merkleRoot": str(block_data.get("merkle_root", "N/A")),
            "timestamp": timestamp_human_readable,
            "timestamp_utc": timestamp_utc,
            "bits": block_data.get("bits", "N/A"),
            "difficulty": block_data.get("difficulty", "N/A"),
            "nonce": block_data.get("nonce", "N/A")
        }

        # Save block info
        all_blocks[str(block_data["height"])] = block_info
        with open(output_file, "w") as f:
            json.dump(all_blocks, f, indent=4)

        current_height += 1
        time.sleep(1)

    print(f"\n✅ Done! Saved {len(all_blocks)} blocks to {output_file}")

except KeyboardInterrupt:
    print("\nStopping process, saving data...")
    with open(output_file, "w") as f:
        json.dump(all_blocks, f, indent=4)
    print("Data saved. Exiting.")
