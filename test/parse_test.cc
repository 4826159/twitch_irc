#include "gtest/gtest.h"

extern "C" {
#include "message_parse.h"
}

#include <string>

TEST(MsgTest, SimplePrefix)
{
    std::string host = "tmi.twitch.tv";
    std::string pstring = ":" + host + " ";
    MsgPrefix *prefix;
    const char *msg = pstring.c_str();
    int len = strlen(msg);

    char *pmsg = (char *)malloc(len + 1);
    char *omsg = pmsg;
    strncpy(pmsg, msg, len + 1);

    prefix = parse_message_prefix(&pmsg);

    EXPECT_STREQ(host.c_str(), prefix->host);
    EXPECT_EQ(NULL, prefix->nick);
    EXPECT_EQ(NULL, prefix->user);
    EXPECT_EQ(&omsg[1], prefix->host);
    EXPECT_FALSE(strncmp(msg, omsg, len) == 0);
    EXPECT_EQ('\0', pmsg[0]);
    EXPECT_EQ('\0', omsg[len]);
    
    delete prefix;
    delete omsg;
}

TEST(MsgTest, FullPrefix)
{
    std::string user = "valid_user";
    std::string host = "tmi.twitch.tv";
    std::string pstring = ":" + user + "!" + user + "@" + user + "." + host + " ";
    MsgPrefix *prefix;
    const char *msg = pstring.c_str();
    int len = strlen(msg);
    char *pmsg = (char *)malloc(len);
    char *omsg = pmsg;
    strncpy(pmsg, msg, len);

    prefix = parse_message_prefix(&pmsg);

    EXPECT_STREQ((user + "." + host).c_str(), prefix->host);
    EXPECT_STREQ(user.c_str(), prefix->nick);
    EXPECT_STREQ(user.c_str(), prefix->user);
    EXPECT_EQ(&omsg[1], prefix->nick);
    EXPECT_EQ(&prefix->nick[user.length() + 1], prefix->user);
    EXPECT_EQ(&prefix->user[user.length() + 1], prefix->host);
    EXPECT_FALSE(strncmp(msg, omsg, len) == 0);
    EXPECT_EQ('\0', pmsg[0]);
    EXPECT_EQ('\0', omsg[len]);
    
    delete prefix;
    delete omsg;
}

TEST(MsgTest, Param)
{
    std::string params = "valid_username :Hello!\r\n";
    char *msg = (char *)malloc(params.length() + 1);
    strncpy(msg, params.c_str(), params.length() + 1);

    MsgParam *p = parse_message_parameter(&msg);

    EXPECT_EQ(2, p->count);
    EXPECT_STREQ("valid_username", p->param[0]);
    EXPECT_STREQ("Hello!", p->param[1]);
}

TEST(MsgTest, PingM)
{
    TwitchMsg *ping;

    ping = parse_message("PING username\r\n");

    EXPECT_TRUE(NULL == ping->prefix);
    EXPECT_STREQ("PING", ping->command->name);
    EXPECT_EQ(0, ping->command->number);
    EXPECT_EQ(1, ping->params->count);
    EXPECT_STREQ("username", ping->params->param[0]);
    EXPECT_STREQ("username", ping->params->trailing);
    EXPECT_STREQ(ping->params->trailing, ping->params->param[0]);

    free_message(ping);
}

TEST(MsgTest, PingT)
{
    TwitchMsg *ping;

    ping = parse_message("PING :username\r\n");

    EXPECT_TRUE(NULL == ping->prefix);
    EXPECT_STREQ("PING", ping->command->name);
    EXPECT_EQ(0, ping->command->number);
    EXPECT_STREQ("username", ping->params->param[0]);
    EXPECT_STREQ("username", ping->params->trailing);
    EXPECT_STREQ(ping->params->trailing, ping->params->param[0]);
    EXPECT_EQ(1, ping->params->count);

    free_message(ping);
}

