#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include "list.h"
#include "libopm/src/opm_types.h"

extern FILE *yyin;

extern char linebuf[512];
extern int  linenum;

extern void yyerror(const char *);
extern void config_load(const char *);


/* structs to hold data */

struct IRCConf
{
   char *nick;
   char *username;
   char *realname;

   char *server;
   int   port;
   char *password;

   char *vhost;

   char *oper;
   char *mode;
   char *away;

   char *connregex;
   char *kline;

   list_t *channels;   /* List of ChannelConf */
};

struct ChannelConf
{
   char *name;
   char *key;         
};

struct OptionsConf
{
   int negcache;
   char *pidfile;
};

struct UserConf
{
   list_t *masks;    /* List of char *    */
   list_t *scanners; /* List of char *    */
};

struct ScannerConf
{
   char   *name;

   list_t *protocols;
   char   *vhost;

   int     fd;

   char   *target_ip;
   int     target_port;

   int     timeout;
   int     max_read;

   list_t  *target_string;
};

struct ProtocolConf
{
   int type;
   int port;
};

struct OpmConf
{
   list_t *blacklists;
   char   *dnsbl_from;
   char   *dnsbl_to;
   char   *sendmail;
};

/* Extern to actual config data declared in config.c */
extern struct IRCConf *IRCItem;
extern struct OptionsConf *OptionsItem;
extern struct OpmConf *OpmItem;
extern list_t *UserItemList;
extern list_t *ScannerItemList;

#endif /* CONFIG_H */
