/*
 * This file was initialy generated by QTArchitect, but was heavily modified
 * afterwards.
 */

#include "CDDBSetup.h"

#define Inherited CDDBSetupData

#include <kapp.h>
#include <cddb.h>
#include <qvalidator.h> 
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

extern KApplication 	*mykapp;

CDDBSetup::CDDBSetup
(
    QWidget* parent,
    const char* name
)
    :
    Inherited( parent, name )
{


    QString temp;
    QString basedirdefault;


    basedir_edit->setText(temp.data());
    connect(basedir_edit,SIGNAL(textChanged(const QString &)),
            this,SLOT(basedir_changed(const QString &)));

    connect(update_button,SIGNAL(clicked()),
            this,SLOT(serverlist_update()));

    connect(defaults_button,SIGNAL(clicked()),
            this,SLOT(set_defaults()));

    connect(help_button,SIGNAL(clicked()),
            this,SLOT(help()));

    connect(server_listbox, SIGNAL(highlighted(int)), this,
            SLOT(set_current_server(int)));
    
    connect(submission_listbox,SIGNAL(highlighted(int)),
            this,SLOT(set_current_submission_address(int)));

    connect(remote_cddb_cb,SIGNAL(toggled(bool)),
            this,SLOT(enable_remote_cddb(bool)));

    connect(currentServerAddPB, SIGNAL(clicked()), this, SLOT(insertSL()));
    connect(currentServerDelPB, SIGNAL(clicked()), this, SLOT(removeSL()));

    connect(currentSubmitAddPB, SIGNAL(clicked()), this, SLOT(insertSUL()));
    connect(currentSubmitDelPB, SIGNAL(clicked()), this, SLOT(removeSUL()));

    QIntValidator *port_validator=new QIntValidator(this,"HTTP Port Validator");
    port_validator->setRange(0, INT_MAX );  
    proxy_port_ef->setValidator(port_validator); 

    proxy_port_ef->setEnabled(cddb_http_cb->isChecked());
    proxy_host_ef->setEnabled(cddb_http_cb->isChecked());
}

CDDBSetup::~CDDBSetup()
{
}

void CDDBSetup::insertSL(void)
{
    uint i;

    current_server_string = currentServerLE->text();
    for(i = 0; i < server_listbox->count(); i++){
        if(current_server_string == server_listbox->text(i)){
            server_listbox->setCurrentItem(i);
            server_listbox->centerCurrentItem();
            return;
        }
    }
    server_listbox->insertItem(current_server_string.data());
    server_listbox->setCurrentItem(server_listbox->count()-1);
    server_listbox->centerCurrentItem();
}

void CDDBSetup::removeSL(void)
{
    int it = server_listbox->currentItem();
    if(it == -1)
        return;
    server_listbox->removeItem(it);
}

void CDDBSetup::insertSUL(void)
{
    uint i;

    submitaddressstring = currentSubmitLE->text();
    for(i = 0; i < submission_listbox->count(); i++){
        if(submitaddressstring == submission_listbox->text(i)){
            submission_listbox->setCurrentItem(i);
            submission_listbox->centerCurrentItem();
            return;
        }
    }
    submission_listbox->insertItem(submitaddressstring.data());
    submission_listbox->setCurrentItem(submission_listbox->count()-1);
    submission_listbox->centerCurrentItem();
}

void CDDBSetup::removeSUL(void)
{
    int it = submission_listbox->currentItem();
    if(it == -1)
        return;
    submission_listbox->removeItem(it);
}


void CDDBSetup::set_current_server(int i)
{
    current_server_string = server_listbox->text(i);
    currentServerLE->setText(current_server_string.data());
    emit updateCurrentServer();
}

void CDDBSetup::set_current_submission_address(int i)
{
    submitaddressstring = submission_listbox->text(i);
    currentSubmitLE->setText(submitaddressstring.data());
}

void CDDBSetup::basedir_changed(const QString &str)
{
    basedirstring = str;
}

void CDDBSetup::enable_remote_cddb(bool)
{
}

void CDDBSetup::help()
{
    if(mykapp)
        mykapp->invokeHTMLHelp("kscd/kscd.html","");

}

extern bool debugflag;

void CDDBSetup::insertData(const QStrList& _serverlist,
                           const QStrList& _submitlist,
			   const QString& _basedir,
			   const QString& _submitaddress,
			   const QString& _current_server,
			   const bool&    remote_enabled,
			   const bool&    http_proxy_enabled,
			   const QString& http_proxy_host,
			   const unsigned short int& http_proxy_port)
{

    current_server_string = _current_server.copy();
    currentServerLE->setText(current_server_string.data());

    submitaddressstring = _submitaddress.copy();
    currentSubmitLE->setText(submitaddressstring);

    submitlist.clear();
    for(uint i = 0; i < _submitlist.count(); i++){
        submitlist.append(QStrList(_submitlist).at(i));
    }
    
    insertServerList(_serverlist);
//    server_listbox->setCurrentItem(_serverlist.find(_current_server.data()));
    //server_listbox->find(_current_server.data());
//    server_listbox->centerCurrentItem();

    if(debugflag) fprintf(stderr, "check point: server_listbox->centerCurrentItem()\n");
    
    basedirstring = _basedir.copy();
    basedir_edit->setText(basedirstring);
    
    remote_cddb_cb->setChecked(remote_enabled);
    cddb_http_cb->setChecked(http_proxy_enabled);
    proxy_host_ef->setText(http_proxy_host);
    char port_str[40];
    sprintf(port_str,"%d",http_proxy_port);
    proxy_port_ef->setText(port_str);
    if(debugflag) fprintf(stderr, "check point: ::insertData{...;return;}\n");
}

