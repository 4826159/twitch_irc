#ifndef __message_parse_h
#define __message_parse_h

#define MSG_MAX 512

// <prefix>   ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
typedef struct msg_prefix_t {
    char *nick;
    char *user;
    char *host; // servername
} MsgPrefix;

// <command>  ::= <letter> { <letter> } | <number> <number> <number>
typedef struct msg_command_t {
    char *name;
    int number;
} MsgCommand;

// <params>   ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
typedef struct msg_param_t {
    // "param" is trailing/middle, but trailing has next == NULL
    char **param;
    int count; // up to 15
    char *trailing; // points to the last element
} MsgParam;

// <message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
typedef struct twitch_msg_t {
    char *parsed;
    char *message;
    char *tag; // Tags for IRC v3, not implemented
    MsgPrefix *prefix;
    MsgCommand *command;
    MsgParam *params;
} TwitchMsg;

MsgPrefix *parse_message_prefix(char **message);
MsgCommand *parse_message_command(char **message);
MsgParam *parse_message_parameter(char **message);
TwitchMsg *parse_message(const char *message);
void free_message(TwitchMsg *msg);

#endif /* __message_parse_h */
