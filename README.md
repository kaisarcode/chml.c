# chml.c - ChatML Message Wrapper

`chml` reads text from standard input and wraps it in a single ChatML message. It supports the `system`, `assistant`, and `user` roles, defaulting to `user`.

---

## CLI

### Examples

```bash
echo 'Hello' | ./bin/x86_64/linux/chml
```

```bash
echo 'You are a helpful assistant.' | ./bin/x86_64/linux/chml --role system
```

### Output

```
<|im_start|>user
Hello
<|im_end|>
```

### Parameters

| Flag | Description |
| :--- | :--- |
| `-r`, `--role ROLE` | Message role: `system`, `assistant`, or `user` |
| `-h`, `--help` | Show help and usage |
| `-v`, `--version` | Show version |

---

## Public API

```c
#include "chml.h"

kc_chml_t *ctx = kc_chml_open();
kc_chml_set_role(ctx, KC_CHML_ROLE_SYSTEM);

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

```bash
make clean && make
```

Compiled artifacts are generated under `bin/{arch}/{platform}/`.

### Multiarch Builds

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

## License

[![GPLv3](https://www.gnu.org/graphics/gplv3-127x51.png)](https://www.gnu.org/licenses/gpl-3.0.html)

This project is distributed under the **GNU General Public License version 3 (GPLv3)**.
