/*
 *
 * kscd -- A simple CD player for the KDE project           
 *
 * $Id$
 * 
 * Copyright (C) 1997 Bernd Johannes Wuebben 
 * wuebben@math.cornell.edu
 *
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <netdb.h>
#include <pwd.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/utsname.h>

#include <qtextstream.h> 
#include <qfile.h>
#include <qdir.h>
#include <qfileinfo.h> 
#include <qregexp.h> 

#include <kapp.h>
#include <kdebug.h>

#include "cddb.h"
#include "version.h"

extern KApplication 	*mykapp;

void cddb_decode(QString& str);
void cddb_encode(QString& str, QStringList &returnlist);
bool cddb_playlist_decode(QStringList& playlist,QString&  value);
void cddb_playlist_encode(QStringList& playlist,QString&  value);

int Ret;
extern char cddbbasedirtext[4096];

CDDB::CDDB(char *host, unsigned short int _port, unsigned short int _timeout)
{
    hostname   = host;
    port       = _port;
    connected  = false;
    readonly   = false;
    timeout    = _timeout;
    tempbuffer = "";

    sock = 0L;
    state = INIT;

    use_http_proxy=false;
    protocol_level=1;
    // for direct connections assuming CDDB protocol level 1

//printf("cddb info: host[%s] port[%d] connected[%d] readonly[%d] timeout[%d]\n", host, port, connected, readonly, timeout);
//printf("attemping to connect to cddb...\n");
//fflush(stdout);
    //Connect handlers
    QObject::connect(&starttimer,SIGNAL(timeout()),this,SLOT(cddb_connect_internal()));
    QObject::connect(&timeouttimer,SIGNAL(timeout()),this,SLOT(cddb_timed_out_slot()));

}

CDDB::~CDDB()
{
    if(sock)
      {
	delete sock;
	sock = 0L;
      }
    timeouttimer.stop();
    starttimer.stop();
} // ~CDDB

void
CDDB::setTimeout(unsigned short int _timeout)
{
  timeout = _timeout;
} // setTimeout

unsigned short int 
CDDB::getTimeout( void )
{
  return timeout;
} // getTimeout

void 
CDDB::sighandler(int signum)
{
    signum = signum;
    
/*      if (signum == SIGALRM && connecting == true ){
      mykapp->processEvents();
      mykapp->flushX();
      signal( SIGALRM , CDDB::sighandler );
      setalarm();
*/      fprintf(stderr,"SIGALRM\n");
/*      }
    */

}

void 
CDDB::setalarm()
{
    struct itimerval  val1;
    struct timeval  tval1;
    struct timeval  tval2;

    tval1.tv_sec = 0;
    tval1.tv_usec = 100000; // 0.1 secs
    tval2.tv_sec = 0;
    tval2.tv_usec = 100000;

    val1.it_interval = tval1;
    val1.it_value    = tval2;

    setitimer(ITIMER_REAL, &val1, 0);
} // setalarm

void 
CDDB::cddbgetServerList(QString& _server)
{
    protocol = UNKNOWN;
  
    QStringList fields = QStringList::split(' ', _server);

    if (fields.count() > 0)
        hostname = fields[0];
    if (fields.count() > 1) {
        fields[1].truncate(20);
        protocol = decodeTransport(fields[1].ascii());
    }
    if (fields.count() > 2)
        port = fields[2].toInt();
    if (fields.count() > 3)
        cgi = fields[3];


    kdDebug() << "GETTING SERVERLIST\n" << endl;

    mode = SERVER_LIST_GET;

    if(protocol==CDDBHTTP)
      {
	cddb_connect_internal();
	if(connected)
	  {
	    QString cmd="sites";
	    send_http_command(cmd);
	    if(use_http_proxy)
	      {
		saved_state=SERVER_LIST_WAIT;
		state=HTTP_REQUEST;
	      } else {
		state = SERVER_LIST_WAIT;
	      }
	  }
      } else {
	starttimer.start(100,TRUE);
      }
} // cddbgetServerList

