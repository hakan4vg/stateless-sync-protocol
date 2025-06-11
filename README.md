#  Pseudo-Signal Stateless-Sync Transmission Protocol (C++) 
#  WIP
# ⚠️ Disclaimer: Use of AI in Project Planning

This README and development roadmap were created with guidance from OpenAI’s ChatGPT and Google's Gemini 2.5 Preview (06-05) solely to structure and clarify the project phases. All coding, design, and implementation will be done manually without AI-generated code.

## 📌 Project Overview

This project implements a custom data obfuscation protocol. In simple terms:

*   The **Encoder** reads a plaintext file (`message.txt`), scrambles its contents using a set of time-based rules, and splits the resulting data into two bitstream files (`freqA.txt`, `freqB.txt`).
*   The **Decoder** reads these two bitstream files, applies the same rules in reverse to de-obfuscate the data, and reconstructs the original plaintext in `decoded.txt`.

This project is an educational exercise in C++ to explore bit manipulation, stateful logic, and stateless synchronization. It simulates a "pseudo-signal transmission" using local files. The goal is to create a system resilient to basic replay attacks, similar in spirit to rolling codes, **not** to build a production-grade cryptographic system.

The core principle is a **single, parameterized algorithm**. Its behavior is modified by a "timestamp signature" (`sig`). The final version of the protocol achieves synchronization by hiding the `sig` within the data streams, requiring the decoder to intelligently discover it by searching a narrow window of recent timestamps.

Development is structured into five distinct layers. Each layer represents a complete, working program, with each subsequent layer adding a significant new feature.

---

## 📖 Data Packet Structure

To eliminate ambiguity, this is the final structure of the data payload after all layers are complete. The components are added in different layers.

`[Sync Header] [Magic Header] [Message Payload] [Checksum] [EOT Marker]`

*   **Sync Header (Layer 5):** A hidden, variable-length block at the start containing the interleaved `sig` and a validator byte, mixed with decoy bits.
*   **Magic Header (Layer 1):** A fixed 1-byte marker (e.g., `0xAA`) used to validate a successful decode.
*   **Message Payload (Layer 1):** The actual user data from `message.txt`.
*   **Checksum (Layer 1):** An 8-bit checksum of the original `Message Payload` only.
*   **EOT Marker (Layer 3):** A fixed 16-bit "End of Transmission" marker (e.g., `0xF0F0`).

---

## 🛠️ Prerequisites & Build Instructions

**Requirements:**
*   A C++17-compatible compiler (e.g., `g++`, `clang++`).
*   Standard headers: `<iostream>`, `<fstream>`, `<vector>`, `<string>`, `<ctime>`, `<cstdint>`, `<numeric>`, `<algorithm>`.

**Build Commands:**
```bash
# Compile the encoder
g++ -std=c++17 encoder.cpp -o encoder

# Compile the decoder
g++ -std=c++17 decoder.cpp -o decoder
```

---

## 🎯 Usage Example

1.  **Prepare Input:**
    ```bash
    echo "Hello, World!" > message.txt
    ```
2.  **Run Encoder:**
    ```bash
    ./encoder
    ```
3.  **Run Decoder:**
    ```bash
    ./decoder
    ```
4.  **Verify:**
    ```bash
    diff message.txt decoded.txt && echo "✅ Success: Messages match." || echo "❌ Failure: Messages do not match."
    ```

---

## 🔁 Development Layers & Steps

This project is built in five logical layers. Implement each layer completely before moving to the next. Higher layers explicitly build upon features from lower layers (e.g., Magic Header, Checksum).

### LAYER 1: The Static Proof-of-Concept
**Goal:** Create a predictable encoder/decoder that proves the basic file I/O and bit manipulation logic works.

*   **1.1: Set up File I/O:** Your program will read `message.txt` and write to `freqA.txt`, `freqB.txt`, and `decoded.txt`.
*   **1.2: Implement Simple Bit Interleaving:** Convert the entire message to one continuous stream of bits. Split this stream by writing the 1st, 3rd, 5th, etc., bits (odd indices) to `freqA` and the 0th, 2nd, 4th, etc., bits (even indices) to `freqB`. The decoder reverses this.
*   **1.3: Add a Magic Header:** Prepend a fixed 1-byte marker like `0xAA` to the message payload. The decoder uses this to confirm a successful decode.
*   **1.4: Add a Fixed XOR Mask:** Obfuscate the data (including the Magic Header) by XORing every byte with a constant like `0x55`.
*   **1.5: Add a Simple Checksum:** Append an 8-bit sum of the original message bytes (payload only) for integrity checking.

> **🏁 Result:** A working, but completely predictable, encoder/decoder.

---

### LAYER 2: The Dynamic Obfuscator
**Goal:** Introduce time-based variability ("rolling code"). For this layer, we will pass the `sig` in a separate `meta.txt` file for simplicity.

*   **2.1: Introduce the `sig` and `meta.txt`:** The encoder gets the current time, calculates a 16-bit `sig`, and writes it to `meta.txt`. The decoder reads this file to get the `sig`.
*   **2.2: Make Transformations Dynamic:** Replace the fixed XOR mask from 1.4 with one derived from the `sig` (e.g., `mask = sig & 0xFF`). Add other `sig`-driven transformations.
*   **2.3: Implement Channel Role Guessing:** The decoder tries decoding in both (A, B) and (B, A) order, using the Magic Header from 1.3 to determine the correct one.
*   **2.4: Implement Predictable Noise Injection:** The encoder inserts deterministic junk bits into the streams at positions determined by the `sig`. The noise bit itself should be predictable (e.g., `(sig >> bit_index) & 1`) to avoid corrupting the stream. The decoder uses the `sig` to know where the noise bits are and discard them.

