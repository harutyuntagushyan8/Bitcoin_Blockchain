# Bitcoin Block Explorer & Nonce Finder (Educational)

This is a **small GUI application** that demonstrates how to:
- Estimate your CPU hashrate for **SHA-256**.
- Find an appropriate **Nonce** for a particular Bitcoin block.

⚠️ **Note:**  
This is **not** a fully functional Bitcoin miner.  
It is only intended for educational purposes and shows the process of finding a Nonce for a given block.

---

## Run Instructions

### 1. Download Blockchain Data
Run the Python helper script to fetch the first *N* blocks from the Bitcoin blockchain using [blockstream.info](https://blockstream.info):

```sh
python bitcoin_blocks.py 10
```

### 2. Download Blockchain Data
Open in Qt Creator, Build and Run.
Insert block number(not 0; it's genesys block) and press FindNonce
After long execution it outputs to same windows and to json file with founded values and extra information