void 
CDDB::cddb_connect(QString& _server)
{
     protocol = UNKNOWN;

     QStringList fields = QStringList::split(' ', _server);

     if (fields.count() > 0)
         hostname = fields[0];
     if (fields.count() > 1) {
         fields[1].truncate(20);
         protocol = decodeTransport(fields[1].ascii());
     }
     if (fields.count() > 2)
         port = fields[2].toInt();
     if (fields.count() > 3)
         cgi = fields[3];


    mode = REGULAR;
    if(protocol==CDDBP)
      {
	// --HERE--
	starttimer.start(100,TRUE);
      } else {
	emit cddb_ready();
      }
} // cddb_connect

void 
CDDB::cddb_connect_internal()
{
    starttimer.stop();
    timeouttimer.start(timeout*1000,TRUE);

	//    kdDebug() << "cddb_connect_internal_timeout = " << timeout*1000 << "\n" << endl;

    if(sock) 
      {
		delete sock;
		sock = 0L;
      }

    // signal( SIGALRM , CDDB::sighandler );
    // setalarm();

    fprintf(stderr, "proto = %d, proxy=%s\n", protocol, use_http_proxy?"true":"false");
    if(protocol==CDDBHTTP && use_http_proxy)
      {
        fprintf(stderr, "PROX\n");
		kdDebug() << "CONNECTING TO " << proxyhost << ":" << proxyport << " ....\n" << endl;
		sock = new KSocket(proxyhost.ascii(),proxyport, timeout);
		kdDebug() << "SOCKET SET" << endl;
      } else {
		kdDebug() << "CONNECTING TO " << hostname << ":" << port << " ....\n" << endl;
		sock = new KSocket(hostname.local8Bit(),port, timeout);
		kdDebug() << "SOCKET SET" << endl;
      }
    
    //signal( SIGALRM , SIG_DFL );
	
    if(sock == 0L || sock->socket() < 0)
      {
		timeouttimer.stop();
		
		kdDebug() << "CONNECT FAILED\n" << endl;
		
		if(mode == REGULAR )
		  emit cddb_failed();      
		else // mode == SERVER_LIST_GET
		  emit get_server_list_failed();
		
		connected = false;
		return;    
      }
    
    connected = true;
    respbuffer = "";
    
    connect(sock,SIGNAL(readEvent(KSocket*)),this,SLOT(cddb_read(KSocket*)));
    connect(sock,SIGNAL(closeEvent(KSocket*)),this,SLOT(cddb_close(KSocket*)));
    sock->enableRead(true);
    
    if(protocol==CDDBHTTP)
      {
		protocol_level=4;
		state=READY;
      } else {
		protocol_level=1;
		state = INIT;
      }
    
    kdDebug() << "CONNECTED\n" << endl;
} // cddb_connect_internal

void 
CDDB::send_http_command(QString &command)
{
    QString request;
    QString prt;
    QString prot;
    QString identification;
    
    prot.setNum(protocol_level);
    identification=QString("&hello=anonymous+kde+Kscd+")+KSCDVERSION+"&proto="+prot;

    prt.setNum(port);
    QString base  = "http://"+hostname+":"+prt;

    cddb_http_xlat(command);

    if(use_http_proxy)
	request="GET "+base+cgi+"?cmd="+command+identification+" HTTP/1.0\r\n\r\n";
    else
	request="GET "+cgi+"?cmd="+command+identification+"\r\n";
    
    kdDebug() << "Sending HTTP request: " << request << endl;
    
    write(sock->socket(),request.ascii(),request.length());
    timeouttimer.stop();
    timeouttimer.start(timeout*1000,TRUE);
} // send_http_command

void 
CDDB::cddb_timed_out_slot()
{
    timeouttimer.stop();

    if(sock) 
	sock->enableRead(false);

    if( mode == REGULAR )
	  emit cddb_timed_out();
    else // mode == SERVER_LIST_GET
	  emit get_server_list_failed();
	
    state = CDDB_TIMEDOUT;
    kdDebug() << "SOCKET CONNECTION TIMED OUT\n" << endl;
    cddb_close(sock);
} // cddb_timed_out_slot

