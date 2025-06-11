#  Pseudo-Signal Stateless-Sync Transmission Protocol (C++)
#  WIP
# ⚠️ Disclaimer: Use of AI in Project Planning

This README and development roadmap were created with guidance from OpenAI’s ChatGPT and Google's Gemini to structure and clarify the project phases. All coding, design, and implementation will be done manually without AI-generated code.

## 📌 Project Overview

This project implements a custom data obfuscation protocol. In simple terms:

*   The **Encoder** reads a plaintext file (`message.txt`), scrambles its contents using a set of time-based rules, and splits the resulting data into two bitstream files (`freqA.txt`, `freqB.txt`).
*   The **Decoder** reads these two bitstream files, applies the same rules in reverse to de-obfuscate the data, and reconstructs the original plaintext in `decoded.txt`.

This project is an educational exercise in C++ to explore bit manipulation, stateful logic, and stateless synchronization. It simulates a "pseudo-signal transmission" using local files. The goal is to create a system resilient to basic replay attacks and resistant to black-box reverse engineering, **not** to build a production-grade cryptographic system.

The core principle is a **single, parameterized algorithm**. Its behavior is modified by a "timestamp signature" (`sig`). The final version of the protocol achieves synchronization not by reading a header, but by actively "hunting" for a dynamically placed marker within the entire data stream.

---

## 📖 Data Packet Structure

The final data stream has no fixed-position header. Its conceptual components are located and assembled by the decoder based on time-based logic. The following list represents the logical order of data *after* a successful decode, not its physical order in the files.

*   **Canary Marker & Sync Header (Layer 6):** These components have a dynamic location and are not at the physical start of the files. The "Canary" is a special bit pattern whose value and position are derived from the timestamp. It acts as a homing signal. The "Sync Header," which contains the all-important `sig`, is hidden immediately after the Canary. The decoder's first job is to find the Canary to learn where to look for the `sig`.
*   **Magic Header (Layer 1):** A fixed, single-byte marker (e.g., `0xAA`). Its purpose is to provide a simple, reliable way for the decoder to verify that its de-obfuscation process has likely succeeded. If this byte is not present after decoding, the process has failed.
*   **Message Payload (Layer 1):** This is the actual user data from the original `message.txt` file, unmodified.
*   **Checksum (Layer 1):** A simple 8-bit checksum calculated from the original `Message Payload` only. It is used as a final integrity check to ensure the payload has not been corrupted.
*   **EOT Marker (Layer 3):** A fixed, 16-bit "End of Transmission" marker (e.g., `0xF0F0`). This tells the decoder to stop processing, making the protocol immune to any extra junk data appended to the files.

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

This project is built in six logical layers. Each layer represents a complete, working program. You should implement each layer fully before moving to the next, as subsequent layers explicitly build upon the features of prior ones.

### LAYER 1: The Static Proof-of-Concept
**Goal:** Create a completely predictable encoder and decoder to verify that the fundamental logic of file I/O, bit manipulation, and data packaging works correctly.

*   **1.1: Set up File I/O:** The encoder must open `message.txt` for reading and `freqA.txt` and `freqB.txt` for writing. The decoder opens `freqA.txt` and `freqB.txt` for reading and `decoded.txt` for writing.
*   **1.2: Implement Simple Bit Interleaving:** The encoder reads the entire `message.txt` into a buffer. It then iterates through every bit of that buffer. If a bit's index is even (0, 2, 4...), it is written to `freqB.txt`. If the index is odd (1, 3, 5...), it is written to `freqA.txt`. The decoder reverses this process, reading one bit from `freqA` and one from `freqB` to reconstruct the original byte stream.
*   **1.3: Add a Magic Header:** Before performing the bit interleaving, the encoder will prepend a single, constant byte (e.g., `0xAA`) to the data from `message.txt`. After the decoder reassembles the bitstream, the very first byte it checks should be this magic header.
*   **1.4: Add a Fixed XOR Mask:** To introduce basic obfuscation, every byte of the data (including the newly added Magic Header) is XORed with a fixed, constant byte (e.g., `0x55`). XORing a value with a key scrambles it, and XORing the result with the same key restores it.
*   **1.5: Add a Simple Checksum:** The encoder calculates an 8-bit checksum by summing the byte values of the original message only (not the magic header). This sum should be performed with an 8-bit unsigned integer, allowing it to naturally overflow. This single checksum byte is appended to the data stream before the XOR mask is applied.

