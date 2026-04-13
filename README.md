*This project has been created as part of the 42 curriculum by rbourkai, egerin.*

# ft_irc

## Description

ft_irc is an IRC server written in C++98. The goal is to implement a functional IRC server that handles multiple simultaneous clients, channel management, operator privileges, and a subset of standard IRC commands — all using a single `poll()` call for non-blocking I/O multiplexing.

The server is compatible with standard IRC clients (irssi, weechat, HexChat).

## Instructions

**Compile:**
```bash
make
```

**Run:**
```bash
./ircserv <port> <password>
```

Example:
```bash
./ircserv 6667 mypassword
```

**Connect with irssi:**
```
/connect 127.0.0.1 6667 mypassword
/join #channel
```

**Connect with nc (testing):**
```bash
nc -C 127.0.0.1 6667
PASS mypassword
NICK alice
USER alice 0 * :Alice
JOIN #lobby
PRIVMSG #lobby :hello
QUIT
```

The `-C` flag makes nc send `\r\n` line endings as required by the IRC protocol.

Authentication order is mandatory: `PASS` → `NICK` → `USER`.

**Test partial commands (ctrl+D sends without newline):**
```bash
nc -C 127.0.0.1 6667
com^Dman^Dd
```
The server reassembles partial data before processing any command.

## Supported Commands

| Command | Description |
|---|---|
| `PASS <password>` | Server password (must be sent first) |
| `NICK <nickname>` | Set or change nickname |
| `USER <user> 0 * :<realname>` | Set username — `0` and `*` are legacy IRC fields (mode mask and unused param) that are always ignored by the server |
| `JOIN <#channel> [key]` | Join or create a channel |
| `PART <#channel> [reason]` | Leave a channel |
| `PRIVMSG <target> :<message>` | Send a message to a channel or user |
| `TOPIC <#channel> [:<topic>]` | View or change channel topic |
| `KICK <#channel> <nick> [:<reason>]` | Remove a user from a channel (op only) |
| `INVITE <nick> <#channel>` | Invite a user (op only) |
| `MODE <#channel> <+\|-><flag> [arg]` | Change channel mode (op only) |
| `QUIT [:<reason>]` | Disconnect |

**MODE flags:**

| Flag | Description |
|---|---|
| `+i` / `-i` | Invite-only |
| `+t` / `-t` | Only operators can change topic |
| `+k <key>` / `-k` | Channel password |
| `+o <nick>` / `-o <nick>` | Grant / revoke operator |
| `+l <n>` / `-l` | Set / remove user limit |

## Resources

- [RFC 2812 — Internet Relay Chat: Client Protocol](https://datatracker.ietf.org/doc/html/rfc2812)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [poll() man page](https://man7.org/linux/man-pages/man2/poll.2.html)

**AI usage:** Claude was used during this project for the following tasks:
- Auditing the code against the 42 evaluation criteria (zero-grade triggers, poll usage, fcntl restrictions, errno rules)
- Fixing bugs: use-after-free in the read loop, missing PART command, incorrect TOPIC broadcast, empty channel cleanup after KICK/PART/QUIT
- Implementing the per-client output buffer with POLLOUT mechanism to handle suspended clients (`^Z` + flood scenario)
- Refactoring `broadcastMessage` to route through the buffered `sendToClient` instead of calling `send()` directly
- Adding inline comments across all source files to document each step
