# DOCRULES.md

## Purpose

This document defines **best-practice rules** for writing **Javadoc-style documentation comments** intended to be processed by **Doxygen** for **C/C++ (and similar)** codebases.

The goal is to produce documentation that is:

- **Accurate**
- **Consistent**
- **Searchable**
- **Readable as a reference**
- **Useful as a guide to correct usage**

Doxygen recognizes Javadoc-style blocks such as `/** ... */` and tags like `@param`, `@return`, etc.

---

## 1. Scope & Coverage Rules

### 1.1 Document these items (minimum)
You MUST document:

- Public and protected **classes / structs / interfaces**
- Public and protected **functions / methods**
- Public **enums and enum values**
- Public **typedefs / using declarations**
- Public **macros** (if part of API surface)
- Public **constants**
- Public **files/modules** (at least one file-level comment per public header)

### 1.2 Private/internal items
You SHOULD document private/internal items only when:

- The logic is non-obvious
- There is a tricky invariant or safety constraint
- The implementation is likely to be modified incorrectly without guidance

---

## 2. Comment Block Style Rules

### 2.1 Use Javadoc-style blocks for API docs
Use:

```c
/**
 * ...
 */
```

NOT:

```c
/* ... */
```

unless it is non-Doxygen commentary.

### 2.2 One summary sentence first
Every doc block MUST begin with a **brief summary sentence** that:

- Starts with a verb (for functions), or a noun phrase (for classes/files)
- Ends with a period
- Is meaningful when shown alone in an index list

Examples:

- “Parses a configuration file and returns a validated model.”
- “Represents a bounded FIFO queue with overwrite semantics.”
- “Implements the device discovery protocol.”

### 2.3 Prefer “what/why” over “how”
Docs MUST describe:

- What the item does
- Why it exists
- What contract it provides

Docs SHOULD NOT narrate the code line-by-line.

### 2.4 Keep doc blocks adjacent to the thing they document
Docs MUST be placed immediately above the declaration (preferred) or definition.

---

## 3. File-Level Documentation Rules

### 3.1 Every public header needs a file doc
At the top of each public header (or primary module file), include:

```c
/**
 * @file
 * @brief One-line purpose of this file.
 *
 * Optional longer description:
 * - what the module provides
 * - intended usage
 * - relationships to other modules
 */
```

### 3.2 Mention ownership boundaries
If the file defines a module boundary, the file docs SHOULD mention:

- The module’s responsibility
- Any dependency expectations (e.g. “no platform I/O here”)
- Thread-safety assumptions if global

---

## 4. Class / Struct Documentation Rules

### 4.1 Class docs must define responsibility and contract
A class/struct doc block MUST include:

- What the type represents
- What invariants it maintains
- Ownership / lifetime rules if relevant
- Thread-safety if relevant

Example template:

```c
/**
 * Represents a connection to the telemetry backend.
 *
 * Responsibilities:
 * - buffers outbound messages
 * - retries transient failures
 *
 * Thread-safety:
 * - Not thread-safe; external synchronization required.
 */
class TelemetryClient { ... };
```

### 4.2 Document invariants explicitly
If a type relies on invariants, docs MUST include them.

Examples:

- “capacity_ is always >= 1”
- “head_ and tail_ are modulo buffer size”
- “timestamps are monotonic increasing”

### 4.3 Avoid implementation detail leakage
Class docs SHOULD avoid exposing internal member names unless necessary.

---

## 5. Function / Method Documentation Rules

### 5.1 Function docs must describe behavior and contract
Every documented function MUST state:

- What it does (behavior)
- Preconditions (required input conditions)
- Postconditions (effects / outputs)
- Side effects (I/O, mutation, locking, allocation)
- Error handling model (return codes, exceptions, status types)

### 5.2 Use tags consistently
Use these tags as applicable:

- `@param name description`
- `@return description`
- `@throws Type condition` (C++ exceptions)
- `@warning` important hazards
- `@note` helpful clarifications
- `@see` related API

### 5.3 Parameter rules
For each `@param`, the description MUST include:

- Meaning and units (if applicable)
- Valid ranges or constraints
- Ownership expectations (if pointers/references)
- Nullability (if pointer)

Examples:

Good:
- `@param timeout_ms Timeout in milliseconds. Must be >= 0.`
- `@param out_result Output pointer. Must not be null. Caller owns storage.`

Bad:
- `@param timeout Timeout.`
- `@param x The x value.`

### 5.4 Return rules
If a function returns something, `@return` MUST state:

- What the return value represents
- Meaning of special values (e.g., `nullptr`, `-1`, `false`)
- Ownership semantics (if returning pointers)

Examples:
- “@return True if the frame was accepted, false if dropped due to backpressure.”
- “@return Pointer to internal buffer; valid until next call to read().”