> **🏁 Result:** A working encoder/decoder pair. The output is always the same for the same input, and the obfuscation is trivial to break, but it proves the core mechanics are sound.

---

### LAYER 2: The Dynamic Obfuscator
**Goal:** Introduce time-based variability to create a "rolling code" system where the output is different each time it's run. For this layer, the time-based key (sig) is shared via an external file to keep things simple.

*   **2.1: Introduce the `sig` and `meta.txt`:** The encoder will get the current system time (as a Unix timestamp), and from this, generate a 16-bit "signature" or `sig`. This `sig` is the key for all dynamic operations in this layer. The encoder writes this `sig` to a new file, `meta.txt`. The decoder will read `meta.txt` to get the `sig` before it starts processing the frequency files. This is a temporary "crutch" to avoid solving synchronization yet.
*   **2.2: Make Transformations Dynamic:** The fixed XOR mask from Layer 1.4 is now derived from the `sig`. For example, the new XOR key could be the lower 8 bits of the `sig` (`sig & 0xFF`).
*   **2.3: Implement Channel Role Guessing:** The decoder has no way of knowing if `freqA.txt` corresponds to the odd bits or the even bits. Therefore, the decoder must first attempt to decode by assuming the (A=odd, B=even) order. If the decoded Magic Header is incorrect, it must then try again, assuming the (A=even, B=odd) order.
*   **2.4: Implement Predictable Noise Injection:** To further obfuscate the data, the encoder will inject single "noise" bits into the output streams at predictable intervals. The positions of these noise bits are determined by the `sig`. The values of the noise bits must also be deterministic (e.g., derived from the `sig` as well), so the decoder can reliably calculate where the noise bits are and simply discard them from the stream instead of processing them.

> **🏁 Result:** A true "rolling code" system. The output files are different every second. However, it is not self-contained and depends on the insecure `meta.txt` file.

---

### LAYER 3: Eliminating External Dependencies
**Goal:** Make the protocol self-contained by removing the need for `meta.txt`. The `sig` must now be embedded directly in the data stream.

*   **3.1: Embed the `sig` as a Plaintext Header:** The encoder will now write the 16-bit `sig` as the very first thing in the transmission, before the Magic Header. It will be split according to the simple interleaving rule (e.g., the first 8 bits of the `sig` go into `freqA`, the second 8 bits go into `freqB`). The decoder's first action is now to read these initial bits from each file to reassemble the `sig`.
*   **3.2: Add an End-of-Transmission (EOT) Marker:** After the checksum, the encoder appends a fixed 16-bit marker (e.g., `0xF0F0`). When the decoder reassembles this marker, it knows the transmission is complete and can safely ignore any subsequent data. This makes the protocol robust against trailing junk data.
*   **3.3: Implement Graceful Failure:** The decoder's logic must be improved. If, after trying both channel roles (from 2.3), it fails to find the correct Magic Header, it must not create a garbage `decoded.txt` file. It should instead print a clear error message to the console (standard error) and exit with a non-zero status code.

> **🏁 Result:** A robust, self-contained protocol. The fatal flaw is that the `sig`, the key to everything, is transmitted in the clear at the very beginning of the files.

---

### LAYER 4: Advanced Stateful Routing
**Goal:** Make the decoding process highly complex and dependent on the data's content and history, turning it into a state machine whose behavior is configured by the `sig`.

*   **4.1: Design the Routing State Machine:** The decoder will no longer use a single, fixed rule for interleaving. Instead, its behavior will change dynamically based on a "state" that is modified by various events. The `sig` (still read from the plaintext header in this layer) will be used to configure the rules of this machine.
*   **4.2: Implement Event-Based Switching:** The decoder will maintain a counter for every bit it successfully assembles. Certain bits within the `sig` will act as flags to enable or disable rules. For example, if a specific flag in the `sig` is set, the decoder must check if its bit counter is a prime number. If it is, the decoder immediately switches which file it reads from next (e.g., if it just read from `freqA`, it will now read from `freqB`, regardless of the even/odd rule).
*   **4.3: Implement Pattern-Based Switching:** The decoder will maintain a sliding window of the last 12 bits it has assembled. The `sig` will be used to define a specific 12-bit target pattern. After each bit is assembled, the decoder compares its 12-bit history to this target pattern. If they match, it's a signal to immediately switch the input file being read from.
*   **4.4: Add LUT-Based Transformations:** A Lookup Table (LUT) is essentially a small array of predefined values. Create an array of four different 8-bit XOR masks. Use two bits from the `sig` as an index (0, 1, 2, or 3) to select which of these four masks will be used to obfuscate the payload. This adds another layer of dynamic behavior.

