# SoDaCLI Harness Development Log

## Implementation Notes

### Socket protocol
Commands are sent/received as raw `sizeof(SoDa::Command)` bytes
with a 4-byte length prefix (handled by `SoDa::UD::NetSocket::put/get`).
The CLI uses `SoDa::UD::ClientSocket` connecting to `<basename>_cmd`.
The socket is non-blocking; `get()` returns 0 when no data is waiting.

### Socket basename
Default is `/tmp/SoDa_` (matching `Params.cxx` default), giving socket
path `/tmp/SoDa__cmd`.  Override with `START --uds_name <base>`.

### START command
Forks and execs `SoDaServer` with the provided arguments.
Polls for the socket file (up to 30 seconds) before connecting.
Extracts the socket basename from `--uds_name` / `-S` in the args.

### Command parsing
Supports both explicit type indicators and auto-detection:
- `SET RX_TUNE_FREQ D 144.2e6`   (explicit double)
- `SET RX_TUNE_FREQ 144.2e6`     (auto-detected double)
- `SET RX_MODE I USB`            (explicit int with enum name)
- `SET RX_MODE USB`              (auto-detected enum -> int)
- `GET TX_STATE`                 (no params)

Enum names supported: USB LSB CW_U CW_L AM WBFM NBFM,
TX_OFF_0..TX_ON_2, EXTERNAL INTERNAL, MIC NOISE.

### Incoming display
Server REP commands received on the cmd socket are printed as:
  `<< REP TARGET ...`
using `Command::toString()`.

### Script files (RUN)
Lines starting with `#` are treated as comments and skipped.
Blank lines are skipped.  Errors stop the script (file is closed).

### Log file
All I/O is appended to `CLIHarnessLog.md` in the current working
directory.  Each session starts a new `# SoDaCLI Session` heading.