### 5.5 Error reporting rules
If the API uses status codes, document them clearly:

- When each status occurs
- Which errors are recoverable
- Whether partial work can occur before error

### 5.6 Side effects and performance
Docs SHOULD mention:

- Allocation behavior (heap/no heap)
- Blocking behavior (may block, never blocks)
- Complexity (e.g. O(n))
- Threading (locks taken, reentrancy)

### 5.7 Avoid redundancy
Do NOT write comments that restate the name.

Bad:
- `/** Gets the size. */ size_t getSize();`

Better:
- “Returns the number of elements currently buffered.”
- “This count excludes dropped frames.”

---

## 6. Enum Documentation Rules

### 6.1 Enum docs must define meaning of each value
Enums MUST include:

- Meaning of each enumerator
- Any ordering assumptions
- Compatibility requirements (serialization / wire protocol)

Example:

```c
/**
 * Indicates the result of a parse operation.
 */
enum class ParseResult {
  /** Parsing succeeded with no warnings. */
  Ok,
  /** Parsing succeeded but fallback defaults were applied. */
  OkWithDefaults,
  /** Input was invalid and could not be parsed. */
  InvalidInput,
};
```

---

## 7. Ownership, Lifetime, and Nullability Rules (C/C++)

### 7.1 Explicitly state ownership transfers
If a function takes or returns a pointer/reference, docs MUST clarify:

- Who owns the memory
- Who frees it
- How long it stays valid

### 7.2 Nullability must be explicit
If a pointer can be null, say so.

Examples:
- “May be null.”
- “Must not be null.”

### 7.3 Thread-safety must be explicit
If used across threads, docs MUST state one of:

- Thread-safe
- Not thread-safe
- Conditionally thread-safe (explain conditions)

---

## 8. Formatting Rules

### 8.1 Keep line width readable
Prefer wrapping at ~80–100 columns (project dependent).

### 8.2 Use bullet lists for multi-part explanations
Prefer:

- Responsibilities:
  - …
  - …

Over long paragraphs.

### 8.3 Use code formatting sparingly
Use backticks for identifiers:

- `TelemetryClient`
- `init()`
- `Status::Timeout`

---

## 9. Cross-References & Navigation Rules

### 9.1 Use `@see` for related APIs
If a function is part of a family, include `@see`:

- factory ↔ destructor
- init ↔ shutdown
- open ↔ close
- encode ↔ decode

### 9.2 Prefer linking to the canonical entry point
Link to the most important “main” function/class for the feature.

---

## 10. “Brief vs Detailed” Rule

### 10.1 Brief summary is mandatory
The first sentence is always the brief summary.

### 10.2 Detailed description is optional but encouraged
Add detailed paragraphs when:

- the function has tricky edge cases
- the type has invariants
- misuse could cause bugs or safety hazards

---

## 11. Examples (Canonical Templates)

### 11.1 File template

```c
/**
 * @file
 * @brief Implements the transport layer for device messages.
 *
 * This module provides:
 * - framing and CRC validation
 * - retry logic for transient failures
 *
 * Thread-safety:
 * - Not thread-safe; external synchronization required.
 */
```

### 11.2 Class template

```c
/**
 * Provides bounded buffering for outgoing messages.
 *
 * This class drops the oldest message when full to ensure
 * the most recent state is transmitted.
 *
 * Invariants:
 * - capacity() >= 1
 * - size() <= capacity()
 */
class MessageQueue { ... };
```

### 11.3 Function template

```c
/**
 * Encodes a message into the wire format.
 *
 * Preconditions:
 * - `msg` must contain a valid type field.
 *
 * Side effects:
 * - Writes into `out_buf`.
 *
 * @param msg Message to encode.
 * @param out_buf Output buffer to receive bytes. Must not be null.
 * @param out_len Capacity of `out_buf` in bytes. Must be >= required size.
 * @return Number of bytes written on success, or 0 on failure.
 */
size_t encode_message(const Message& msg, uint8_t* out_buf, size_t out_len);
```

---

## 12. Review Checklist (Quick)

Before merging, verify:

- [ ] Every public API item has a doc block
- [ ] Summary sentence exists and is meaningful
- [ ] All `@param` entries exist and have real descriptions
- [ ] `@return` exists when needed and is specific
- [ ] Ownership / nullability / lifetime is documented where relevant
- [ ] Thread-safety is documented where relevant
- [ ] Side effects and error behavior are described
- [ ] Docs match current behavior (not stale)

---

## 13. Local LLM Instruction (Optional)

When generating or rewriting docs, the model MUST:

- Preserve API behavior (do not invent features)
- Prefer clarity and correctness over verbosity
- Use the templates above
- Never omit parameter documentation for public functions
- Always document ownership and nullability for pointer parameters

---

End of DOCRULES.md