TEST(MsgTest, Join)
{
    std::string s1 = ":tmi.twitch.tv 001 valid_user :Welcome, GLHF!\r\n"; // twitch welcome messages
    std::string s2 = ":tmi.twitch.tv 002 valid_user :Your host is tmi.twitch.tv\r\n";
    std::string s3 = ":tmi.twitch.tv 003 valid_user :This server is rather new\r\n";
    std::string s4 = ":tmi.twitch.tv 004 valid_user :-\r\n";
    std::string s375 = ":tmi.twitch.tv 375 valid_user :-\r\n"; // RPL_MOTDSTART
    std::string s372 = ":tmi.twitch.tv 372 valid_user :You are in a maze of twisty passages.\r\n"; // RPL_MOTD
    std::string s376 = ":tmi.twitch.tv 376 valid_user :>\r\n"; // RPL_ENDOFMOTD

    TwitchMsg *m1 = parse_message(s1.c_str());

    EXPECT_EQ(NULL, m1->prefix->nick);
    EXPECT_EQ(NULL, m1->prefix->user);
    EXPECT_STREQ("tmi.twitch.tv", m1->prefix->host);
    EXPECT_STREQ("001", m1->command->name);
    EXPECT_EQ(1, m1->command->number) << "Command number";
    EXPECT_EQ(2, m1->params->count) << "Params count";
    EXPECT_STREQ("valid_user", m1->params->param[0]);
    EXPECT_STREQ("Welcome, GLHF!", m1->params->param[1]);
    EXPECT_STREQ(m1->params->trailing, m1->params->param[1]);

    TwitchMsg *m2 = parse_message(s2.c_str());

    EXPECT_EQ(NULL, m2->prefix->nick);
    EXPECT_EQ(NULL, m2->prefix->user);
    EXPECT_STREQ("tmi.twitch.tv", m2->prefix->host);
    EXPECT_STREQ("002", m2->command->name);
    EXPECT_EQ(2, m2->command->number) << "Command number";
    EXPECT_EQ(2, m2->params->count) << "Params count";
    EXPECT_STREQ("valid_user", m2->params->param[0]);
    EXPECT_STREQ("Your host is tmi.twitch.tv", m2->params->param[1]);
    EXPECT_STREQ(m2->params->trailing, m2->params->param[1]);

    TwitchMsg *m3 = parse_message(s3.c_str());

    EXPECT_EQ(NULL, m3->prefix->nick);
    EXPECT_EQ(NULL, m3->prefix->user);
    EXPECT_STREQ("tmi.twitch.tv", m3->prefix->host);
    EXPECT_STREQ("003", m3->command->name);
    EXPECT_EQ(3, m3->command->number) << "Command number";
    EXPECT_EQ(2, m3->params->count) << "Params count";
    EXPECT_STREQ("valid_user", m3->params->param[0]);
    EXPECT_STREQ("This server is rather new", m3->params->param[1]);
    EXPECT_STREQ(m3->params->trailing, m3->params->param[1]);

    TwitchMsg *m4 = parse_message(s4.c_str());

    EXPECT_EQ(NULL, m4->prefix->nick);
    EXPECT_EQ(NULL, m4->prefix->user);
    EXPECT_STREQ("tmi.twitch.tv", m4->prefix->host);
    EXPECT_STREQ("004", m4->command->name);
    EXPECT_EQ(4, m4->command->number) << "Command number";
    EXPECT_EQ(2, m4->params->count) << "Params count";
    EXPECT_STREQ("valid_user", m4->params->param[0]);
    EXPECT_STREQ("-", m4->params->param[1]);
    EXPECT_STREQ(m4->params->trailing, m4->params->param[1]);

    TwitchMsg *m375 = parse_message(s375.c_str());

    EXPECT_EQ(NULL, m375->prefix->nick);
    EXPECT_EQ(NULL, m375->prefix->user);
    EXPECT_STREQ("tmi.twitch.tv", m375->prefix->host);
    EXPECT_STREQ("375", m375->command->name);
    EXPECT_EQ(375, m375->command->number) << "Command number";
    EXPECT_EQ(2, m375->params->count) << "Params count";
    EXPECT_STREQ("valid_user", m375->params->param[0]);
    EXPECT_STREQ("-", m375->params->param[1]);
    EXPECT_STREQ(m375->params->trailing, m375->params->param[1]);

    TwitchMsg *m372 = parse_message(s372.c_str());

    EXPECT_EQ(NULL, m372->prefix->nick);
    EXPECT_EQ(NULL, m372->prefix->user);
    EXPECT_STREQ("tmi.twitch.tv", m372->prefix->host);
    EXPECT_STREQ("372", m372->command->name);
    EXPECT_EQ(372, m372->command->number) << "Command number";
    EXPECT_EQ(2, m372->params->count) << "Params count";
    EXPECT_STREQ("valid_user", m372->params->param[0]);
    EXPECT_STREQ("You are in a maze of twisty passages.", m372->params->param[1]);
    EXPECT_STREQ(m372->params->trailing, m372->params->param[1]);

    TwitchMsg *m376 = parse_message(s376.c_str());

    EXPECT_EQ(NULL, m376->prefix->nick);
    EXPECT_EQ(NULL, m376->prefix->user);
    EXPECT_STREQ("tmi.twitch.tv", m376->prefix->host);
    EXPECT_STREQ("376", m376->command->name);
    EXPECT_EQ(376, m376->command->number) << "Command number";
    EXPECT_EQ(2, m376->params->count) << "Params count";
    EXPECT_STREQ("valid_user", m376->params->param[0]);
    EXPECT_STREQ(">", m376->params->param[1]);
    EXPECT_STREQ(m376->params->trailing, m376->params->param[1]);
}
