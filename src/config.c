/*
Copyright (C) 2002  Erik Fears

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software

      Foundation, Inc.
      59 Temple Place - Suite 330
      Boston, MA  02111-1307, USA.

*/

#include "setup.h"

#ifdef STDC_HEADERS
#include <string.h>
#include <stdlib.h>
#endif

#include <stdio.h>
#include <time.h>

#include "config.h"
#include "log.h"
#include "misc.h"
#include "extern.h"
#include "match.h"

static void config_checkreq(void);
static void config_memfail(void);
static void add_to_list(int type, string_list *oldlist, const char *item);
static void free_list(string_list *list);
static void add_to_config(const char *key, const char *val);
static void init_lists(void);

extern int remote_is_ipv6;
extern int bindto_ipv6;

/* Global Configuration Variables */

char *CONF_SERVER		= 0;
char *CONF_PASSWORD		= 0;
char *CONF_USER			= 0;
char *CONF_NICK			= 0;
char *CONF_REALNAME		= 0;
char *CONF_OPER			= 0;
char *CONF_OPER_MODES		= 0;
char *CONF_SCANIP		= 0;
char *CONF_BINDIRC		= 0;
char *CONF_BINDSCAN		= 0;
char *CONF_CHANNELS		= 0;
char *CONF_KEYS			= 0;
char *CONF_NICKSERV_IDENT	= 0;
char *CONF_CHANSERV_INVITE	= 0;
char *CONF_KLINE_COMMAND	= 0;
char *CONF_DNSBL_ZONE		= 0;
char *CONF_DNSBL_FROM		= 0;
char *CONF_DNSBL_TO		= 0;
char *CONF_SENDMAIL		= 0;
char *CONF_HELP_EMAIL		= 0;
char *CONF_AWAY			= 0;
char *CONF_TARGET_STRING	= 0;
char *CONF_PIDFILE		= 0;

string_list *CONF_EXCLUDE	= 0;
string_list *CONF_SCAN_WARNING	= 0;

unsigned int CONF_SCANPORT	= 0;
unsigned int CONF_PORT		= 0;
unsigned int CONF_FDLIMIT	= 0;
unsigned int CONF_TIMEOUT	= 0;
unsigned int CONF_NEG_CACHE	= 0;

/* Configuration Hash , Hashes Config Params to their Function Handlers*/
/*      NAME                  , TYPE   , REQ, REQMET, PTR TO VAR        */
config_hash hash[] = {
       {"SERVER",              TYPE_STRING,   0,0, &CONF_SERVER          },
       {"SERVER6",	       TYPE_AF,       0,0, &CONF_SERVER          },
       {"PORT",                TYPE_INT,      1,0, &CONF_PORT            },
       {"PASSWORD",            TYPE_STRING,   0,0, &CONF_PASSWORD        },
       {"USER",                TYPE_STRING,   1,0, &CONF_USER            },
       {"NICK",                TYPE_STRING,   1,0, &CONF_NICK            },
       {"REALNAME",            TYPE_STRING,   1,0, &CONF_REALNAME        },
       {"OPER",                TYPE_STRING,   1,0, &CONF_OPER            },
       {"OPER_MODES",          TYPE_STRING,   1,0, &CONF_OPER_MODES      },
       {"SCANIP",              TYPE_STRING,   1,0, &CONF_SCANIP          },
       {"SCANPORT",            TYPE_INT,      1,0, &CONF_SCANPORT        },
       {"BINDIRC",             TYPE_STRING,   0,0, &CONF_BINDIRC         },
       {"BINDSCAN",            TYPE_STRING,   0,0, &CONF_BINDSCAN        },
       {"BINDIRC6",            TYPE_AF,       0,0, &CONF_BINDIRC         },
       {"BINDSCAN6",           TYPE_AF,       0,0, &CONF_BINDSCAN        },
       {"FDLIMIT",             TYPE_INT,      1,0, &CONF_FDLIMIT         },
       {"CHANNELS",            TYPE_STRING,   1,0, &CONF_CHANNELS        },
       {"KEYS",                TYPE_STRING,   0,0, &CONF_KEYS            },
       {"NICKSERV_IDENT",      TYPE_STRING,   0,0, &CONF_NICKSERV_IDENT  },
       {"CHANSERV_INVITE",     TYPE_STRING,   0,0, &CONF_CHANSERV_INVITE },
       {"KLINE_COMMAND",       TYPE_STRING,   1,0, &CONF_KLINE_COMMAND   },
       {"DNSBL_ZONE",          TYPE_STRING,   0,0, &CONF_DNSBL_ZONE      },
       {"DNSBL_FROM",          TYPE_STRING,   0,0, &CONF_DNSBL_FROM      },
       {"DNSBL_TO",            TYPE_STRING,   0,0, &CONF_DNSBL_TO        },
       {"SENDMAIL",            TYPE_STRING,   0,0, &CONF_SENDMAIL        },
       {"HELP_EMAIL",          TYPE_STRING,   1,0, &CONF_HELP_EMAIL      },
       {"AWAY",                TYPE_STRING,   1,0, &CONF_AWAY            },
       {"TARGET_STRING",       TYPE_STRING,   1,0, &CONF_TARGET_STRING   },
       {"EXCLUDE",             TYPE_WILDLIST, 0,0, &CONF_EXCLUDE         },
       {"TIMEOUT",             TYPE_INT,      1,0, &CONF_TIMEOUT         },
       {"PIDFILE",             TYPE_STRING,   1,0, &CONF_PIDFILE         },
       {"SCAN_WARNING",        TYPE_LIST,     0,0, &CONF_SCAN_WARNING    },
       {"NEG_CACHE",           TYPE_INT,      0,0, &CONF_NEG_CACHE       },
       {0,                     0,             0,0, 0                     },
};



