/***********************************************************************
 *
 * I can't get no qtarch running. So I put this note here:
 *
 *
 *      --- This file has been manually midified ---
 *
 *
 * $Id$
 *
 ***********************************************************************/

#include "CDDBSetupData.h"

#define Inherited QWidget

#include <qlabel.h>
#include <qgroupbox.h>

#include <klocale.h>
#include <kapp.h>

CDDBSetupData::CDDBSetupData
(
	QWidget* parent,
	const char* name
)
	:
	Inherited( parent, name, 0 )
{
	QGroupBox* dlgedit_GroupBox_1;
	dlgedit_GroupBox_1 = new QGroupBox( this, "GroupBox_1" );
	dlgedit_GroupBox_1->setGeometry( 10, 10, 520, 420 );
	dlgedit_GroupBox_1->setMinimumSize( 10, 10 );
	dlgedit_GroupBox_1->setMaximumSize( 32767, 32767 );
	dlgedit_GroupBox_1->setFrameStyle( 49 );
	dlgedit_GroupBox_1->setTitle( "" );
	dlgedit_GroupBox_1->setAlignment( 1 );

	QLabel* dlgedit_Label_8;
	dlgedit_Label_8 = new QLabel( this, "Label_8" );
	dlgedit_Label_8->setGeometry( 20, 120, 160, 20 );
	dlgedit_Label_8->setMinimumSize( 10, 10 );
	dlgedit_Label_8->setMaximumSize( 32767, 32767 );
	dlgedit_Label_8->setText( i18n("CDDB Base Directory") );
        dlgedit_Label_8->setFixedSize( dlgedit_Label_8->sizeHint() );
	dlgedit_Label_8->setAlignment( 289 );
	dlgedit_Label_8->setMargin( -1 );

	basedir_edit = new QLineEdit( this, "LineEdit_4" );
	basedir_edit->setGeometry( 20, 143, 495, 25 );
	basedir_edit->setMinimumSize( 10, 10 );
	basedir_edit->setMaximumSize( 32767, 32767 );
	basedir_edit->setText( "" );
	basedir_edit->setMaxLength( 32767 );
	basedir_edit->setEchoMode( QLineEdit::Normal );
	basedir_edit->setFrame( TRUE );

	QLabel* dlgedit_Label_9;
	dlgedit_Label_9 = new QLabel( this, "Label_9" );
	dlgedit_Label_9->setGeometry( 275, 180, 210, 20 );
	dlgedit_Label_9->setMinimumSize( 10, 10 );
	dlgedit_Label_9->setMaximumSize( 32767, 32767 );
	dlgedit_Label_9->setText( i18n("Send CDDB submissions to:") );
        dlgedit_Label_9->setFixedSize( dlgedit_Label_9->sizeHint() );
	dlgedit_Label_9->setAlignment( 289 );
	dlgedit_Label_9->setMargin( -1 );

	server_listbox = new QListBox( this, "ListBox_2" );
	server_listbox->setGeometry( 20, 230, 240, 155 );
	server_listbox->setMinimumSize( 10, 10 );
	server_listbox->setMaximumSize( 32767, 32767 );
	server_listbox->setFrameStyle( 51 );
	server_listbox->setLineWidth( 2 );
	server_listbox->insertItem( "freedb.freedb.org cddbp 8880 -" );
	server_listbox->insertItem( "www.freedb.org http 80 /~cddb/cddb.cgi" );
	server_listbox->setHScrollBarMode( QListBox::AlwaysOn );
	server_listbox->setMultiSelection( FALSE );

	QLabel* dlgedit_Label_10;
	dlgedit_Label_10 = new QLabel( this, "Label_10" );
	dlgedit_Label_10->setGeometry( 20, 180, 235, 20 );
	dlgedit_Label_10->setMinimumSize( 10, 10 );
	dlgedit_Label_10->setMaximumSize( 32767, 32767 );
	dlgedit_Label_10->setText( i18n("CDDB server:") );
        dlgedit_Label_10->setFixedSize( dlgedit_Label_10->sizeHint() );
	dlgedit_Label_10->setAlignment( 289 );
	dlgedit_Label_10->setMargin( -1 );

	update_button = new QPushButton( this, "PushButton_7" );
	update_button->setGeometry( 35, 400, 90, 25 );
	update_button->setMinimumSize( 10, 10 );
	update_button->setMaximumSize( 32767, 32767 );
	update_button->setText( i18n("Update") );
	update_button->setAutoRepeat( FALSE );
	update_button->setAutoResize( FALSE );

        currentServerLE = new QLineEdit(this, "currentServerLE");
        currentServerLE->setGeometry(20, 200, 202, 28);
        currentServerLE->setMinimumSize(10, 10);
        currentServerLE->setMaximumSize(32767, 32767);
        currentServerLE->clear();

        currentServerAddPB = new QPushButton(this, "currentServerAddPB");
        currentServerAddPB->setGeometry(225, 200, 16, 28);
        currentServerAddPB->setText("+");
        
        currentServerDelPB = new QPushButton(this, "currentServerDelPB");
        currentServerDelPB->setGeometry(244, 200, 16, 28);
        currentServerDelPB->setText("-");

	remote_cddb_cb = new QCheckBox( this, "CheckBox_1" );
	remote_cddb_cb->setGeometry( 20, 15, 270, 30 );
	remote_cddb_cb->setMinimumSize( 10, 10 );
	remote_cddb_cb->setMaximumSize( 32767, 32767 );
	connect( remote_cddb_cb, SIGNAL(toggled(bool)), SLOT(enable_remote_cddb(bool)) );
	remote_cddb_cb->setText( i18n("Enable Remote CDDB") );
        remote_cddb_cb->setFixedSize( remote_cddb_cb->sizeHint() );
	remote_cddb_cb->setAutoRepeat( FALSE );
	remote_cddb_cb->setAutoResize( FALSE );

	QLabel* cddb_timeout_lb;
	cddb_timeout_lb = new QLabel( this, "CDDBTimeoutLabel" );
	cddb_timeout_lb->setGeometry( 285, 15, 150, 20 );
	cddb_timeout_lb->setMinimumSize( 10, 10 );
	cddb_timeout_lb->setMaximumSize( 32767, 32767 );
	cddb_timeout_lb->setText( i18n("seconds CDDB timeout") );
	cddb_timeout_lb->setAlignment( 289 );
	cddb_timeout_lb->setMargin( -1 );

	cddb_timeout_ef = new QLineEdit( this, "CDDBTimeout" );
	cddb_timeout_ef->setGeometry( 230, 15, 45, 20 );
	cddb_timeout_ef->setMinimumSize( 10, 10 );
	cddb_timeout_ef->setMaximumSize( 32767, 32767 );
	cddb_timeout_ef->setText( "30" );
	cddb_timeout_ef->setMaxLength( 5 );
	cddb_timeout_ef->setEchoMode( QLineEdit::Normal );
	cddb_timeout_ef->setFrame( TRUE );

	defaults_button = new QPushButton( this, "PushButton_2" );
	defaults_button->setGeometry( 230, 400, 90, 25 );
	defaults_button->setMinimumSize( 10, 10 );
	defaults_button->setMaximumSize( 32767, 32767 );
	defaults_button->setText( i18n("Defaults") );
	defaults_button->setAutoRepeat( FALSE );
	defaults_button->setAutoResize( FALSE );

	help_button = new QPushButton( this, "PushButton_3" );
	help_button->setGeometry( 420, 400, 90, 25 );
	help_button->setMinimumSize( 10, 10 );
	help_button->setMaximumSize( 32767, 32767 );
	help_button->setText( i18n("Help"));
	help_button->setAutoRepeat( FALSE );
	help_button->setAutoResize( FALSE );

	cddb_http_cb = new QCheckBox( this, "CheckBox_2" );
	cddb_http_cb->setGeometry( 20, 40, 230, 30 );
	cddb_http_cb->setMinimumSize( 10, 10 );
	cddb_http_cb->setMaximumSize( 32767, 32767 );
	connect( cddb_http_cb, SIGNAL(toggled(bool)), SLOT(http_access_toggled(bool)) );
	cddb_http_cb->setText( i18n("Use HTTP proxy to access CDDB") );
        cddb_http_cb->setFixedSize( cddb_http_cb->sizeHint() );
	cddb_http_cb->setAutoRepeat( FALSE );
	cddb_http_cb->setAutoResize( FALSE );

	proxy_port_ef = new QLineEdit( this, "LineEdit_6" );
	proxy_port_ef->setGeometry( 470, 80, 45, 25 );
	proxy_port_ef->setMinimumSize( 10, 10 );
	proxy_port_ef->setMaximumSize( 32767, 32767 );
	proxy_port_ef->setText( "" );
	proxy_port_ef->setMaxLength( 5 );
	proxy_port_ef->setEchoMode( QLineEdit::Normal );
	proxy_port_ef->setFrame( TRUE );

	proxy_host_ef = new QLineEdit( this, "LineEdit_7" );
	proxy_host_ef->setGeometry( 70, 80, 385, 25 );
	proxy_host_ef->setMinimumSize( 10, 10 );
	proxy_host_ef->setMaximumSize( 32767, 32767 );
	proxy_host_ef->setText( "" );
	proxy_host_ef->setMaxLength( 32767 );
	proxy_host_ef->setEchoMode( QLineEdit::Normal );
	proxy_host_ef->setFrame( TRUE );

	QLabel* dlgedit_Label_12;
	dlgedit_Label_12 = new QLabel( this, "Label_12" );
	dlgedit_Label_12->setGeometry( 20, 75, 45, 30 );
	dlgedit_Label_12->setMinimumSize( 10, 10 );
	dlgedit_Label_12->setMaximumSize( 32767, 32767 );
	dlgedit_Label_12->setText( "HTTP://" );
	dlgedit_Label_12->setAlignment( 289 );
	dlgedit_Label_12->setMargin( -1 );

	QLabel* dlgedit_Label_13;
	dlgedit_Label_13 = new QLabel( this, "Label_13" );
	dlgedit_Label_13->setGeometry( 460, 75, 10, 30 );
	dlgedit_Label_13->setMinimumSize( 10, 10 );
	dlgedit_Label_13->setMaximumSize( 32767, 32767 );
	dlgedit_Label_13->setText( ":" );
	dlgedit_Label_13->setAlignment( 289 );
	dlgedit_Label_13->setMargin( -1 );

	submission_listbox = new QListBox( this, "ListBox_3" );
	submission_listbox->setGeometry( 275, 230, 240, 155 );
	submission_listbox->setMinimumSize( 10, 10 );
	submission_listbox->setMaximumSize( 32767, 32767 );
	submission_listbox->setFrameStyle( 51 );
	submission_listbox->setLineWidth( 2 );
        submission_listbox->insertItem( "cddb-test@xmcd.com" );
	submission_listbox->setMultiSelection( FALSE );

        currentSubmitLE = new QLineEdit(this, "currentSubmitLE");
        currentSubmitLE->setGeometry(275, 200, 202, 28);
        currentSubmitLE->setMinimumSize(10, 10);
        currentSubmitLE->setMaximumSize(32767, 32767);
        currentSubmitLE->clear();

        currentSubmitAddPB = new QPushButton(this, "currentSubmitAddPB");
        currentSubmitAddPB->setGeometry(480, 200, 16, 28);
        currentSubmitAddPB->setText("+");
        
        currentSubmitDelPB = new QPushButton(this, "currentSubmitDelPB");
        currentSubmitDelPB->setGeometry(499, 200, 16, 28);
        currentSubmitDelPB->setText("-");

/*	current_submit_label = new QLabel( this, "Label_14" );
	current_submit_label->setGeometry( 275, 200, 240, 30 );
	current_submit_label->setMinimumSize( 10, 10 );
	current_submit_label->setMaximumSize( 32767, 32767 );
	current_submit_label->setFrameStyle( 51 );
	current_submit_label->setLineWidth( 2 );
	current_submit_label->setText( "" );
	current_submit_label->setAlignment( 289 );
        current_submit_label->setMargin( -1 );
        */

	resize( 535,435 );
	setMinimumSize( 0, 0 );
	setMaximumSize( 32767, 32767 );
} // CDDBSetupData


CDDBSetupData::~CDDBSetupData()
{
} // ~CDDBSetupData

void 
CDDBSetupData::http_access_toggled(bool)
{
} // http_access_toggled

void 
CDDBSetupData::enable_remote_cddb(bool)
{
} // enable_remote_cddb
