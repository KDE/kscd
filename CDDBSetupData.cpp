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

#include <qlayout.h>
#include <qfontmetrics.h>


CDDBSetupData::CDDBSetupData
(
	QWidget* parent,
	const char* name
)
	:
	Inherited( parent, name, 0 )
{
        QFontMetrics fm ( font() );

	QBoxLayout * lay1 = new QVBoxLayout ( this, 10 );
	QGroupBox* group = new QGroupBox( this, "GroupBox_1" );
	lay1->addWidget ( group );
	QBoxLayout * lay2 = new QVBoxLayout ( group, 10, 0 );

	QBoxLayout * lay3 = new QHBoxLayout ( lay2, 5 );
	remote_cddb_cb = new QCheckBox( i18n("Enable Remote CDDB"), group, "CheckBox_1" );
	connect( remote_cddb_cb, SIGNAL(toggled(bool)), SLOT(enable_remote_cddb(bool)) );
	lay3->addWidget ( remote_cddb_cb );
	lay3->addSpacing ( 15 );
	cddb_timeout_ef = new QLineEdit( group, "CDDBTimeout" );
	cddb_timeout_ef->setText( "30" );
	cddb_timeout_ef->setMaxLength ( 5 );
	cddb_timeout_ef->setFixedWidth ( 5*fm.maxWidth() );
	lay3->addWidget ( cddb_timeout_ef );
	QLabel* cddb_timeout_lb = new QLabel( i18n("seconds CDDB timeout"), group, "CDDBTimeoutLabel" );
        lay3->addWidget ( cddb_timeout_lb );
        lay3->addStretch ( 1 );

	lay2->addSpacing ( fm.lineSpacing() );
	cddb_http_cb = new QCheckBox( i18n("Use HTTP proxy to access CDDB"), group, "CheckBox_2" );
	connect( cddb_http_cb, SIGNAL(toggled(bool)), SLOT(http_access_toggled(bool)) );
	lay2->addWidget ( cddb_http_cb );
	QBoxLayout * lay4 = new QHBoxLayout ( lay2, 5 );
	QLabel* dlgedit_Label_12 = new QLabel( "HTTP://", group, "Label_12" );
	lay4->addWidget ( dlgedit_Label_12 );
	proxy_host_ef = new QLineEdit( group, "LineEdit_7" );
	lay4->addWidget ( proxy_host_ef, 1 );
	QLabel* dlgedit_Label_13 = new QLabel( ":", group, "Label_13" );
	lay4->addWidget ( dlgedit_Label_13 );
	proxy_port_ef = new QLineEdit( group, "LineEdit_6" );
	proxy_port_ef->setMaxLength ( 5 );
	proxy_port_ef->setFixedWidth ( 5*fm.maxWidth() );
	lay4->addWidget ( proxy_port_ef );
	
	/* edm new section start */
	lay2->addSpacing ( fm.lineSpacing() );
	enable_auto_save_cddb = new QCheckBox( i18n("Enable auto save to local cddb"), group, "CheckBox_3" );
	connect( cddb_http_cb, SIGNAL(toggled(bool)), SLOT(http_access_toggled(bool)) );
	lay2->addWidget ( enable_auto_save_cddb );
	/* edm new section end */

	lay2->addSpacing ( fm.lineSpacing() );
	QLabel* dlgedit_Label_8 = new QLabel( i18n("CDDB Base Directory:"), group, "Label_8" );
	lay2->addWidget ( dlgedit_Label_8 );
	basedir_edit = new QLineEdit( group, "LineEdit_4" );
	lay2->addWidget ( basedir_edit );

	lay2->addSpacing ( fm.lineSpacing() );
	QGridLayout * glay = new QGridLayout ( lay2, 3, 3, 0 );
	glay->addColSpacing ( 1, 5 );
	glay->setRowStretch ( 2, 1 );
	glay->setColStretch ( 0, 1 );
	glay->setColStretch ( 2, 1 );
	QLabel* dlgedit_Label_10 = new QLabel( i18n("CDDB server:"), group, "Label_10" );
	glay->addWidget ( dlgedit_Label_10, 0, 0, AlignLeft );
	QLabel* dlgedit_Label_9 = new QLabel( i18n("Send CDDB submissions to:"), group, "Label_9" );
	glay->addWidget ( dlgedit_Label_9, 0, 2, AlignLeft );

	// The default sizeHint() for "+" and "-" buttons is huge for some reason ...
	int smallButtonWidth = fm.width('+') + 10;

        QBoxLayout * lay5 = new QHBoxLayout;
	glay->addLayout ( lay5, 1, 0 );
        currentServerLE = new QLineEdit(group, "currentServerLE");
	lay5->addWidget ( currentServerLE, 1 );
        currentServerAddPB = new QPushButton("+", group, "currentServerAddPB");
	currentServerAddPB->setFixedWidth ( smallButtonWidth );
	lay5->addWidget ( currentServerAddPB );
        currentServerDelPB = new QPushButton("-", group, "currentServerDelPB");
	currentServerDelPB->setFixedWidth ( smallButtonWidth );
	lay5->addWidget ( currentServerDelPB );
        QBoxLayout * lay6 = new QHBoxLayout;
	glay->addLayout ( lay6, 1, 2 );
        currentSubmitLE = new QLineEdit(group, "currentSubmitLE");
	lay6->addWidget ( currentSubmitLE, 1 );
        currentSubmitAddPB = new QPushButton("+", group, "currentSubmitAddPB");
	currentSubmitAddPB->setFixedWidth ( smallButtonWidth );
	lay6->addWidget ( currentSubmitAddPB );
        currentSubmitDelPB = new QPushButton("-", group, "currentSubmitDelPB");
	currentSubmitDelPB->setFixedWidth ( smallButtonWidth );
	lay6->addWidget ( currentSubmitDelPB );

	server_listbox = new QListBox( group, "ListBox_2" );
	glay->addWidget ( server_listbox, 2, 0 );
	server_listbox->insertItem( "freedb.freedb.org cddbp 8880 -" );
	server_listbox->insertItem( "www.freedb.org http 80 /~cddb/cddb.cgi" );
	submission_listbox = new QListBox( group, "ListBox_3" );
	glay->addWidget ( submission_listbox, 2, 2 );
        submission_listbox->insertItem( "cddb-test@xmcd.com" );

	lay2->addSpacing ( fm.lineSpacing() );
        QBoxLayout * lay7 = new QHBoxLayout ( lay2 );
	lay7->addStretch ( 1 );
	update_button = new QPushButton( i18n("Update"), group, "PushButton_7" );
	lay7->addWidget ( update_button );
	lay7->addStretch ( 1 );
	defaults_button = new QPushButton( i18n("Defaults"), group, "PushButton_2" );
	lay7->addWidget ( defaults_button );
	lay7->addStretch ( 1 );
	help_button = new QPushButton( i18n("Help"), group, "PushButton_3" );
	lay7->addWidget ( help_button );
	lay7->addStretch ( 1 );
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
