/**********************************************************************

	--- Qt Architect generated file ---

	File: CDDBSetupData.h
	Last generated: Wed Apr 15 20:39:43 1998

	DO NOT EDIT!!!  This file will be automatically
	regenerated by qtarch.  All changes will be lost.

 *********************************************************************/

#ifndef CDDBSetupData_included
#define CDDBSetupData_included

#include <qwidget.h>
#include <qlabel.h>
#include <qchkbox.h>
#include <qpushbt.h>
#include <qlistbox.h>
#include <qlined.h>

class CDDBSetupData : public QWidget
{
    Q_OBJECT

public:

    CDDBSetupData
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~CDDBSetupData();

public slots:


protected slots:

    virtual void http_access_toggled(bool);

protected:
    QLineEdit* basedir_edit;
    QLineEdit* submit_edit;
    QListBox* server_listbox;
    QPushButton* update_button;
    QLabel* current_server_label;
    QCheckBox* remote_cddb_cb;
    QPushButton* defaults_button;
    QPushButton* help_button;
    QCheckBox* cddb_http_cb;
    QLineEdit* proxy_port_ef;
    QLineEdit* proxy_host_ef;

};

#endif // CDDBSetupData_included