/* Parse File */

void config_load(char *filename)
{
	/* 1k buffer for reading the file. */
	char line[1024];
	size_t i;
	char *key, *args;
	FILE *in;

	if (!(in = fopen(filename, "r"))) {
		log("CONFIG -> No config file found, aborting.");
		exit(1);
	}

	init_lists();
    
	/* Clear anything we have already. */
	for (i = 0; i < (sizeof(hash) / sizeof(config_hash) - 1); i++) {
		switch(hash[i].type) { 
		case TYPE_STRING:
		case TYPE_AF:
			if (( *(char**) hash[i].var))
				free(*(char**)hash[i].var);
			*(char**)hash[i].var = 0;
			break;
		case TYPE_INT:
			*(int *) hash[i].var = 0;
			break;
		case TYPE_LIST:
		case TYPE_WILDLIST:
			free_list(* (string_list **) hash[i].var);
			break;
		}
		hash[i].reqmet = 0;
	}

	while (fgets(line, 1023, in))  {
		if(line[0] == '#')
			continue;
		    
		key = strtok(line, " ");
		args = strtok(NULL, "\n");

		if (!args)
			continue;

		/* Strip leading and trailing spaces. */
		args = clean(args);
		add_to_config(key, args);
	}

	fclose(in);
	/* Check required parameters. */
	config_checkreq();
}

static void config_checkreq()
{
	size_t i;
	int errfnd;
	string_list *list;

	errfnd = 0;

	for (i = 0; i < (sizeof(hash) / sizeof(config_hash) - 1); i++) {
		if (hash[i].req && !hash[i].reqmet) {               
			log("CONFIG -> Parameter [%s] required but not "
			    "defined in config.", hash[i].key);
			errfnd++;
		} else if (OPT_DEBUG >= 3 && hash[i].reqmet) {
			switch (hash[i].type) {
			case TYPE_AF:
			case TYPE_STRING:
				log("CONFIG -> Set [%s]: %s", hash[i].key,
				    *(char**) hash[i].var);
				break;
			case TYPE_INT:
				log("CONFIG -> Set [%s]: %d", hash[i].key,
				    *(int *) hash[i].var);
				break;
			case TYPE_LIST:
			case TYPE_WILDLIST:
				for (list =
				    (*(string_list **)(hash[i].var))->next;
				    list; list = list->next) {
					log("CONFIG -> Set [%s]: %s",
					    hash[i].key, list->text, list);
				}
				break;
			}
		}
	}

	if (errfnd) {
		log("CONFIG -> %d parameters missing from config file, "
		    "aborting.", errfnd);
		exit(1);
	}
}

