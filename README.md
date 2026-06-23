# chml.c - ChatML Message Wrapper

`chml` reads text from standard input and wraps it in a single message. It supports the `system`, `assistant`, and `user` roles, defaulting to `user`. The default output format is ChatML (`<|im_start|>`), with additional model template formats available through `-f` or `--format`.

---

## CLI

### Examples

```bash
echo 'Hello' | ./bin/x86_64/linux/chml
```

```bash
echo 'You are a helpful assistant.' | ./bin/x86_64/linux/chml --role system
```

```bash
echo 'Hello' | ./bin/x86_64/linux/chml --format gemma
```

```bash
echo 'Hello' | ./bin/x86_64/linux/chml --format llama
```

### Output (ChatML, default)

```
<|im_start|>user
Hello
<|im_end|>
```

### Output (Gemma)

```
<|turn|>user
Hello
<|turn|>
<|turn|>model
```

### Output (Llama)

```
<|begin_of_text|><|start_header_id|>user<|end_header_id|>

Hello<|eot_id|><|start_header_id|>assistant<|end_header_id|>

```

### Output (Mistral)

```
[INST] Hello [/INST]
```

### Output (Alpaca)

```
### Instruction:
Hello

### Response:
```

### Output (Phi)

```
<|user|>
Hello<|end|>
<|assistant|>
```

### Output (Zephyr)

```
<|user|>
Hello</s>
<|assistant|>
```

### Parameters

| Flag | Description |
| :--- | :--- |
| `-r`, `--role ROLE` | Message role: `system`, `assistant`, or `user` |
| `-f`, `--format FMT` | Output format: `chatml` (default), `qwen`, `gemma`, `llama`, `mistral`, `alpaca`, `phi`, or `zephyr` |
| `-h`, `--help` | Show help and usage |
| `-v`, `--version` | Show version |

---

## Public API

```c
#include "chml.h"

kc_chml_options_t opts = kc_chml_options_default();
opts.role = "system";
opts.format = KC_CHML_FMT_CHATML;

kc_chml_t *ctx = NULL;
if (kc_chml_open(&ctx, &opts) != KC_CHML_OK) {
    return 1;
}

char *msg = kc_chml_render(ctx, "You are a bot.");
printf("%s", msg);
free(msg);

kc_chml_close(ctx);
```

### Role Constants

| Constant | Value | String |
|----------|-------|--------|
| `KC_CHML_ROLE_SYSTEM` | 0 | `system` |
| `KC_CHML_ROLE_ASSISTANT` | 1 | `assistant` |
| `KC_CHML_ROLE_USER` | 2 | `user` |

### Format Constants

| Constant | Value |
|----------|-------|
| `KC_CHML_FMT_CHATML` | 0 |
| `KC_CHML_FMT_GEMMA` | 1 |
| `KC_CHML_FMT_LLAMA` | 2 |
| `KC_CHML_FMT_MISTRAL` | 3 |
| `KC_CHML_FMT_ALPACA` | 4 |
| `KC_CHML_FMT_PHI` | 5 |
| `KC_CHML_FMT_ZEPHYR` | 6 |

---

## Lifecycle

- `kc_chml_open()` - Creates a new ChatML context with default role `user`. Returns NULL on failure.
- `kc_chml_set_role()` - Sets the message role. Returns `KC_CHML_ERROR` on invalid role.
- `kc_chml_get_role()` - Returns the current role constant.
- `kc_chml_get_role_name()` - Returns the role name string.
- `kc_chml_render()` - Allocates and returns a ChatML-formatted string. Caller must `free()`.
- `kc_chml_close()` - Releases the context. Safe to call with NULL.

---

## Build

Compiled artifacts are generated under `bin/{arch}/{platform}/` for the host architecture running the build.

```bash
make clean && make
```

### Multiarch Builds

The project is prepared to build artifacts for multiple architectures under `bin/{arch}/{platform}/`. A plain `make` builds only the current host architecture.

```bash
make all
make x86_64/linux
make x86_64/windows
make i686/linux
make i686/windows
make aarch64/linux
make aarch64/android
make armv7/linux
make armv7/android
make armv7hf/linux
make riscv64/linux
make powerpc64le/linux
make mips/linux
make mipsel/linux
make mips64el/linux
make s390x/linux
make loongarch64/linux
```

---

## Beta Notice

This is a beta project tested only on Debian x86_64. It was created out of a personal need for these libraries, but no guarantees are provided regarding its stability or future support. You are free to test it, use it, and modify it as you please.

If you'd like to reach out, you can send an email to kaisar@kaisarcode.com. Please note that I do not accept pull requests; the goal is to avoid long-term dependency on platforms like GitHub, and I do not maintain fixed infrastructure to guarantee long-term stability for these projects.

---

## License

[![GPLv3](https://www.gnu.org/graphics/gplv3-127x51.png)](https://www.gnu.org/licenses/gpl-3.0.html)

This project is distributed under the **GNU General Public License version 3 (GPLv3)**.
