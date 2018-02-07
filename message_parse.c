#include "message_parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Deallocates an TwitchMsg
 * Message contains the original message with spaces
 * Parsed is a copy with \0 put in
 */
void free_message(TwitchMsg *msg)
{
    free(msg->parsed);
    free(msg->message);
    free(msg->tag);
    free(msg->prefix);
    free(msg->command);
    free(msg->params->param);
    free(msg->params);
    free(msg);
}

/*
 * Parse the <prefix> from a message
 * Can return NULL
 */
MsgPrefix *parse_message_prefix(char **message)
{
    char *prefix_msg = *message;
    MsgPrefix *p;
    size_t i = 0;

    if (prefix_msg[0] != ':')
        return NULL;

    p = malloc(sizeof(MsgPrefix));

    //i = strcspn(prefix_msg, " ");
    //prefix_msg[i] = 0;
    //*message = &prefix_msg[i + 1];
    prefix_msg = &prefix_msg[1];

    // <prefix> ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
    // if it finds now !@, the return will be equal to strlen
    i = strcspn(prefix_msg, "!@");
    if (i > strcspn(prefix_msg, " ")) {
        // <prefix> ::= <servername>
        p->nick = NULL;
        p->user = NULL;
        // <servername> ::= <host>
        p->host = prefix_msg;
    }
    else {
        // <prefix> ::= <nick> [ '!' <user> ] [ '@' <host> ]
        
        p->nick = prefix_msg;

        if (prefix_msg[i] == '!') {
            prefix_msg[i] = 0;
            i++;
            p->user = &prefix_msg[i];
            prefix_msg = &prefix_msg[i];
            i = strcspn(prefix_msg, "@");
        }
        else {
            p->user = NULL;
        }

        if (prefix_msg[i] == '@') {
            prefix_msg[i] = 0;
            i++;
            p->host = &prefix_msg[i];
            prefix_msg = &prefix_msg[i];
        }
        else {
            p->host = NULL;
        }
    }

    i = strcspn(prefix_msg, " ");
    prefix_msg[i] = 0;
    i++;
    *message = &prefix_msg[i];

    return p;
}

/*
 * Parse the command from a message.
 * message should point to <command> segment, will return a irc_command
 */
MsgCommand *parse_message_command(char **message)
{
    char *cmd_msg = *message;
    MsgCommand *cmd = malloc(sizeof(MsgCommand));
    
    size_t i = 0;

    i = strspn(cmd_msg, " ");
    cmd_msg = &cmd_msg[i];
    cmd->name = cmd_msg;
    i = strcspn(cmd_msg, " ");
    cmd_msg[i] = 0;
    i++;

    
    if ('0' <= cmd_msg[0] && cmd_msg[0] <= '9')
        cmd->number = strtol(cmd_msg, NULL, 10);
    else
        cmd->number = 0;

    *message = &cmd_msg[i];
    return cmd;
}

/*
 * Parse the <params> from a message
 * if it encounters a <crlf> returns a null
 */
MsgParam *parse_message_parameter(char **message)
{
    char *params[15];// = (char **)malloc(15 * sizeof(char *));
    int count = 0;
    char *param_msg = *message;
    size_t i = 0;
    MsgParam *param = malloc(sizeof(MsgParam));

    do {
        // RFC1459 states <space> can be 1 or more ' '
        i = strspn(param_msg, " ");
        param_msg = &param_msg[i];

        if (param_msg[0] != ':') {
            params[count] = param_msg;
            count++;
            i = strcspn(param_msg, " \r\n");
            param_msg = &param_msg[i];
            if (param_msg[0] == '\r' || param_msg[0] == '\n') {
                param_msg[0] = 0;
            }
            else if (param_msg[0] == ' ') {
                param_msg[0] = 0;
                param_msg = &param_msg[1];
            }
        }
        else {
            params[count] = &param_msg[1];
            count++;
            param_msg = strstr(param_msg, "\r\n");
            //i = strlen(param_msg) - 2;
            //param_msg = &param_msg[i];
            param_msg[0] = 0;
            param_msg[1] = 0;
        }
    } while (param_msg[0] != 0);
    
    param->param = (char **)malloc(count * sizeof(char *));
    memcpy(param->param, params, count * sizeof(char *));
    param->count = count;
    param->trailing = params[count - 1];
    /*
    param->param = param_msg;
    param->next  = NULL;

    // Middle if not ':'
    if (param_msg[0] != ':') {
        i = strcspn(param_msg, " \r\n");
        // If not \r or \n, there is another param
        if (param_msg[i] == ' ') {
            param_msg[i] = 0;
            i++;
            param->next = parse_message_parameter(&param_msg);
        }
    }
    else {
        param->param++;
        i = strcspn(param_msg, "\r\n");
    }

    */
    *message = param_msg;
    return param;
}

/*
 * Parse the IRC message
 */
TwitchMsg *parse_message(const char *message)
{
    TwitchMsg *tm = malloc(sizeof(TwitchMsg));
    tm->message = malloc(strlen(message) + 1);
    tm->parsed  = malloc(strlen(message) + 1);
    tm->tag     = NULL; // TODO: IRCv3 tags
    strcpy(tm->message, message);
    strcpy(tm->parsed, message);
    char *msg;
    size_t i;

    // Get the message
    //dequeue_line(twitch->recv_q, &tm->message);
    msg = tm->parsed;

    // Get prefix (optional)
    tm->prefix = parse_message_prefix(&msg);

    // Get command
    tm->command = parse_message_command(&msg);

    // Get parameters, at least one
    tm->params = parse_message_parameter(&msg);

    // Ignore the cr at the end
    msg[0] = 0;
    return tm;
}

