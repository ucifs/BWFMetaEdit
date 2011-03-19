// BWF MetaEdit GUI - A GUI for BWF MetaEdit
//
// This code was created in 2010 for the Library of Congress and the
// other federal government agencies participating in the Federal Agencies
// Digitization Guidelines Initiative and it is in the public domain.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#include "GUI/Qt/GUI_Main_xxxx_TimeReferenceDialog.h"
#include "Common/Core.h"
#include "ZenLib/Ztring.h"
#include "ZenLib/File.h"
#include <QtGui/QTimeEdit>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtCore/QEvent>
#include <QtGui/QGridLayout>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QIcon>
#include <QtGui/QMessageBox>
#include <ZenLib/Ztring.h>
using namespace ZenLib;
//---------------------------------------------------------------------------

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
GUI_Main_xxxx_TimeReferenceDialog::GUI_Main_xxxx_TimeReferenceDialog(Core* _C, const std::string &FileName_, const std::string &Field_, QWidget* parent)
: QDialog(parent)
{
    //Internal
    C=_C;
    FileName=FileName_;
    Field=Field_;

    //Configuration
    setWindowFlags(windowFlags()&(0xFFFFFFFF-Qt::WindowContextHelpButtonHint));
    setWindowTitle("TimeReference");
    setWindowIcon (QIcon(":/Image/FADGI/Logo.png"));

    //Buttons
    QDialogButtonBox* Dialog=new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(Dialog, SIGNAL(accepted()), this, SLOT(OnAccept()));
    connect(Dialog, SIGNAL(rejected()), this, SLOT(reject()));

    TimeEdit=new QTimeEdit(this);
    TimeEdit->setDisplayFormat("hh:mm:ss.zzz");
    connect(TimeEdit, SIGNAL(timeChanged (const QTime&)), this, SLOT(OnTimeChanged (const QTime&)));
    TimeEdit_Label=new QLabel(this);
    TimeEdit_Label->setText(tr("(HH:MM:SS.mmm)"));
    Label=new QLabel(this);
    Label->setText(tr("or"));
    LineEdit=new QLineEdit(this);
    connect(LineEdit, SIGNAL(textEdited (const QString &)), this, SLOT(OnValueEdited (const QString &)));
    LimeEdit_Label=new QLabel(this);
    LimeEdit_Label->setText(tr("(in samples)"));
        
    QGridLayout* L=new QGridLayout();
    L->addWidget(TimeEdit, 0, 0);
    L->addWidget(TimeEdit_Label, 0, 1);
    L->addWidget(Label, 1, 0, 1, 2);
    L->addWidget(LineEdit, 2, 0);
    L->addWidget(LimeEdit_Label, 2, 1);
    L->addWidget(Dialog, 3, 0, 1, 2);

    setLayout(L);

    //Filling
    SampleRate=Ztring(C->Get(FileName, "Sample rate")).To_int64u();
    Ztring TimeReference=C->Get(FileName, "TimeReference");
    Ztring TimeReferenceS=C->Get(FileName, "TimeReference (translated)");
    TimeEdit->setTime(QTime::fromString(QString().fromUtf8(TimeReferenceS.To_Local().c_str()), Qt::ISODate));
    LineEdit->setText(QString().fromUtf8(TimeReference.To_Local().c_str()));
    if (LineEdit->text().isEmpty())
        LineEdit->setText("0");
    IsChanging=false;
}

//***************************************************************************
// Menu actions
//***************************************************************************

//---------------------------------------------------------------------------
void GUI_Main_xxxx_TimeReferenceDialog::OnAccept ()
{
    std::string Value=LineEdit->text().toLocal8Bit().data();
    if (!C->IsValid(FileName, Field, Value))
    {
        QMessageBox MessageBox;
        MessageBox.setWindowTitle("BWF MetaEdit");
        MessageBox.setText((string("Field does not conform to rules:\n")+C->IsValid_LastError(FileName)).c_str());
        #if (QT_VERSION >= 0x040200)
            MessageBox.setStandardButtons(QMessageBox::Ok);
        #endif // (QT_VERSION >= 0x040200)
        MessageBox.setIcon(QMessageBox::Warning);
        MessageBox.setWindowIcon(QIcon(":/Image/FADGI/Logo.png"));
        MessageBox.exec();
        return;
    }

    C->Set(FileName, Field, Value);

    accept();
}

//---------------------------------------------------------------------------
void GUI_Main_xxxx_TimeReferenceDialog::OnTimeChanged (const QTime &Time)
{
    if (IsChanging || SampleRate==0)
        return;

    LineEdit->setText(Ztring::ToZtring((((int64u)QTime().msecsTo(Time))*SampleRate/1000)).c_str());
}

//---------------------------------------------------------------------------
void GUI_Main_xxxx_TimeReferenceDialog::OnValueEdited (const QString &Value)
{
    QString Text=LineEdit->text();
    if (Text.size()>18)
        Text.resize(18);
    for (int Pos=0; Pos<Text.size(); Pos++)
    {
        if (Pos==0 && Text[0]=='0')
        {
            Text.remove(0, 1);
            Pos--;
        }
        else if (Text[Pos]<'0' || Text[Pos]>'9')
        {
            Text.remove(Pos, 1);
            Pos--;
        }
    }
    if (Text.isEmpty())
        Text="0";
    if (Text!=LineEdit->text())
        LineEdit->setText(Text);
    
    if (IsChanging || SampleRate==0)
        return;

    int64u TimeReference=Ztring(LineEdit->text().toLocal8Bit().data()).To_int64u();
    TimeReference=(int64u)(((float64)TimeReference)/(((float64)SampleRate)/1000));
    IsChanging=true;
    if (TimeReference>23*60*60*1000+59*60*1000+59*1000+999)
    {
        TimeEdit->setEnabled(false);
        TimeEdit->setTime(QTime().addMSecs(23*60*60*1000+59*60*1000+59*1000+999));
    }
    else
    {
        TimeEdit->setEnabled(true);
        TimeEdit->setTime(QTime().addMSecs((int)TimeReference));
    }
    IsChanging=false;
}
