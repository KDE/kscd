/**********************************************************************

	--- Dlgedit generated file ---

	File: CDDialogData.cpp
	Last generated: Thu Jan 1 15:56:19 1998

	FILE WAS MODIFIED to add layouts

 *********************************************************************/

#include "CDDialogData.h"

#define Inherited QDialog

#include <qframe.h>
#include <qlabel.h>
#include <klocale.h>
#include <kapp.h>

#include <qlayout.h>
#include <qfontmetrics.h>


CDDialogData::CDDialogData
(
	QWidget* parent,
	const char* name
)
	:
	Inherited( parent, name )
{

	QBoxLayout * lay1 = new QVBoxLayout ( this, 10, 5 );

	QLabel * label1 = new QLabel( i18n("Disc Artist / Title"), this, "Label_4" );
	lay1->addWidget ( label1 );
	QBoxLayout * lay2 = new QHBoxLayout ( lay1 );
	titleedit = new QLineEdit( this, "titleedit" );
	lay2->addWidget ( titleedit, 1 );
	titleedit->setMaxLength( 70 );
	ext_info_title_button = new QPushButton( i18n("Ext Info"), this, "PushButton_5" );
	lay2->addWidget ( ext_info_title_button );

	lay1->addSpacing ( 10 );
	QGridLayout * glay = new QGridLayout ( lay1, 2, 2, 5 );
	glay->setColStretch ( 1, 1 );
	QLabel * label2 = new QLabel( i18n("Disc ID: "), this, "Label_7" );
	glay->addWidget ( label2, 0, 0 );
	disc_id_label = new QLabel( this, "diskid_label" );
	glay->addWidget ( disc_id_label, 0, 1, AlignLeft );
	QLabel * label22 = new QLabel( i18n("Total time: "), this );
	glay->addWidget ( label22, 1, 0 );
	total_time_label = new QLabel( this, "diskid_label" );
	glay->addWidget ( total_time_label, 1, 1, AlignLeft );

	lay1->addSpacing ( 10 );
	QLabel * label3 = new QLabel( i18n("Track / Time / Title"), this, "Label_5" );
	lay1->addWidget ( label3 );
	
	listbox = new QListBox( this, "listbox" );
	lay1->addWidget ( listbox );
	connect( listbox, SIGNAL(highlighted(int)), SLOT(titleselected(int)) );

	lay1->addSpacing ( 10 );
	QLabel * label4 = new QLabel( i18n("Edit Track Title"), this, "Label_3" );
	lay1->addWidget ( label4 );
	QBoxLayout * lay4 = new QHBoxLayout ( lay1 );
	trackedit = new QLineEdit( this, "trackedit" );
	lay4->addWidget ( trackedit, 1 );
	trackedit->setMaxLength( 70 );
	ext_info_button = new QPushButton( i18n("Ext Info"), this, "extinfo_button" );
	lay4->addWidget ( ext_info_button );

	lay1->addSpacing ( 10 );
	QLabel * label5 = new QLabel( i18n("Edit Play Sequence"), this, "Label_2" );
	lay1->addWidget ( label5 );
	progseq_edit = new QLineEdit( this, "programsequence_edit" );
	lay1->addWidget ( progseq_edit );
	progseq_edit->setMaxLength( 70 );

	lay1->addSpacing ( 10 );
	QBoxLayout * lay5 = new QHBoxLayout ( lay1 );
	lay5->addStretch ( 1 );
	save_button = new QPushButton( i18n("Save"), this, "save_button" );
	lay5->addWidget ( save_button );
	lay5->addStretch ( 1 );
	load_button = new QPushButton( i18n("Load"), this, "PushButton_3" );
	lay5->addWidget ( load_button );
	lay5->addStretch ( 1 );
	upload_button = new QPushButton( i18n("Upload"), this, "PushButton_8" );
	lay5->addWidget ( upload_button );
	lay5->addStretch ( 1 );
	ok_button = new QPushButton( i18n("Close"), this, "PushButton_4" );
	lay5->addWidget ( ok_button );
	lay5->addStretch ( 1 );
	
	resize ( 400, 500 );
}


CDDialogData::~CDDialogData()
{
}
void CDDialogData::titleselected(int)
{
}

#undef Inherited
