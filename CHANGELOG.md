# CHANGELOG

## v1.1.0

- Added data-driven configuration with table-driven environment variable loading.
- Added `kc_chml_options_default()`, `kc_chml_options_load_env()`, and `kc_chml_options_free()` to the public API.
- Refactored `kc_chml_open()` to take `kc_chml_options_t`.
- CLI is now decoupled from `libchml`; configuration is initialized through options, then overridden by flags.
- Env vars: `KC_CHML_ROLE`.
- Added signal listener lifecycle: `kc_chml_on_signal()`, `kc_chml_raise_signal()`, `kc_chml_listen_signals()`, `kc_chml_listen_signal()`, and `kc_chml_signal_listener()`.

## v1.0.1

- Append assistant opening tag `<|im_start|>assistant` to user messages.

## v1.0.0

- Published the stable baseline release.
- Provided ChatML message wrapping with system, assistant, and user role support.
- Supported stdin-based message input with trailing newline stripping.
- Offered a public C API with context lifecycle management.