// called externally if we want to close or interrupt the cddb connection
void 
CDDB::close_connection()
{
    if(sock)
    {
	cddb_close(sock);
	sock = 0L;
    }
} // close_connection

void 
CDDB::cddb_close(KSocket *socket)
{
    timeouttimer.stop();
    disconnect(socket,SIGNAL(readEvent(KSocket*)),this,SLOT(cddb_read(KSocket*)));
    disconnect(socket,SIGNAL(closeEvent(KSocket*)),this,SLOT(cddb_close(KSocket*)));
    socket->enableRead(false);
    kdDebug() << "SOCKET CONNECTION TERMINATED\n" << endl;
    connected = false;
    if(socket)
      {
	delete socket;
	socket = 0L;
	sock = 0L;
      }
} // cddb_close

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <kstddirs.h>
#include <kglobal.h>

void 
CDDB::cddb_read(KSocket *socket)
{
    int  n;
    char buffer[CDDB_READ_BUFFER_LEN];
    
    if(socket == 0L || socket->socket() < 0)
	return;

    memset(buffer,0,CDDB_READ_BUFFER_LEN);
    n = read(socket->socket(), buffer, CDDB_READ_BUFFER_LEN-1 );
    buffer[n] = '\0';
    tempbuffer += buffer;

//    kdDebug() << "BUFFER: '" << buffer << "'" << endl;

    while(next_token())
        do_state_machine();
} // cddb_read

bool 
CDDB::next_token()
{
    int newlinepos = tempbuffer.find('\n');
    if(newlinepos != -1)
      {
	lastline    = tempbuffer.left(newlinepos);
	tempbuffer  = tempbuffer.right(tempbuffer.length() - newlinepos -1);
	return true;
      } else {
	return false;
      }
} // next_token

void 
CDDB::queryCD(unsigned long _magicID,QStringList& querylist)
{
//    if(state == DO_NOTHING)
//        return;
//    state = DO_NOTHING;
    if((sock == 0L || sock->socket() < 0) && protocol==CDDBP)
	return;

    QString str;
    title = "";
    category = "";
    magicID = _magicID;

    str = str.sprintf("cddb query %08lx %u ",magicID,querylist.count()-1);
    str += querylist.join(" ") + ' ';

    if(protocol==CDDBHTTP)
      {
	cddb_connect_internal();
	if(connected)
	  {
	    QString param = str;
	    send_http_command(param);
	    if(use_http_proxy)
	      {
		saved_state = QUERY;
		state       = HTTP_REQUEST;
	      } else {
		state  = QUERY;
	      }
	  }
      } else {
	// CDDB
	timeouttimer.stop();
	timeouttimer.start(timeout*1000,TRUE);
	str += "\n";
	kdDebug() << "strdata: " << str << "\n" << endl;
	write(sock->socket(),str.ascii(),str.length());
        state  = QUERY;
      }
} // queryCD

void 
CDDB::query_exact(QString line)
{
  int category_start = 0;
  int category_end 	 = 0;
  int magic_start 	 = 0;
  int magic_end 	 = 0;
  
  QString magicstr;
  
  category_start = line.find(" ",0,true) + 1;
  category_end = line.find(" ",category_start,true);
  category = line.mid(category_start,category_end-category_start);
  
  magic_start = category_end + 1;
  magic_end = line.find(" ",magic_start,true);
  magicstr = line.mid( magic_start, magic_end - magic_start);
  
  title = line.mid(magic_end + 1,line.length());
  
  QString readstring;
  if((sock == 0L || sock ->socket() < 0) && protocol==CDDBP)
    {
      kdDebug() << "sock = 0L!!!\n" << endl;
      return;
    }
  
  if(protocol==CDDBHTTP)
    {
      cddb_connect_internal();
      if(connected)
	{
	  readstring = QString::fromLatin1("cddb read %1 %2")
            .arg(category).arg(magicstr);
	  send_http_command(readstring);
	}
    } else {
      // CDDB
      timeouttimer.stop();
      timeouttimer.start(timeout*1000,TRUE);
      //  readstring.sprintf("cddb read %s %lx \n",category.data(),magicID);
      readstring = QString::fromLatin1("cddb read %1 %2 \n")
        .arg(category)
        .arg(magicstr);
      write(sock->socket(),readstring.ascii(),readstring.length());
    }
  
  state = CDDB_READ;
  respbuffer = "";
  sock->enableRead(true);
} // query_exact


