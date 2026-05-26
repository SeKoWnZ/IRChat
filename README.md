*This project has been created as part of the 42 curriculum by jose-gon, sarajime*

# ft_irc

## Description

`ft_irc` is a custom Internet Relay Chat (IRC) server developed in C++ as part of the 42 curriculum.

The goal of this project is to recreate the core behavior of a real IRC server using low-level socket programming and an event-driven architecture based on `poll()`.

The server supports multiple simultaneous clients, user registration, private messaging, channels, operators, and several IRC commands inspired by the IRC protocol.

Main implemented features include:

- Non-blocking TCP sockets
- Multi-client handling with `poll()`
- User authentication with password
- Nickname and username registration
- Private messages between users
- Channel creation and management
- Operator permissions
- Channel modes:
  - Invite-only (`+i`)
  - Topic restriction (`+t`)
  - Channel key/password (`+k`)
  - User limit (`+l`)
  - Operator management (`+o`)
- Commands:
  - PASS
  - NICK
  - USER
  - JOIN
  - PART
  - PRIVMSG
  - INVITE
  - TOPIC
  - MODE
  - KICK
  - QUIT

---

## Instructions

### Compilation

Clone the repository and compile the project using `make`.

```bash
git clone <repository_url>
cd ft_irc
make
```

### Execution

Run the server with:

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 mypassword
```

---

## Usage

You can connect to the server using an IRC client such as:

- HexChat
- netcat (for testing)

Example with `nc`:

```bash
nc localhost 6667
```

Then authenticate:

```irc
PASS mypassword
NICK user1
USER user1 0 * :realname
```

---

## Technical Overview

The server uses:

- `poll()` for asynchronous I/O multiplexing
- Non-blocking sockets
- Dynamic client and channel management
- Custom command parsing
- Manual IRC message formatting

The project was developed following the constraints of the 42 subject:

- C++98 standard
- No external libraries
- Single-threaded server
- One poll loop handling all clients

---

## Resources

### Documentation

- RFC 1459 — Internet Relay Chat Protocol
- Beej's Guide to Network Programming
- Linux man pages:
  - socket(2)
  - poll(2)
  - send(2)
  - recv(2)

### Tutorials and References

- https://modern.ircdocs.horse/
- https://beej.us/guide/bgnet/
- https://cplusplus.com/
- https://man7.org/linux/man-pages/

### AI Usage

AI tools were used during the development of this project for:

- Understanding IRC protocol behavior
- Debugging networking issues
- Reviewing socket and `poll()` usage
- Improving code structure and readability
- Generating test cases and edge-case analysis

All design decisions, implementation, debugging, and final code integration were completed manually by the project authors.

---

## Notes

This project is an educational implementation and does not aim to fully replicate a production-grade IRC server.

Its purpose is to explore:

- network programming
- asynchronous I/O
- protocol design
- state management
- low-level systems programming in C++