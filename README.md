*This project has been created as part of the 42 curriculum by rbourkai, egerin.*

# ft_irc

## Description

IRC server in C++98 using non-blocking sockets and `poll()` multiplexing. Supports multiple channels, operators, and standard IRC commands (PASS, NICK, USER, JOIN, PRIVMSG, TOPIC, KICK, INVITE, MODE).

## Instructions

### Build
```
make
```

### Run
```
./ircserv <port> <password>
```

### Example
```
./ircserv 6667 mypassword
nc localhost 6667
```

## Usage

### Authentication (Required on Connection)

Upon connection, you must authenticate in this exact order:

```
PASS <password>                          # Send the server password
NICK <nickname>                          # Set your nickname
USER <username> 0 * :<realname>          # Set username and real name
```

**Example:**
```
PASS mypassword
NICK alice
USER alice 0 * :Alice User
```

After successful authentication, you will receive: `:server 001 <nickname> :Welcome...`

### Available Commands

#### Channel Operations
```
JOIN <#channel>                          # Join or create a channel
PRIVMSG <#channel|user> :<message>       # Send message to channel or user
TOPIC <#channel> [:<new_topic>]          # View or set channel topic
QUIT                                     # Disconnect gracefully
```

#### Operator Commands (channel operators only)
```
KICK <#channel> <user> [:<reason>]       # Remove user from channel
INVITE <user> <#channel>                 # Invite user to channel
MODE <#channel> <+|-><mode> [args]       # Change channel mode
```

#### MODE Flags (Operators Only)
```
MODE <#channel> +i                       # Set channel to invite-only
MODE <#channel> -i                       # Remove invite-only
MODE <#channel> +t                       # Restrict TOPIC to operators
MODE <#channel> -t                       # Allow users to change TOPIC
MODE <#channel> +k <password>            # Set channel password
MODE <#channel> -k <password>            # Remove channel password
MODE <#channel> +o <user>                # Give operator privilege
MODE <#channel> -o <user>                # Remove operator privilege
MODE <#channel> +l <limit>               # Set user limit
MODE <#channel> -l                       # Remove user limit
```

### Test Example with nc

```
nc localhost 6667
PASS secret
NICK alice
USER alice 0 * :Alice User
JOIN #lobby
PRIVMSG #lobby :Hello everyone!
QUIT
```

## Resources

- [RFC 1459 - Internet Relay Chat Protocol](https://tools.ietf.org/html/rfc1459)
- [poll() System Call](https://man7.org/linux/man-pages/man2/poll.2.html)

## AI Usage

GitHub Copilot was used for: code architecture, bug discovery/fixing, test suite generation (102 checks), and documentation.