void 
CDDB::do_state_machine()
{
  static int cddbfh = 0;
  int cddblinelen;
  
  
  kdDebug() << "STATE MACHINE: State: " << (int)state << " Got: " << lastline << "\n" << endl;
  
  switch (state)
	{
	case HTTP_HEADER:
	  
	  if(lastline.stripWhiteSpace()==QString(""))
		{
		  state=saved_state;
		  kdDebug() << "HTTP Header is done. Moving on.\n" << endl;
		}
	  break;
	  
	case HTTP_REQUEST:
        {
		// Parse response and check numeric code.
		QString code;
		QStringList fields = QStringList::split(' ', lastline);
		if (fields.count() > 1)
			code = fields[1];
		if(code == "200")
		{
			if(use_http_proxy)
			{
				state = HTTP_HEADER;
				kdDebug() << "HTTP request is OK. Reading HTTP header.\n" << endl;
			} else {
				state = saved_state;
				kdDebug() << "HTTP request is OK. Mooving on.\n" << endl;
			}
		} else {
			kdDebug() << "HTTP error: " << lastline << "\n" << endl;
			if (saved_state == SERVER_LIST_WAIT)
			{
				emit get_server_list_failed();
			}
			state = CDDB_DONE; //TODO: some error state
		}
	}
	break;
	
	case INIT:
	  kdDebug() << "case INIT == true\n" << endl;
	  if((lastline.left(3) == QString("201")) ||(lastline.left(3) == QString("200")) )
		{
		  kdDebug() << "next if == true\n" << endl;
		  QString hellostr;
		  
		  // cddb hello username hostname clientname version
		  hellostr = QString("cddb hello anonymous kde Kscd %1\n")
			.arg(KSCDVERSION);
		  kdDebug() << "hellostr: " << hellostr << "\n" << endl;
		  
		  Ret = write(sock->socket(),hellostr.ascii(),hellostr.length());
		  kdDebug() << "write() returned: " << Ret << " [" << strerror(errno) << "]\n" << endl;
		  state = HELLO;
		} else {
		  state = ERROR_INIT;	
		  cddb_close(sock);
		  kdDebug() << "ERROR_INIT\n" << endl;
		  emit cddb_failed();
		}
	  
	  respbuffer = "";
	  break;
	  
	case HELLO:
	  if(lastline.left(3) == QString("200"))
		{
		  // Negotiate protocol level
		  state = PROTO;
		  // Let's try to request protocol level 3
		  // so we'll get list of servers with protocol.
		  write(sock->socket(),"proto 3\n",8); 
		} else {
		  state = ERROR_HELLO;
		  cddb_close(sock);
		  kdDebug() << "ERROR_HELLO\n" << endl;
		  emit cddb_failed();
		}
	  
	  respbuffer = "";
	  break;
	  
	case PROTO:
	  if(lastline.left(3) == QString("201"))
		protocol_level=3;
	  else
		protocol_level=1;
	  
	  state = READY;
	  if(mode == REGULAR)
		{
		  emit cddb_ready();
		} else {
		  write(sock->socket(),"sites\n",6);
		  state = SERVER_LIST_WAIT;
		}
	  break;
	  
	case QUERY:
	  if(lastline.left(3) == QString("200"))
		{
		  query_exact(lastline);
		} else {
		  if(lastline.left(3) == QString("211")) // single or multiple inexact
			{
			  inexact_list.clear();
			  state = INEX_READ;
			} else {
			  if(lastline.left(3) == QString("202"))
				{
				  state = CDDB_DONE;
				  
				  cddb_close(sock);
				  emit cddb_no_info();
				} else {
				  if(lastline.left(3) == QString("210")) // multiple exact
					{
					  inexact_list.clear();
					  state = MULTEX_READ;
					} else {
					  state = ERROR_QUERY;
					  cddb_close(sock);
					  kdDebug() << "ERROR_QUERY\n" << endl;
					  emit cddb_failed();
					}
				}
			}
		}
	  break;
	  
	case INEX_READ:
	  if(lastline.at(0) == '.')
		{
		  state = CDDB_DONE;
		  timeouttimer.stop();
		  emit cddb_inexact_read();
		} else {
		  inexact_list.append(lastline);
		}
	  break;

	case MULTEX_READ:
	  if(lastline.at(0) == '.')
		{
		  state = CDDB_DONE;
		  timeouttimer.stop();
		  emit cddb_inexact_read();
		} else {
		  inexact_list.append(lastline);
		}
	  break;
	  
	case CDDB_READING:
	  if(lastline.at(0) == '.')
		{
		  close(cddbfh);
		  cddbfh = 0;
		  if(protocol!=CDDBHTTP)
			write(sock->socket(),"quit\n",6);
		  state = CDDB_DONE;
		  
		  cddb_close(sock);
		  emit cddb_done();
		} else {
		  if(!cddbfh)
			{
			  QString file;
			  file.sprintf("%s/%08lx", category.utf8().data(), magicID);
			  file = locate("cddb", file);
			  
			  kdDebug() << "dir/file path: " << file << "\n" << endl;
			  cddbfh = open(QFile::encodeName(file), O_CREAT|O_WRONLY|O_TRUNC,
							S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
			}
		  cddblinelen = lastline.length();
		  write(cddbfh, lastline.ascii(), cddblinelen);
		  write(cddbfh, "\n", strlen("\n"));
		  //            kdDebug() << "line written: " << cddblinelen << "\n" << endl;
		  
		  respbuffer.prepend("\n");
		  respbuffer.prepend(lastline);
		}
	  break;
	  
	case CDDB_READ:
	  
	  if(lastline.at(0) == '4')
		{
		  state = ERROR_CDDB_READ;
		  kdDebug() << "ERROR_CDDB_READ\n" << endl;
		  cddb_close(sock);
		  emit cddb_failed();
		} else {
		  respbuffer="";
		  state = CDDB_READING;
		}
	  break;
	  
	case SERVER_LIST_WAIT:
	  
	  if(lastline.left(3) == QString("210"))
		{
		  serverlist.clear();
		  state=GETTING_SERVER_LIST;
		} else {
		  state=CDDB_DONE;
		  emit get_server_list_failed();
		}
	  break;
	  
	case GETTING_SERVER_LIST:
	  if(lastline.at(0) == '.')
		{
		  kdDebug() << "GOT SERVERLIST\n" << endl;
		  if(protocol!=CDDBHTTP)
			write(sock->socket(),"quit\n",6);
		  cddb_close(sock);
		  emit get_server_list_done();
		  state = CDDB_DONE;
		} else {
		  parse_serverlist_entry();
		}
	  break;
	  
	default:
	  break;
	}
  lastline="";
} // do_state_machine


void 
CDDB::serverList(QStringList& list)
{
  list = serverlist;
}

void
CDDB::parse_serverlist_entry()
{
    QString tempstr;

    QStringList sl = QStringList::split(' ', lastline);
    if(protocol_level<3)
      {
        if (sl.count() > 1) {
        	tempstr = sl[0] + " cddbp " + sl[1] + " -";
		serverlist.append(tempstr);
	}
      } else {
        if (sl.count() > 3) {
		tempstr = sl[0] + " " + sl[1] + " " + sl[2] + " " + sl[3];
		//         transport tr=decodeTransport(proto);
		//         if(tr==CDDBP || tr==CDDBHTTP)
	        serverlist.append(tempstr);
	}
      }
} // parse_serverlist_entry

void 
CDDB::get_inexact_list(QStringList& p_inexact_list)
{
    p_inexact_list=inexact_list;
} // get_inexact_list

bool 
CDDB::local_query(
		  unsigned  long magicID,
		  QString&  data,
		  QStringList& titlelist,
		  QStringList& extlist,
		  QString&  _category,
		  QStringList& discidlist,
		  int&	 revision,
		  QStringList& playlist
		  )
{
  
  // pre-set revision to 0
  revision = 0;
  QStringList pathlist = KGlobal::dirs()->resourceDirs("cddb");
  
  if(pathlist.count() == 0)
    return false;
  
  QDir d;
  
  for(QStringList::ConstIterator it = pathlist.begin() ; it != pathlist.end(); it++)
    { 
      d.setPath(*it);
      d.setFilter( QDir::Dirs);
      d.setSorting( QDir::Size);
      
      QStringList list = d.entryList();
      QStringList::Iterator it2;    
      
      for(it2 = list.begin(); it2 != list.end(); it2++)
	{
	  if (*it2 != "." && *it2 != "..")
	    {
	      if(checkDir(magicID, *it + *it2))
		{
		  category = *it2;
		  
		  getData(data,titlelist,extlist,_category,discidlist,revision,playlist);
		  return true;
		}
	    }
	}
    }
  return false;
} // local_query

bool 
CDDB::checkDir(unsigned long magicID, const QString& dir)
{
    QString mag;
    mag.sprintf("%s/%08lx",dir.utf8().data(),magicID);

    QFileInfo info(mag);
    if(!info.isReadable())
	return false;

    respbuffer = "";

    QFile file(mag);

    if( !file.open( IO_ReadOnly )) 
      {
	return false;
      }
    
    QTextStream t(&file);
    
    while ( !t.eof() ) 
      {
	QString s = t.readLine() + "\n";
	// The following swallowed the last line of the file
	// no matter if it's just a newline or a whole entry.
	//	if(!t.eof())    
	  respbuffer += s;
      }

    file.close();
    return true;
} // checkDir

// scan the relevant parts of the cddba database entry in the the provied structures
void 
CDDB::getData(
	      QString& data,
	      QStringList& titles, 
	      QStringList& extlist,
	      QString& categ,
	      QStringList& discidlist, 
	      int& revision,
	      QStringList& playlist
	      )
{
  data = "";
  titles.clear();
  extlist.clear();
  discidlist.clear();
  categ      = category;
  data       = respbuffer;
    
  
    int pos1,pos2,pos3,pos4 = 0;

    int revtmp = data.find("Revision:",0,true);
    if(revtmp == -1)
      {
	revision = 0;  // no Revision -> we are first (ZERO)!
      } else {
	// Well, I'll see if there's a simpler way for this.
	QString revstr;
	int revtmp2;
	revtmp2 = data.find("\n",revtmp,true);
	if(revtmp2 - revtmp - 9 >=0)
	  revstr = data.mid(revtmp +9,revtmp2 - revtmp -9);
	revstr.stripWhiteSpace();
	bool ok;
	revision = revstr.toInt(&ok);
	if(!ok) // Bogus Revision -> we claim to be first (ZERO)!
	  revision = 0;
      }
    
    // lets get all DISCID's in the data. Remeber there can be many DISCID's on
    // several lines separated by commas on each line
    //
    // DISCID= 47842934,4h48393,47839492
    // DISCID= 47fd2934,4h48343,47839492,43879074
    
    while((pos3 = data.find("DISCID=",pos4,true))!= -1)
      {
	pos1 = pos3;
	pos2 = data.find("\n",pos1,true);
	
	QString discidtemp;
	QString temp3;
	
	if( pos2 - pos1 -7 >= 0)
	  {
	    discidtemp = data.mid(pos1 + 7,pos2- pos1 -7);
	  } else {
	    kdDebug() << "ANOMALY 1\n" << endl;
	  }
	
	kdDebug() << "DISCDID " << discidtemp << "\n" << endl;
	
	pos1 = 0;
	while((pos2 = discidtemp.find(",",pos1,true)) != -1)
	  {
	    if( pos2 - pos1 >= 0)
	      {
		temp3 = discidtemp.mid(pos1,pos2-pos1);
	      } else {
		kdDebug() << "ANOMALY 2\n" << endl;
	      }
	    
	    temp3 = temp3.stripWhiteSpace();
	    
	    if(!temp3.isEmpty())
	      discidlist.append(temp3);
	    pos1 = pos2 + 1;
	  }

	temp3 = discidtemp.mid(pos1,discidtemp.length());
	temp3.stripWhiteSpace();
	
	if(!temp3.isEmpty())
	  {
	    discidlist.append(temp3);
	  }
	pos4 = pos3 + 1;
      }// end get DISCID's

    kdDebug() << "FOUND " << discidlist.count() << " DISCID's\n" << endl;

  // Get the DTITLE

    QString value;
    QString key;
    key = "DTITLE=";

    getValue(key,value,data);
    titles.append(value);


    int counter = 0;
    key = key.sprintf("TTITLE%d=",counter);
    while(getValue(key,value,data))
      {
	titles.append(value);
	key = key.sprintf("TTITLE%d=",++counter);
      }
    
    key = "EXTD=";
    getValue(key,value,data);
    extlist.append(value);
    
    counter = 0;
    key = key.sprintf("EXTT%d=",counter);
    while(getValue(key,value,data))
      {
	extlist.append(value);
	key = key.sprintf("EXTT%d=",++counter);
      }
    
    key = "PLAYORDER=";
    getValue(key,value,data);
    cddb_playlist_decode(playlist, value);
} // getData

QString 
CDDB::getCategoryFromPathName(const QString& pathname){
  
    QString path = pathname;
    path = path.stripWhiteSpace();

    while(path.right(1).at(1) == '/'){
	path = path.left(path.length() - 1);
    }

    int pos = 0;
    pos  = path.findRev("/",-1,true);
    if(pos == -1)
	return path;
    else
	return path.mid(pos+1,path.length());

} // getCategoryFromPathName

bool 
CDDB::getValue(QString& key,QString& value, QString& data)
{

    bool found_one = false;
    int pos1 = 0;
    int pos2 = 0;

    value = "";

    while((  pos1 = data.find(key,pos1,true)) != -1)
      {
	found_one = true;
	pos2 = data.find("\n",pos1,true);
	if( (pos2 - pos1 - (int)key.length()) >= 0)
	  {
	    value += data.mid(pos1 + key.length(), pos2 - pos1 - key.length());
	  } else {
	    kdDebug() << "GET VALUE ANOMALY 1\n" << endl;
	  }
	pos1 = pos1 + 1;
      }

    if(value.isNull())
	value = "";

    cddb_decode(value);
    return found_one;
} // getValue

void 
cddb_playlist_encode(QStringList& list,QString& playstr)
{
  playstr = list.join(",");
} // cddb_playlist_encode


bool 
cddb_playlist_decode(QStringList& list, QString& str)
{ 
  bool isok = true;
  int pos1, pos2;
  pos1 = 0;
  pos2 = 0;
  
  list.clear();

  list = QStringList::split(',', str);

    QString check;
    bool 	ok1;
    int   num;

    for ( QStringList::Iterator it = list.begin();
          it != list.end();
          ++it )
      {
	check = *it;
	check = check.stripWhiteSpace();

        if (check.isEmpty())
          {
            it = list.remove(it);
            continue;
          }
	
	num = check.toInt(&ok1);
	if(!ok1 || num < 1)
	  {
	    it = list.remove(it);
	    isok = false;
	    continue;
	  }
    
        *it = check;
      }

    /*  for(uint i = 0; i < list.count(); i++){
	printf("playlist %d=%s\n",i,list.at(i));
	}*/
    return isok;
} // cddb_playlist_decode

void 
cddb_decode(QString& str)
{
    int pos1 = 0;
    int pos2 = 0;

    while((pos2 = str.find("\\n",pos1,true)) !=-1  )
      {
	if(pos2>0)
	  {
	    if(str.mid(pos2-1,3) == QString("\\\\n"))
	      {
		pos1 = pos2 + 2;
		continue;
	      }
	  }
	str.replace(pos2 , 2 , "\n");
	pos1 = pos2 + 1;
      }

    pos1 = 0;
    pos2 = 0;

    while((pos2 = str.find("\\t",pos1,true)) !=-1)
      {
	if( pos2 > 0 )
	  {
	    if(str.mid(pos2-1,3) == QString("\\\\t"))
	      {
		pos1 = pos2 + 2;
		continue;
	      }
	  }
	str.replace(pos2 , 2 , "\t");
	pos1 = pos2 + 1;
      }
    
    pos1 = 0;
    pos2 = 0;
    
    while((pos2 = str.find("\\\\",pos1,true)) !=-1)
      {
	str.replace(pos2 , 2 , "\\");
	pos1 = pos2 + 1;
      }
} // cddb_decode

void 
cddb_encode(QString& str, QStringList &returnlist)
{
    returnlist.clear();
    
    int pos1 = 0;
    int pos2 = 0;
    
    while((pos2 = str.find("\\",pos1,true)) !=-1)
      {
	str.replace(pos2 , 1 , "\\\\");
	pos1 = pos2 + 2;
      }

    pos1 = 0;
    pos2 = 0;

    while((pos2 = str.find("\n",pos1,true)) !=-1)
      {
	str.replace(pos2 , 1 , "\\n");
	pos1 = pos2 + 2;
      }

    pos1 = 0;
    pos2 = 0;

    while((pos2 = str.find("\t",pos1,true)) !=-1)
      {
	str.replace(pos2 , 1 , "\\t");
	pos1 = pos2 + 2;
      }

    while(str.length() > 70)
      {
	returnlist.append(str.left(70));
	str = str.mid(70,str.length());
      }

    returnlist.append(str);
} // cddb_encode

// This function converts server list entry from "Server Port" format
// To "Server Protocol Port Address".
//     The fields are as follows:
//         site:
//             The Internet address of the remote site.
//         protocol:
//             The transfer protocol used to access the site.
//         port:
//             The port at which the server resides on that site.
//         address:
//             Any additional addressing information needed to access the
//             server. For example, for HTTP protocol servers, this would be
//             the path to the CDDB server CGI script. This field will be
//             "-" if no additional addressing information is needed.
//
// Returns 'true' if format have been converted.
bool 
CDDB::normalize_server_list_entry(QString &entry)
{
    QStringList sl = QStringList::split(' ', entry);
    
    if(sl.count() == 2)
      {
	// old format
	entry = sl[0] + " cddbp " + sl[1] + " -";
	return true;
      } else {
	// Otherwise let us leave the item unchanged.
	return false;
      }
} // normalize_server_list_entry

void 
CDDB::setHTTPProxy(QString host, unsigned short int port)
{
  proxyhost=host;
  proxyport=port;
} // setHTTPProxy

void 
CDDB::useHTTPProxy(bool flag)
{
    use_http_proxy=flag;
} // useHTTPProxy(bool)

bool
CDDB::useHTTPProxy()
{
    return use_http_proxy;
} // useHTTPProxy

unsigned short 
int CDDB::getHTTPProxyPort()
{
    return proxyport;
} // getHTTPProxyPort

QString 
CDDB::getHTTPProxyHost()
{
    return proxyhost;
} // getHTTPProxyHost

CDDB::transport 
CDDB::decodeTransport(const char *proto)
{
    if(strcasecmp(proto,"cddbp")==0)
        return CDDBP;
    else
        if(strcasecmp(proto,"http")==0)
            return CDDBHTTP;
        else
            if(strcasecmp(proto,"smtp")==0)
                return SMTP;
            else
                return UNKNOWN;
} // decodeTransport

void 
CDDB::cddb_http_xlat(QString &s)
{
    char q[6];

    if(s.isEmpty())
        return;

    unsigned int pos=0;
    while(pos < s.length()+1)
      {
        switch (s[pos].latin1()) 
	  {
	  case ' ':
            s[pos]='+';
            pos++;
            break;
	  case '?':
	  case '=':
	  case '+':
	  case '&':
	  case '%':
            (void) sprintf(q, "%%%02X", (char) s[pos].latin1());
            s.remove(pos,1);
            s.insert(pos+1,q);
            pos += 3;
            break;
	  default:
            pos++;
	  }
      }
} // cddb_http_xlat


void 
CDDB::setPathList(QStringList& _paths)
{
    pathlist = _paths; // automatically makes deep copies is _paths has deep copies
} // setPathList

#include "cddb.moc"





