t_irc: main.c twitch_connect.o chat_queue.o commands.o message_parse.o curses_screen.o
	cc -o t_irc main.c twitch_connect.o chat_queue.o message_parse.o commands.o curses_screen.o -lpthread -lncursesw

curses_screen.o: curses_screen.c curses_screen.h twitch.h
	cc -c curses_screen.c -lpthread -lncursesw

message_parse.o: message_parse.c message_parse.h
	cc -c message_parse.c

chat_queue.o: chat_queue.c chat_queue.h
	cc -c chat_queue.c -lpthread

twitch_connect.o: twitch_connect.c twitch_connect.h twitch.h
	cc -c twitch_connect.c -lpthread

commands.o: commands.c twitch.h commands.h
	cc -c commands.c

clean:
	rm t_irc *.o

logged_twitch_connect.o: twitch_connect.c twitch_connect.h twitch.h
	cc -ggdb -D CREATE_LOGS -o logged_twitch_connect.o -c twitch_connect.c -lpthread

log_irc: main.c logged_twitch_connect.o chat_queue.o commands.o message_parse.o curses_screen.o
	cc -ggdb -D CREATE_LOGS -o log_irc main.c logged_twitch_connect.o chat_queue.o message_parse.o commands.o curses_screen.o -lpthread -lncursesw
