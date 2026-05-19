# chml.c — ChatML Message Wrapper

## Overview
Reads text from standard input and wraps it in a single ChatML message (`<|im_start|>role\ncontent\n<|im_end|>`). Supports system, assistant, and user roles. Pure standard C library with no external dependencies.

## Architecture
Three-file split following kclib conventions. `chml.h` exposes the opaque `kc_chml_t` type, role constants, and five public functions. `libchml.c` implements the library — string formatting, role management, and memory allocation. `chml.c` is the CLI layer — parses argv, reads stdin, calls libchml functions, and prints to stdout.

## Directory Layout
| Path | Contents |
|------|----------|
| `src/chml.h` | Public API — role constants, function declarations |
| `src/libchml.c` | Library implementation — context management, ChatML render |
| `src/chml.c` | CLI entry point — argv parsing, stdin read, error handling |
| `Makefile` | Cross-compilation builder via CMake + Ninja |
| `CMakeLists.txt` | CMake project definition (C11, static + shared + exe) |
| `test.sh` | Shell test suite — flags, role validation, output correctness |
| `README.md` | Project documentation and usage examples |
| `LICENSE` | GPL v3.0 |
| `.kcsignore` | KCS exclusion list |

## Data Model
### Internal Structures
| Symbol | Type | Role |
|--------|------|------|
| `kc_chml_t` (opaque) | `struct kc_chml` | Allocated context with role field |
| `struct kc_chml` | `{ int role; }` | Internal — tracks current role constant |

### Role Constants
| Constant | Value | String |
|----------|-------|--------|
| `KC_CHML_ROLE_SYSTEM` | 0 | `system` |
| `KC_CHML_ROLE_ASSISTANT` | 1 | `assistant` |
| `KC_CHML_ROLE_USER` | 2 | `user` |

## Public API
| Function | Returns | Description |
|----------|---------|-------------|
| `kc_chml_open()` | `kc_chml_t *` | Create context, default role = user; returns NULL on OOM |
| `kc_chml_set_role(ctx, role)` | `int` | Set role; invalid role returns `KC_CHML_ERROR` |
| `kc_chml_get_role(ctx)` | `int` | Return current role constant; `KC_CHML_ERROR` if NULL |
| `kc_chml_get_role_name(ctx)` | `const char *` | Return role string or NULL on error |
| `kc_chml_render(ctx, content)` | `char *` | Allocate and format `"<\|im_start\|>%s\n%s\n<\|im_end\|>"`; caller frees |
| `kc_chml_close(ctx)` | `void` | Free context; safe on NULL |

## CLI
| Argument | Description |
|----------|-------------|
| `-r`, `--role ROLE` | Message role: system, assistant, or user |
| `-h`, `--help` | Print usage and exit 0 |
| `-v`, `--version` | Print version (`chml 1.0.0`) and exit 0 |

### Exit Codes
| Code | Meaning |
|------|---------|
| 0 | Success (or empty stdin) |
| 1 | Error (unknown option, missing value, invalid role, OOM, read failure) |

## Build
| Target | Description |
|--------|-------------|
| `make` (default: `native`) | Build for host arch/platform |
| `make all` | Build full cross-compilation matrix |
| `make test` | Run `sh test.sh` |
| `make clean` | Remove `.build/` |

## Error Handling
| Condition | Stderr Message | Exit Code |
|-----------|----------------|-----------|
| Unknown option | `chml: unknown option '<opt>'` | 1 |
| Missing role value | `chml: missing value for --role` | 1 |
| Invalid role | `chml: invalid role '<name>'` | 1 |
| Stdin read failure | `chml: failed to read stdin` | 1 |
| Out of memory | `chml: out of memory` | 1 |
| Render failure | `chml: render failed` | 1 |

## Constraints
- Rendered string is heap-allocated; caller must `free()`.
- Empty stdin produces no output and exits 0.
- No thread-safety guarantees on the context object.
- Input content is treated as a plain string; no escaping or encoding transformations are applied.
