#include "CDDialogData.h"

#include <qframe.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qheader.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qfontmetrics.h>

#include <klocale.h>
#include <kapp.h>

CDDialogData::CDDialogData ( QWidget* parent, const char* name) : QDialog( parent, name ) {
    QVBoxLayout *dialogLayout = new QVBoxLayout( this, 11, 6);

    QGroupBox *discGroupBox = new QGroupBox( this, "Disc" );
    discGroupBox->setTitle( i18n( "Disc" ) );
    discGroupBox->setColumnLayout(0, Qt::Vertical );
    discGroupBox->layout()->setSpacing( 6 );
    discGroupBox->layout()->setMargin( 11 );
    QGridLayout *discGroupBoxLayout = new QGridLayout( discGroupBox->layout() );
    discGroupBoxLayout->setAlignment( Qt::AlignTop );

    artistEdit = new QLineEdit( discGroupBox, "artist" );
    discGroupBoxLayout->addMultiCellWidget( artistEdit, 0, 0, 1, 3 );

    titleEdit = new QLineEdit( discGroupBox, "titleEdit" );
    discGroupBoxLayout->addMultiCellWidget( titleEdit, 1, 1, 1, 3 );

    QLabel *diskID = new QLabel( i18n( "Disc ID:" ),discGroupBox, "diskID" );
    discGroupBoxLayout->addWidget( diskID, 2, 0 );

    QLabel *timeLabel = new QLabel( i18n( "Total Time:" ), discGroupBox, "timeLabel" );
    discGroupBoxLayout->addWidget( timeLabel, 3, 0 );

    QLabel *titleLabel = new QLabel(titleEdit, i18n( "&Title:" ),discGroupBox, "title" );
    discGroupBoxLayout->addWidget( titleLabel, 1, 0 );

    QLabel *artistLabel = new QLabel(artistEdit, i18n( "&Artist:" ), discGroupBox, "artistLabel" );
    discGroupBoxLayout->addWidget( artistLabel, 0, 0 );

    disc_id_label = new QLabel( i18n( "id" ), discGroupBox, "idLabel" );
    discGroupBoxLayout->addWidget( disc_id_label, 2, 1 );

    total_time_label = new QLabel(i18n( "time" ),  discGroupBox, "timeLabel" );
    discGroupBoxLayout->addWidget( total_time_label, 3, 1 );

    ext_info_title_button = new QPushButton( i18n( "Comment" ), discGroupBox, "discComment" );
    discGroupBoxLayout->addMultiCellWidget( ext_info_title_button, 2, 3, 3, 3 );

    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    discGroupBoxLayout->addItem( spacer, 2, 2 );
    dialogLayout->addWidget( discGroupBox );

    QGroupBox *tracksGroup = new QGroupBox( this, "TracksGroup" );
    tracksGroup->setTitle( i18n( "Tracks" ) );
    tracksGroup->setColumnLayout(0, Qt::Vertical );
    tracksGroup->layout()->setSpacing( 6 );
    tracksGroup->layout()->setMargin( 11 );
    QVBoxLayout *gbLayout = new QVBoxLayout( tracksGroup->layout() );
    gbLayout->setAlignment( Qt::AlignTop );

    tracksList = new QListView( tracksGroup, "TracksList" );
    tracksList->addColumn( i18n( "Nr" ) );
    tracksList->addColumn( i18n( "Time" ) );
    tracksList->header()->setClickEnabled( FALSE, tracksList->header()->count() - 1 );
    tracksList->addColumn( i18n( "Title" ) );
    tracksList->header()->setClickEnabled( FALSE, tracksList->header()->count() - 1 );

    gbLayout->addWidget( tracksList );

    QHBoxLayout *layout1 = new QHBoxLayout( 0, 0, 6);

    trackEdit = new QLineEdit( tracksGroup, "trackTitleEdit" );
	trackEdit->setMaxLength( 70 );
	trackEdit->setEnabled(false);
    QLabel *trackTitleLabel = new QLabel( trackEdit, i18n( "Title:" ), tracksGroup, "trackTitleLabel" );
    layout1->addWidget( trackTitleLabel );
    layout1->addWidget( trackEdit );

    ext_info_button = new QPushButton( i18n( "Comment" ), tracksGroup, "ext_info_button" );
    layout1->addWidget( ext_info_button );
    gbLayout->addLayout( layout1 );
    dialogLayout->addWidget( tracksGroup );

    QHBoxLayout *layout2 = new QHBoxLayout( 0, 0, 6);

    progseq_edit = new QLineEdit( this, "playingOrderEdit" );
    QLabel *playingOrderLabel = new QLabel( progseq_edit, i18n( "Playing order:" ), this, "playingOrder" );
	progseq_edit->setMaxLength( 70 );
    layout2->addWidget( playingOrderLabel );
    layout2->addWidget( progseq_edit );
    dialogLayout->addLayout( layout2 );

    QHBoxLayout *layout3 = new QHBoxLayout( 0, 0, 6);
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout3->addItem( spacer_2 );

    upload_button = new QPushButton( i18n( "Upload" ), this, "uploadButton" );
    layout3->addWidget( upload_button );

    ok_button = new QPushButton( i18n( "&OK" ), this, "okButton" );
    ok_button->setDefault(true);
    layout3->addWidget( ok_button );

    cancel_button = new QPushButton( i18n( "&Cancel" ), this, "cancelButton" );
    layout3->addWidget( cancel_button );
    dialogLayout->addLayout( layout3 );

	resize ( 400, 500 );
}


CDDialogData::~CDDialogData()
{
}
void CDDialogData::titleselected(int)
{
}

