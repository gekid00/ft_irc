# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

```bash
make          # build → produces ./ircserv
make re       # clean rebuild
make clean    # remove object files
make fclean   # remove objects + binary
```

```bash
./ircserv <port> <password>   # port must be 1025–65535
```

Manual testing with netcat (must send `\r\n` endings):
```bash
nc -C 127.0.0.1 6667
PASS mypassword
NICK alice
USER alice 0 * :Alice
JOIN #lobby
PRIVMSG #lobby :hello
```

## Language & Compiler Constraints

Strict **C++98** only (`-std=c++98 -Wall -Wextra -Werror`). No C++11 or later features: no `auto`, no range-for, no `nullptr`, no `<functional>`, no lambdas. Use `NULL`, explicit iterator types, and manual casts.

## Architecture

### Event Loop

The server is entirely **single-threaded** and uses a single `poll()` call to multiplex all file descriptors. There are no threads, no `fork()`, and no `select()`/`epoll()`. `Server::pollMonitoring()` in `src/Server.cpp` is the main loop.

`_pollfds` is the vector watched by `poll()`. It contains:
- The listen socket (always monitored for `POLLIN`)
- One entry per connected client (monitored for `POLLIN`, and `POLLOUT` when there is buffered output)

### Core Data Structures (owned by `Server`)

- `_clients`: `std::map<int, Client>` — fd → Client object
- `_channels`: `std::map<std::string, Channel>` — channel name → Channel object

Channels are created automatically on the first `JOIN` and deleted when the last member leaves (`cleanupClient` and `handlePart` prune empty channels).

### Buffered I/O

All sends go through `Server::sendToClient(fd, msg)` — **never call `send()` directly**. This appends to the client's `_outBuffer` and activates `POLLOUT` on that fd. The buffer is flushed by `Server::flushClient()` when `poll()` fires `POLLOUT`.

On the read side, `recv()` data is accumulated in `Client::_buffer`. `Client::hasLine()` / `Client::extractLine()` extract complete `\r\n`-terminated lines. This handles TCP fragmentation transparently.

### Command Dispatch

`Server::handleCommand()` (`src/Server.cpp`) tokenises the raw IRC line with `split()` (from `includes/Utils.hpp`, which handles the IRC trailing `:` parameter), uppercases the command, and dispatches:

- **Auth commands** (`PASS`, `NICK`, `USER`): handled as direct methods on `Server` — allowed before registration.
- **Simple channel commands** (`PART`, `QUIT`, `MODE`): also direct methods on `Server`.
- **Complex channel commands** (`JOIN`, `PRIVMSG`, `TOPIC`, `KICK`, `INVITE`): delegated to dedicated command classes (`Join`, `Privmsg`, `Topic`, `Kick`, `Invite`). Each class holds a `Server&` reference and an `int fd`, and exposes a single `execute()` method.

All other commands require the client to be registered; unregistered clients receive `ERR_NOTREGISTERED`.

### Registration Flow

A client must: send `PASS` (validated against server password) → then `NICK` + `USER` (in any order). `Client::tryRegister()` fires `RPL_WELCOME` once all three are satisfied. `PASS` must come before `NICK` and `USER` or those commands return `ERR_NOTREGISTERED`.

### IRC Reply Format

All numeric replies and message formats are inline functions in `includes/Replies.hpp`. Every message ends with `\r\n`. The server prefix is always the literal string `:server`.

### Channel Operator Rules

- First member to join a channel is automatically made operator.
- `MODE`, `KICK`, `INVITE` (when channel is +i) require the caller to be a channel operator.
- `TOPIC` requires operator only when the channel has `+t` mode set.

### Key Utilities (`includes/Utils.hpp`)

- `split(line)` — tokenises an IRC line, respecting the trailing `:` parameter (everything after `:` is one token including spaces).
- `split(str, delimiter)` — standard delimiter split (used for comma-separated channel/key lists).
- `toUpperCase(str)` — used to normalise command names.
- `isValidChannel(name)` — checks that a name starts with `#` or `&`.
- `cleanLine(line)` — strips trailing `\r` and `\n`.
