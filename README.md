# ft_irc

An IRC server written in C++98 as part of the 42 curriculum.
Implements non-blocking sockets with `poll()` multiplexing, supporting multiple clients,
channels, operator privileges, and standard IRC commands.

## Technologies

- C++98
- POSIX sockets (`socket`, `bind`, `listen`, `accept`)
- `poll()` for I/O multiplexing
- `fcntl()` for non-blocking file descriptors

## Build

```
make
```

## Usage

```
./ircserv <port> <password>
```

Connect with any IRC client or with `nc`:

```
nc localhost 6667
PASS mypassword
NICK alice
USER alice 0 * :Alice User
JOIN #lobby
PRIVMSG #lobby :Hello everyone!
QUIT
```

Authentication must follow the order: `PASS`, `NICK`, `USER`.

## Supported Commands

| Command | Description |
|---------|-------------|
| `PASS <password>` | Authenticate with the server password |
| `NICK <nickname>` | Set or change nickname |
| `USER <user> 0 * :<real>` | Set username and real name |
| `JOIN <#channel>` | Join or create a channel |
| `PRIVMSG <target> :<msg>` | Send a message to a channel or user |
| `TOPIC <#channel> [:<topic>]` | View or set channel topic |
| `KICK <#channel> <user> [:<reason>]` | Remove a user from a channel (operator) |
| `INVITE <user> <#channel>` | Invite a user to a channel (operator) |
| `MODE <#channel> <+\|-><flag> [arg]` | Change channel mode (operator) |
| `QUIT` | Disconnect from the server |

### Mode Flags

- `+i` / `-i` -- invite-only
- `+t` / `-t` -- restrict TOPIC changes to operators
- `+k` / `-k <password>` -- channel password
- `+o` / `-o <user>` -- grant or revoke operator status
- `+l` / `-l <limit>` -- set or remove user limit

## Authors

rbourkai, egerin
