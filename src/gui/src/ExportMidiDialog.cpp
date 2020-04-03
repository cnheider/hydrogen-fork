/*
 * Hydrogen
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * http://www.hydrogen-music.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <QFileDialog>
#include <QLabel>

#include "ExportMidiDialog.h"
#include "Skin.h"
#include "HydrogenApp.h"

#include <hydrogen/basics/song.h>
#include <hydrogen/hydrogen.h>
#include <hydrogen/Preferences.h>
#include <hydrogen/smf/SMF.h>

using namespace H2Core;

const char* ExportMidiDialog::__class_name = "ExportMidiDialog";

enum ExportModes { EXPORT_SMF1_SINGLE, EXPORT_SMF1_MULTI, EXPORT_SMF0 };

ExportMidiDialog::ExportMidiDialog( QWidget* parent )
	: QDialog( parent )
	, Object( __class_name )
	, m_bFileSelected( false )
	, m_sExtension( ".mid" )
	, m_pPreferences( Preferences::get_instance() )
{
	setupUi( this );
	setModal( true );
	setWindowTitle( trUtf8( "Export midi" ) );

	exportTypeCombo->addItem( trUtf8("SMF1 single: export all instruments to a single track") );
	exportTypeCombo->addItem( trUtf8("SMF1 multi: export each instrument to separate track") );
	exportTypeCombo->addItem( trUtf8("SMF0: export all events to one track") );

	restoreSettingsFromPreferences();

	adjustSize();
}

ExportMidiDialog::~ExportMidiDialog()
{
}

void ExportMidiDialog::saveSettingsToPreferences()
{
	m_pPreferences->setMidiExportMode( exportTypeCombo->currentIndex() );
	
	// extracting dirname from export box	
	QString sFilename = exportNameTxt->text();
	QFileInfo info( sFilename );
	QDir dir = info.absoluteDir();
	if ( !dir.exists() ) {
		// very strange if it happens but better to check for it anyway
		return;	
	}
	QString sSelectedDirname = dir.absolutePath();
	m_pPreferences->setMidiExportDirectory( sSelectedDirname );
}

void ExportMidiDialog::restoreSettingsFromPreferences()
{
	// loading previous directory and filling filename text field
	Hydrogen * pHydrogen = Hydrogen::get_instance();

	QString sDefaultFilename = pHydrogen->getSong()->get_filename();

	if( sDefaultFilename.isEmpty() ){
		sDefaultFilename = pHydrogen->getSong()->__name;
	} else {
		// extracting filename from full path
		QFileInfo qDefaultFile( sDefaultFilename ); 
		sDefaultFilename = qDefaultFile.fileName();
	}

	sDefaultFilename.replace( '*', "_" );
	sDefaultFilename.replace( Filesystem::songs_ext, "" );
	sDefaultFilename += m_sExtension;

	QString sDirPath = m_pPreferences->getMidiExportDirectory();
	QDir qd = QDir( sDirPath );
	// joining filepath with dirname
	QString sFullPath = qd.absoluteFilePath( sDefaultFilename );
	exportNameTxt->setText( sFullPath );

	// loading rest of the options
	exportTypeCombo->setCurrentIndex( m_pPreferences->getMidiExportMode() );
}

void ExportMidiDialog::on_browseBtn_clicked()
{
	QFileDialog fd( this );
	QString sPrevDirname = m_pPreferences->getMidiExportDirectory();

	fd.setFileMode( QFileDialog::AnyFile );
	fd.setNameFilter( trUtf8("Midi file (*%1)").arg( m_sExtension ) );
	fd.setDirectory( sPrevDirname );
	fd.setWindowTitle( trUtf8( "Export MIDI file" ) );
	fd.setAcceptMode( QFileDialog::AcceptSave );

	QString sDefaultFilename = exportNameTxt->text();
	fd.selectFile( sDefaultFilename );

	QString sFilename;
	if ( fd.exec() == QDialog::Accepted ) {
		m_bFileSelected = true;
		sFilename = fd.selectedFiles().first();
	}

	if ( sFilename.isEmpty() ) {
		return;
	}

	if ( sFilename.endsWith( m_sExtension ) == false ) {
		sFilename += m_sExtension;
	}

	exportNameTxt->setText( sFilename );
}


void ExportMidiDialog::on_okBtn_clicked()
{
	saveSettingsToPreferences();

	Hydrogen *pEngine = Hydrogen::get_instance();
	Song *pSong = pEngine->getSong();

	// checking file overwrite
	QString sFilename = exportNameTxt->text();
	QFileInfo qFile( sFilename );
	
	if ( qFile.exists() == true && m_bFileSelected == false ) {
		int res = QMessageBox::information( this, "Hydrogen", tr( "The file %1 exists. \nOverwrite the existing file?").arg(sFilename), QMessageBox::Yes | QMessageBox::No );
		if ( res == QMessageBox::No ) {
			return;
		}
	}

	// choosing writer
	SMFWriter *pSmfWriter;
	if( exportTypeCombo->currentIndex() == EXPORT_SMF1_SINGLE ){
		pSmfWriter = new SMF1WriterSingle();
	} else if ( exportTypeCombo->currentIndex() == EXPORT_SMF1_MULTI ){
		pSmfWriter = new SMF1WriterMulti();
	} else if ( exportTypeCombo->currentIndex() == EXPORT_SMF0 ){
		pSmfWriter = new SMF0Writer();
	}

	pSmfWriter->save( sFilename, pSong );

	delete pSmfWriter;
	accept();
}

void ExportMidiDialog::on_closeBtn_clicked()
{
	accept();
}


void ExportMidiDialog::on_exportNameTxt_textChanged( const QString& )
{
	QString filename = exportNameTxt->text();
	if ( !filename.isEmpty() ) {
		okBtn->setEnabled( true );
	}
	else {
		okBtn->setEnabled( false );
	}
}