> **🏁 Result:** A true "rolling code" system where the output is different every second. It relies on the `meta.txt` crutch for synchronization.

---

### LAYER 3: Eliminating External Dependencies
**Goal:** Remove the `meta.txt` crutch. The protocol must now work with just the two frequency files by explicitly embedding the `sig` in the data stream.

*   **3.1: Embed the `sig` as a Plaintext Header:**
    *   **Encoder:** At the very beginning of the transmission (before the Magic Header), write the 16 bits of the `sig` plainly, split across the two files (e.g., first 8 bits to `freqA`, second 8 bits to `freqB`).
    *   **Decoder:** Before doing anything else, read the first bits from each file to reassemble the `sig`. Now it has the "key" for the rest of the message.
*   **3.2: Add an End-of-Transmission (EOT) Marker:**
    *   **Encoder:** After the Checksum, append a fixed 16-bit marker like `0xF0F0`.
    *   **Decoder:** Stop reading as soon as this marker is detected. This makes the protocol immune to any trailing junk data.
*   **3.3: Implement Graceful Failure:**
    *   **Decoder:** If the Magic Header is not found after trying both channel roles, the program must exit with a clear error message ("Error: Invalid data.") and not create a garbage `decoded.txt` file.

> **🏁 Result:** A robust protocol that no longer requires a separate metadata file. This is a major milestone in self-containment.

---

### LAYER 4: Advanced Stateful Routing
**Goal:** Evolve the core decryption logic from simple interleaving into a complex, content-aware state machine.

*   **4.1: Design the Routing State Machine:** The decoder will now operate based on complex rules. The `sig` (which is still read from the plaintext header in this layer) provides the parameters for this state machine.
*   **4.2: Implement Event-Based Switching:** Implement rules enabled by flags in the `sig`.
    *   **Example (Prime Number Rule):** If a certain bit in `sig` is 1, switch the channel being read from every time the total number of bits decoded so far is a prime number. *(Note: For this project, a simple trial division primality test is sufficient, as bit counts will be manageable.)*
*   **4.3: Implement Pattern-Based Switching:** Implement rules that react to data content.
    *   **Example (Dynamic Pattern Rule):** Use a few bits of `sig` to select one of four predefined 12-bit patterns. When the decoder sees this specific pattern in the combined bitstream, it's a signal to immediately switch to the other frequency file.
*   **4.4: Add LUT-Based Transformations:** Increase the variety of obfuscation by creating a small list (a Lookup Table, or LUT) of four different XOR masks. Use two bits from the `sig` to choose which mask from the list to use.

> **🏁 Result:** The protocol's internal logic is now extremely complex and stateful, making it much harder to analyze from the outside.

---

### LAYER 5: Undiscoverable Synchronization
**Goal:** The final evolution. The `sig` is no longer sent plainly; it is hidden. The decoder must find the `sig` by solving a time-based puzzle.

*   **5.1: Implement the `sig` and Validator Hiding (Encoder):**
    *   **Encoder:** Instead of writing the `sig` plainly as in Layer 3.1, create a `Sync Header`.
        1.  Generate the `sig` from the timestamp.
        2.  Create an 8-bit **validator byte** derived purely from the `sig` (e.g., `validator = (sig >> 8) ^ (sig & 0xFF)`).
        3.  Use the timestamp to generate a unique **hiding pattern**. This pattern dictates how the bits of the `sig` and `validator` are interleaved with deterministic decoy bits at the very start of the transmission.
*   **5.2: Implement the Synchronization Search Loop (Decoder):**
    *   **Decoder:** This replaces the simple header read from Layer 3.1.
        1.  **Create a Time Window:** Get the current time (`t_now`) and create a list of recent timestamps to check (e.g., the last 5 seconds).
        2.  **Start the Search Loop:** For each `t_candidate` in the window:
            a. Calculate the `sig_guess` and `validator_guess` that would have been generated at that time.
            b. Generate the exact `hiding pattern` that would have been used at `t_candidate`.
            c. Use this pattern to *try* and read the `sig` and `validator` bits from the start of the frequency files, skipping the decoys.
            d. **Validate:** If the extracted `sig` and `validator` match the `sig_guess` and `validator_guess`, the correct `sig` has been found. Break the loop and use this `sig` to decode the rest of the message using the logic from Layer 4.
*   **5.3: Finalize Error Handling:**
    *   **Decoder:** If the search loop finishes and no valid `sig` is found, exit with a clear error ("Synchronization failed. Data is corrupt or too old.")

> **🏁 Result:** The ultimate protocol. It synchronizes without any explicit key exchange and is highly resistant to reverse-engineering from the signal alone.

---
## 📜 License
This project is licensed under the MIT License. See the `LICENSE` file for details.

---
## 🤝 Contributing
This is a personal educational project. While contributions are not actively sought, feedback and suggestions for improvement are always welcome. Please feel free to open an issue to discuss any ideas.

---
## 🛡️ Security Notes
*   **Educational Use Only:** This protocol is an exercise in obfuscation and protocol design, not a substitute for peer-reviewed cryptography.
*   **Replay Resistance:** The time-based signature ensures a captured transmission cannot be used to decode a *different, future* transmission. It does not prevent an attacker from replaying a complete, captured set of files to the decoder immediately after capture, before the time window expires.
*   **Security Through Obscurity:** The resilience of this system depends on the secrecy and complexity of the algorithm. An attacker with the source code can reverse-engineer it. The goal is to make reverse-engineering from the signal alone computationally infeasible for a casual attacker.
