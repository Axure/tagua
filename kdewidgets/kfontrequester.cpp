/*
    Copyright (C) 2003 Nadeem Hasan <nhasan@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kfontrequester.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>

//#include <kfontdialog.h>
//#include <klocale.h>

#include <QFontDialog>
#include <QApplication>

inline QString i18n(const char* s) { return QApplication::tr(s); }
inline QString i18n(const char* s, const QString& a) { return QApplication::tr(s).arg(a); }

KFontRequester::KFontRequester( QWidget *parent, bool onlyFixed )
    : QWidget( parent ),
    m_onlyFixed( onlyFixed )
{
  QHBoxLayout *layout = new QHBoxLayout( this );
  //layout->setSpacing( KDialog::spacingHint() );

  m_sampleLabel = new QLabel( this );
  m_button = new QPushButton( i18n( "Choose..." ), this );

  m_sampleLabel->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
  setFocusProxy( m_button );

  layout->addWidget( m_sampleLabel, 1 );
  layout->addWidget( m_button );

  connect( m_button, SIGNAL( clicked() ), SLOT( buttonClicked() ) );

  displaySampleText();
  setToolTip();
}

void KFontRequester::setFont( const QFont &font, bool onlyFixed )
{
  m_selFont = font;
  m_onlyFixed = onlyFixed;

  displaySampleText();
  emit fontSelected( m_selFont );
}

void KFontRequester::setSampleText( const QString &text )
{
  m_sampleText = text;
  displaySampleText();
}

void KFontRequester::setTitle( const QString &title )
{
  m_title = title;
  setToolTip();
}

void KFontRequester::buttonClicked()
{
  //int result = KFontDialog::getFont( m_selFont, m_onlyFixed, parentWidget() );

  //if ( result == KDialog::Accepted )

  bool ok;
  QFont f = QFontDialog::getFont( &ok, m_selFont, parentWidget() );

  if(ok)
  {
    m_selFont = f;
    displaySampleText();
    emit fontSelected( m_selFont );
  }
}

void KFontRequester::displaySampleText()
{
  m_sampleLabel->setFont( m_selFont );

  int size = m_selFont.pointSize();
  if(size == -1)
    size = m_selFont.pixelSize();

  if ( m_sampleText.isEmpty() )
    m_sampleLabel->setText( QString( "%1 %2" ).arg( m_selFont.family() )
      .arg( size ) );
  else
    m_sampleLabel->setText( m_sampleText );
}

void KFontRequester::setToolTip()
{
  m_button->setToolTip( i18n( "Click to select a font" ) );

  m_sampleLabel->setToolTip( QString() );
  m_sampleLabel->setWhatsThis(QString());

  if ( m_title.isNull() )
  {
    m_sampleLabel->setToolTip( i18n( "Preview of the selected font" ) );
    m_sampleLabel->setWhatsThis(        i18n( "This is a preview of the selected font. You can change it"
        " by clicking the \"Choose...\" button." ) );
  }
  else
  {
    m_sampleLabel->setToolTip( i18n( "Preview of the \"%1\" font" ,  m_title ) );
    m_sampleLabel->setWhatsThis(        i18n( "This is a preview of the \"%1\" font. You can change it"
        " by clicking the \"Choose...\" button." ,  m_title ) );
  }
}

#include "kfontrequester.moc"

/* vim: et sw=2 ts=2
*/