void CDDBSetup::set_defaults()
{
    server_listbox->setAutoUpdate(false);
    server_listbox->clear();
    server_listbox->insertItem(DEFAULT_CDDB_SERVER, -1);
    // We should provide at least one entry
    // with HTTP protocol so people behind firewals can get servers list
    server_listbox->insertItem(DEFAULT_CDDBHTTP_SERVER, -1); 
    server_listbox->setAutoUpdate(true);
    server_listbox->repaint();
    server_listbox->setCurrentItem(0);

    basedirstring = mykapp->kde_datadir().copy();
    basedirstring += "/kscd/cddb/";
    basedir_edit->setText(basedirstring);

    submission_listbox->setAutoUpdate(false);
    submission_listbox->clear();
    submission_listbox->insertItem(DEFAULT_SUBMIT_EMAIL,-1);
    submission_listbox->insertItem(DEFAULT_TEST_EMAIL, -1);
    submission_listbox->setAutoUpdate(true);
    submission_listbox->repaint();
    submission_listbox->setCurrentItem(0);

    remote_cddb_cb->setChecked(true);
    cddb_http_cb->setChecked(false);
    // Leave proxy host and port values unchanged, just disable them

    emit updateCurrentServer();
}
void CDDBSetup::getData(QStrList& _serverlist,
                        QStrList& _submitlist,
			QString& _basedir,
			QString& _submitaddress, 
			QString& _current_server,
			bool&    remote_enabled,
			bool&    http_proxy_enabled,
			QString  &http_proxy_host,
			unsigned short int &http_proxy_port)
{
    uint i;

    _serverlist.clear();
    _submitlist.clear();
    for(i = 0; i < server_listbox->count();i++){
        _serverlist.append(server_listbox->text(i));
    }
    for(i = 0; i < submission_listbox->count(); i++){
        _submitlist.append(submission_listbox->text(i));
    }
    _basedir = basedirstring.copy();
    _submitaddress = submitaddressstring.copy();

    _current_server     = current_server_string.copy();
    remote_enabled      = remote_cddb_cb->isChecked();
    http_proxy_enabled  = cddb_http_cb->isChecked();
    http_proxy_host     = proxy_host_ef->text();
    http_proxy_port     = atoi(proxy_port_ef->text());
}

void CDDBSetup::getCurrentServer(QString& ser)
{
    ser = current_server_string.copy();
}

void CDDBSetup::serverlist_update()
{
    emit updateCDDBServers();
}

void CDDBSetup::insertServerList(const QStrList& list)
{
    QString current_server_string_backup;
    uint i;

    current_server_string_backup = current_server_string.copy();
    server_listbox->setAutoUpdate(false);
    server_listbox->clear();
    submission_listbox->setAutoUpdate(false);
    submission_listbox->clear();

    bool have_email = false;
    bool have_srv   = false;

    QListIterator<char> it(list);

    for(;it.current();++it)
    {
        char ser   [CDDB_FIELD_BUFFER_LEN];
        char por   [CDDB_FIELD_BUFFER_LEN];
        char proto [CDDB_FIELD_BUFFER_LEN];
        char extra [CDDB_FIELD_BUFFER_LEN];
        char email [CDDB_FIELD_BUFFER_LEN];
        
        const char* srv=it.current();
        sscanf(srv,"%s %s %s %s",ser,proto,por,extra);
        CDDB::transport t=CDDB::decodeTransport(proto);
        if(t==CDDB::UNKNOWN)
            continue;
        else
            if(t==CDDB::SMTP)
            {
                sprintf(email,"%s@%s",extra,ser);
                have_email=true;
                submission_listbox->insertItem(email, -1);
            }
            else
            {
                have_srv=true;
                server_listbox->insertItem(srv, -1);
            }
    }
 
    if(!have_srv)
    {
        server_listbox->insertItem(DEFAULT_CDDB_SERVER, -1);
        server_listbox->insertItem(DEFAULT_CDDBHTTP_SERVER, -1); 
    }
    
    if(!have_email){
        for(i = 0; i < submitlist.count(); i++){
            submission_listbox->insertItem(submitlist.at(i));
        }
//        submission_listbox->insertItem(DEFAULT_SUBMIT_EMAIL,-1);
//        submission_listbox->insertItem(DEFAULT_TEST_EMAIL, -1);
    }
   
    server_listbox->setAutoUpdate(true);
    server_listbox->repaint();
    submission_listbox->setAutoUpdate(true);
    submission_listbox->repaint();

    bool found = 0;

    current_server_string = current_server_string_backup.copy();
    //current_server_string = currentServerLE->text();
    if(debugflag) fprintf(stderr, "current_server_string: %s\n", current_server_string.data());
    for(i = 0; i < server_listbox->count(); i++){
        if(current_server_string == server_listbox->text(i)){
            server_listbox->setCurrentItem(i);
            server_listbox->centerCurrentItem();
            found = 1;
            break;
        }
    }
    if(!found){
        server_listbox->setCurrentItem(0);
        server_listbox->centerCurrentItem();
    }

//    submitaddressstring = currentSubmitLE->text();
    for(i = 0; i < submission_listbox->count(); i++){
        if(submitaddressstring == submission_listbox->text(i)){
            submission_listbox->setCurrentItem(i);
            submission_listbox->centerCurrentItem();
            found = 1;
            break;
        }
    }
    if(!found){
        submission_listbox->setCurrentItem(0);
        submission_listbox->centerCurrentItem();
    }
//    server_listbox->setCurrentItem(0);
//    submission_listbox->setCurrentItem(0);
}

void CDDBSetup::http_access_toggled(bool state)
{
    proxy_port_ef->setEnabled(state);
    proxy_host_ef->setEnabled(state);
}

#include "CDDBSetup.moc"
#include "CDDBSetupData.moc"