/*
 * Called when memory allocation somewhere returns an error.
 */

static void config_memfail(void)
{
	log("CONFIG -> Error allocating memory.");
	exit(1);
}

static void add_to_list(int type, string_list *oldlist, const char *item)
{
	string_list *p, *x, *list;

	if (OPT_DEBUG >= 4) {
		log("Adding '%s' to string_list", item);
	}

	list = malloc(sizeof(*list));
	
	if (!list)
		config_memfail();
	
	list->next = NULL;
	list->text = strdup(item);
	
	if (!list->text)
		config_memfail();

	if (type == TYPE_WILDLIST)
		collapse(list->text);

	for (p = oldlist->next, x = oldlist; p; p = p->next) {
		/*
		 * If this is a wildcard list then we do not allow
		 * duplicate entries.
		 */
		if (type == TYPE_WILDLIST &&
		    strcasecmp(list->text, p->text) == 0) {
			free(list);
			return;
		}

		x = p;
	}

	x->next = list;
}

static void free_list(string_list *list)
{
	string_list *t, *nextlist;

	if (!list)
		return;
	
	for (t = list->next; t; ) {
		nextlist = t->next;
		free(t->text);
		free(t);
		t = nextlist;
	}
} 

static void add_to_config(const char *key, const char *val)
{
	size_t i;

	if (OPT_DEBUG >= 3) {
		log("Read config line: '%s' '%s'", key, val);
	}
	
	for (i = 0; i < (sizeof(hash) / sizeof(config_hash)) - 1; i++) {
		if (OPT_DEBUG >= 4) {
			log("Comparing config_hash key '%s' against "
			    "string '%s'", hash[i].key, key);
		}

		if (strcasecmp(key, hash[i].key) == 0) {
			if (OPT_DEBUG >= 4) {
				log("Got a match, directive type is %u",
				    hash[i].type);
			}

			switch (hash[i].type) {
			case TYPE_STRING: 
				*(char**) hash[i].var = strdup(val);
				break;
			case TYPE_AF:
                                *(char**) hash[i].var = strdup(val);
				
			/*
			 * Ugly hack here. The letter E means, it is the word
			 * SERVER BINDIRC and BINDSCAN do not have an E in
			 * them.
			 * -TimeMr14C - 28.06.2002
			 */
				if (strchr(key, 'E'))
					remote_is_ipv6 = 1;
				else
					bindto_ipv6 = 1;
			/* This was required when setting ipv6 savvy values */

                                break;
			case TYPE_INT:
				*(int *) hash[i].var = atoi(val);
				break;
			case TYPE_LIST:
			case TYPE_WILDLIST:
				add_to_list(hash[i].type,
				    *(string_list **)hash[i].var, val);
				break;
			}
			hash[i].reqmet = 1;
		}
	}
}

/*
 * Initialise any linked lists we have in the config hash by creating their
 * head nodes.
 */
static void init_lists(void)
{
	size_t i;

	for (i = 0; i < (sizeof(hash) / sizeof(config_hash)) - 1; i++) {
		if (strcasecmp("EXCLUDE", hash[i].key) == 0) {
			if (!CONF_EXCLUDE)
				CONF_EXCLUDE = malloc(sizeof(*CONF_EXCLUDE));
			
			CONF_EXCLUDE->next = NULL;
			CONF_EXCLUDE->text = NULL;
		} else if (strcasecmp("SCAN_WARNING", hash[i].key) == 0) {
			if (!CONF_SCAN_WARNING) {
				CONF_SCAN_WARNING =
				    malloc(sizeof(*CONF_EXCLUDE));
			}
			
			CONF_SCAN_WARNING->next = NULL;
			CONF_SCAN_WARNING->text = NULL;
		}
	}
}