> **🏁 Result:** The protocol's internal logic is now extremely complex and stateful, making it much harder to analyze just by looking at the bitstream. However, its security is still undermined by the plaintext `sig` header.

---

### LAYER 5: Undiscoverable Sync Header
**Goal:** Obfuscate the `sig` itself. Instead of sending it in plaintext, it will be hidden within a block of decoy data at the beginning of the transmission.

*   **5.1: Implement `sig` and Validator Hiding:**
    *   **Encoder:** At the physical start of the stream, the encoder creates a `Sync Header`. It generates the `sig` from the timestamp, and also creates an 8-bit **validator byte** derived from the `sig` (e.g., `validator = (sig >> 8) ^ (sig & 0xFF)`). Then, using the timestamp as a seed, it generates a "hiding pattern"—a list of bit positions. The 24 bits of the `sig` and validator are placed at these positions, and all other positions in the header are filled with decoy bits.
*   **5.2: Implement Synchronization Search Loop:**
    *   **Decoder:** The decoder's logic for finding the `sig` is now a search. It knows the `sig` is hidden somewhere in the first ~200 bits. It creates a list of recent timestamps to check (e.g., the last 5 seconds). For each candidate timestamp, it performs the following:
        a. Calculate the `sig_guess` and `validator_guess` that would have been generated at that time.
        b. Generate the exact same `hiding pattern` the encoder would have used for that timestamp.
        c. Using this pattern, it tries to pick out the 24 specific bits from the incoming data stream, ignoring the decoys.
        d. It checks if the extracted bits match its `sig_guess` and `validator_guess`. If they do, the correct `sig` has been found, and it can proceed.
*   **5.3: Finalize Error Handling:**
    *   **Decoder:** If the decoder's search loop finishes and no valid `sig` is found among any of the recent candidate timestamps, it must exit with a clear error: "Synchronization failed. Data is corrupt or too old."

> **🏁 Result:** The `sig` is now hidden. An attacker can no longer simply read it. However, they know that all the information needed for synchronization is located at the beginning of the files, which focuses their analysis.

---

### LAYER 6: The Final Form - Dynamic Logic Probing
**Goal:** Achieve the highest level of obfuscation by removing all fixed landmarks. The `sig`'s location is now a secret, and the decoder must find a separate, dynamic "canary" just to know where to begin its search.

*   **6.1: The Dynamic Canary (Encoder):** The encoder's logic changes fundamentally. It no longer places the `Sync Header` at the physical start. Instead, it embeds two components at a location deep within the stream, determined by the timestamp:
    *   **The Canary:** This is a special bit pattern whose value is determined by applying a unique, pre-agreed mathematical process to the timestamp. This Canary acts as a "homing signal."
    *   **The Sync Block:** Immediately following the Canary's location, the encoder embeds the `sig` and validator, hidden with decoy bits using the same method as in Layer 5.
*   **6.2: The Synchronization Hunt (Decoder):** The decoder's process is now a complex, two-stage hunt. It completely replaces the logic from Layer 5.2.
    *   **Stage 1: The Canary Hunt.** The decoder starts in a "default state" (e.g., using simple A/B interleaving and no XOR mask). It creates its time window of recent timestamps. It must scan the entire combined bitstream from start to finish. For each candidate timestamp (`t_candidate`), it calculates the value the Canary should have, and searches the entire stream for a bit pattern that matches.
    *   **Stage 2: The `sig` Validation.** If Stage 1 finds a potential Canary that matches a `t_candidate`, the decoder focuses on that location. It then performs the `sig` search loop from Layer 5, but critically, it runs this search on the bits immediately following the discovered Canary, not at the start of the file. If the extracted `sig` and validator match the ones derived from `t_candidate`, then synchronization is successful.
*   **6.3: Finalizing the Decoder Logic:** If the decoder scans the entire stream and the two-stage hunt fails for all candidate timestamps, it must exit with a final, robust error message: "Synchronization failed: Undiscoverable or corrupt signal."

> **🏁 Result:** The ultimate protocol. It has no fixed starting point for an attacker to analyze. To break it, an attacker must first reverse-engineer the mathematical process that generates the Canary, then find it, and only then can they attempt to break the hidden `sig` mechanism.

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
