/*
 * Kscd - A simple cd player for the KDE Project
 *
 * $Id$
 *
 * Copyright (c) 1997 Bernd Johannes wuebben@math.cornell.edu
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <smtp.h>

#include <sys/utsname.h>
#include <unistd.h>
#include <stdio.h>

#include <kdebug.h>

SMTP::SMTP(char *serverhost, unsigned short int port, int timeout)
{
    struct utsname uts;

    serverHost = serverhost;
    hostPort = port;
    timeOut = timeout * 1000;

    // The following defaults are supposed to be overwritten
    senderAddress = "user@host.ext";
    senderReplyTo = "";
    recipientAddress = "user@host.ext";
    messageSubject = "freedb unknown unknown";
    messageBody = "empty";
    
    connected = false;
    finished = false;

    sock = 0L;
    state = INIT;
    serverState = NONE;

    uname(&uts);
    domainName = uts.nodename;
    
    
    // this is for the HELO
    if(domainName.isEmpty())
        domainName = "somemachine.nowhere.org";

    connect(&connectTimer, SIGNAL(timeout()), this, SLOT(connectTimerTick()));
    connect(&timeOutTimer, SIGNAL(timeout()), this, SLOT(connectTimedOut()));
    connect(&interactTimer, SIGNAL(timeout()), this, SLOT(interactTimedOut()));

    // some sendmail will give 'duplicate helo' error, quick fix for now
    connect(this, SIGNAL(messageSent()), SLOT(closeConnection()));
}

SMTP::~SMTP()
{
    if(sock){
        delete sock;
        sock = 0L;
    }
    connectTimer.stop();
    timeOutTimer.stop();
}

void SMTP::setServerHost(const QString& serverhost)
{
    serverHost = serverhost;
}

void SMTP::setPort(unsigned short int port)
{
    hostPort = port;
}

void SMTP::setTimeOut(int timeout)
{
    timeOut = timeout;
}

void SMTP::setSenderAddress(const QString& sender)
{
    senderAddress = sender;
}

void SMTP::setSenderReplyTo(const QString& replyto)
{
    senderReplyTo = replyto;
}

void SMTP::setRecipientAddress(const QString& recipient)
{
    recipientAddress = recipient;
}

void SMTP::setMessageSubject(const QString& subject)
{
    messageSubject = subject;
}

void SMTP::setMessageBody(const QString& message)
{
    messageBody = message;
}

void SMTP::openConnection(void)
{
    kdDebug() << "started connect timer\n" << endl;
    connectTimer.start(100, TRUE);
}

void SMTP::closeConnection(void)
{
    socketClose(sock);
}

void SMTP::sendMessage(void)
{
    if(!connected)
        connectTimerTick();
    if(state == FINISHED && connected){
        kdDebug() << "state was == FINISHED\n" << endl;
        finished = false;
        state = IN;
        writeString = QString::fromLocal8Bit("helo %1\r\n").arg(domainName);
        write(sock->socket(), writeString.ascii(), writeString.length());
    }
    if(connected){
        kdDebug() << "enabling read on sock...\n" << endl;
        interactTimer.start(timeOut, TRUE);
        sock->enableRead(true);
    }
}
#include <stdio.h>

void SMTP::connectTimerTick(void)
{
    connectTimer.stop();
//    timeOutTimer.start(timeOut, TRUE);

    kdDebug() << "connectTimerTick called...\n" << endl;
    
    if(sock){
        delete sock;
        sock = 0L;
    }

    kdDebug() << "connecting to " << serverHost << ":" << hostPort << " ..... \n" << endl;
    sock = new KSocket(serverHost.ascii(), hostPort);

    if(sock == 0L || sock->socket() < 0){
        timeOutTimer.stop();
        kdDebug() << "connection failed!\n" << endl;
        emit error(CONNECTERROR);
        connected = false;
        return;
    }
    connected = true;
    finished = false;
    state = INIT;
    serverState = NONE;

    connect(sock, SIGNAL(readEvent(KSocket *)), this, SLOT(socketRead(KSocket *)));
    connect(sock, SIGNAL(closeEvent(KSocket *)), this, SLOT(socketClose(KSocket *)));
    //    sock->enableRead(true);
    timeOutTimer.stop();
    kdDebug() << "connected\n" << endl;
}

void SMTP::connectTimedOut(void)
{
    timeOutTimer.stop();

    if(sock) 
	sock->enableRead(false);
    kdDebug() << "socket connection timed out\n" << endl;
    socketClose(sock);
    emit error(CONNECTTIMEOUT);
}

void SMTP::interactTimedOut(void)
{
    interactTimer.stop();

    if(sock)
        sock->enableRead(false);
    kdDebug() << "time out waiting for server interaction" << endl;
    socketClose(sock);
    emit error(INTERACTTIMEOUT);
}

void SMTP::socketRead(KSocket *socket)
{
    int n, nl;

    kdDebug() << "socketRead() called...\n" << endl;
    interactTimer.stop();

    if(socket == 0L || socket->socket() < 0)
        return;
    n = read(socket->socket(), readBuffer, SMTP_READ_BUFFER_SIZE-1 );
    readBuffer[n] = '\0';
    lineBuffer += readBuffer;
    nl = lineBuffer.find('\n');
    if(nl == -1)
        return;
    lastLine = lineBuffer.left(nl);
    lineBuffer = lineBuffer.right(lineBuffer.length() - nl - 1);
    processLine(&lastLine);
    if(connected)
        interactTimer.start(timeOut, TRUE);
}

void SMTP::socketClose(KSocket *socket)
{
    timeOutTimer.stop();
    disconnect(sock, SIGNAL(readEvent(KSocket *)), this, SLOT(socketRead(KSocket *)));
    disconnect(sock, SIGNAL(closeEvent(KSocket *)), this, SLOT(socketClose(KSocket *)));
    socket->enableRead(false);
    kdDebug() << "connection terminated\n" << endl;
    connected = false;
    if(socket){
        delete socket;
        socket = 0L;
        sock = 0L;
    }
    emit connectionClosed();
}

void SMTP::processLine(QString *line)
{
    int i, stat;
    QString tmpstr;
    
    i = line->find(' ');
    tmpstr = line->left(i);
    if(i > 3)
        kdDebug() << "warning: SMTP status code longer then 3 digits: " << tmpstr << "\n" << endl;
    stat = tmpstr.toInt();
    serverState = (SMTPServerStatus)stat;
    lastState = state;

    kdDebug() << "smtp state: [" << stat << "][" << *line << "]\n" << endl;

    switch(stat){
    case GREET:     //220
        state = IN;
        writeString = QString::fromLocal8Bit("helo %1\r\n").arg(domainName);
	write(sock->socket(), writeString.ascii(), writeString.length());
        break;
    case GOODBYE:   //221
        state = QUIT;
        break;
    case SUCCESSFUL://250
        switch(state){
        case IN:
            state = READY;
            writeString = QString::fromLocal8Bit("mail from: %1\r\n").arg(senderAddress);
            write(sock->socket(), writeString.ascii(), writeString.length());
            break;
        case READY:
            state = SENTFROM;
            writeString = QString::fromLocal8Bit("rcpt to: %1\r\n").arg(recipientAddress);
            write(sock->socket(), writeString.ascii(), writeString.length());
            break;
        case SENTFROM:
            state = SENTTO;
            writeString = QString::fromLocal8Bit("data\r\n");
            write(sock->socket(), writeString.ascii(), writeString.length());
            break;
        case DATA:
            state = FINISHED;
            finished = true;
            sock->enableRead(false);
            emit messageSent();
            break;
        default:
            state = CERROR;
            kdDebug() << "smtp error (state error): [" << lastState << "]:[" << stat << "][" << *line << "]\n" << endl;
            socketClose(sock);
            emit error(COMMAND);
            break;
        }
        break;
    case READYDATA: //354
        state = DATA;
        //        writeString = QString::fromLocal8Bit("Subject: %1\n%2\n.\n").arg(messageSubject).arg(messageBody);
        writeString = QString::fromLocal8Bit("Subject: %1\r\n").arg(messageSubject);
	writeString += QString::fromLocal8Bit("From: %1\r\n").arg(senderAddress);
	kdDebug() << senderReplyTo << endl;
	if( (senderReplyTo != NULL) && senderReplyTo.contains("@") )
	  {
	    writeString += QString::fromLocal8Bit("Reply-To: %1\r\n").arg(senderReplyTo);
	  }
	writeString += QString::fromLocal8Bit("To: %1\r\n\r\n").arg(recipientAddress);
        writeString += messageBody;
        writeString += QString::fromLocal8Bit(".\r\n");
        write(sock->socket(), writeString.ascii(), writeString.length());
        break;
    case ERROR:     //501
        state = CERROR;
        kdDebug() << "smtp error (command error): [" << lastState << "]:[" << stat << "][" << *line << "]\n" << endl;
        socketClose(sock);
        emit error(COMMAND);
        break;
    case UNKNOWN:   //550
        state = CERROR;
        kdDebug() << "smtp error (unknown user): [" << lastState << "]:[" << stat << "][" << *line << "]\n" << endl;
        socketClose(sock);
        emit error(UNKNOWNUSER);
        break;
    default:
        state = CERROR;
        kdDebug() << "unknown response: [" << lastState << "]:[" << stat << "][" << *line << "]\n" << endl;
        socketClose(sock);
        emit error(UNKNOWNRESPONSE);
    }
} // processLine

#include "smtp.moc"
