/*
 * Hydrogen
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 * Copyright(c) 2008-2021 The hydrogen development team [hydrogen-devel@lists.sourceforge.net]
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
 * along with this program. If not, see https://www.gnu.org/licenses
 *
 */
#include <unistd.h>


#include <core/Preferences/Preferences.h>
#include <core/AudioEngine/AudioEngine.h>
#include <core/EventQueue.h>
#include <core/CoreActionController.h>
#include <core/Hydrogen.h>
#include <core/Basics/Pattern.h>
#include <core/Basics/PatternList.h>
#include <core/IO/DiskWriterDriver.h>

#include <pthread.h>
#include <cassert>

#if defined(WIN32) || _DOXYGEN_
#include <windows.h>
/*
 * In Windows the unistd function sleep( seconds ) is not available.
 * Treat sleep( SECONDS ) as a macro that uses SleepEx.
 * Convert seconds to milliseconds for the first argument of SleepEx.
 * Use false for the second argument of SleepEx.
 * This way SleepEx always returns 0, after the specified time has passed.
 */
#define sleep( SECONDS ) SleepEx( SECONDS * 1000, false )
#endif

namespace H2Core
{

pthread_t diskWriterDriverThread;

void* diskWriterDriver_thread( void* param )
{
	Base * __object = ( Base * )param;
	DiskWriterDriver *pDriver = ( DiskWriterDriver* )param;

	EventQueue::get_instance()->push_event( EVENT_PROGRESS, 0 );

	auto pAudioEngine = Hydrogen::get_instance()->getAudioEngine();
	
	__INFOLOG( "DiskWriterDriver thread start" );

	// always rolling, no user interaction
	pAudioEngine->play();
	SF_INFO soundInfo;
	soundInfo.samplerate = pDriver->m_nSampleRate;
//	soundInfo.frames = -1;//getNFrames();		///\todo: da terminare
	soundInfo.channels = 2;
	//default format
	int sfformat = 0x010000; //wav format (default)
	int bits = 0x0002; //16 bit PCM (default)
	//sf_format switch
	if( pDriver->m_sFilename.endsWith(".aiff") || pDriver->m_sFilename.endsWith(".AIFF") ){
		sfformat =  0x020000; //Apple/SGI AIFF format (big endian)
	}
	if( pDriver->m_sFilename.endsWith(".flac") || pDriver->m_sFilename.endsWith(".FLAC") ){
		sfformat =  0x170000; //FLAC lossless file format
	}
	if( ( pDriver->m_nSampleDepth == 8 ) && ( pDriver->m_sFilename.endsWith(".aiff") || pDriver->m_sFilename.endsWith(".AIFF") ) ){
		bits = 0x0001; //Signed 8 bit data works with aiff
	}
	if( ( pDriver->m_nSampleDepth == 8 ) && ( pDriver->m_sFilename.endsWith(".wav") || pDriver->m_sFilename.endsWith(".WAV") ) ){
		bits = 0x0005; //Unsigned 8 bit data needed for Microsoft WAV format
	}
	if( pDriver->m_nSampleDepth == 16 ){
		bits = 0x0002; //Signed 16 bit data
	}
	if( pDriver->m_nSampleDepth == 24 ){
		bits = 0x0003; //Signed 24 bit data
	}
	if( pDriver->m_nSampleDepth == 32 ){
		bits = 0x0004; ////Signed 32 bit data
	}

	soundInfo.format =  sfformat|bits;

//	#ifdef HAVE_OGGVORBIS

	//ogg vorbis option
	if( pDriver->m_sFilename.endsWith( ".ogg" ) | pDriver->m_sFilename.endsWith( ".OGG" ) ) {
		soundInfo.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS;
	}
//	#endif


///formats
//          SF_FORMAT_WAV          = 0x010000,     /* Microsoft WAV format (little endian). */
//          SF_FORMAT_AIFF         = 0x020000,     /* Apple/SGI AIFF format (big endian). */
//          SF_FORMAT_AU           = 0x030000,     /* Sun/NeXT AU format (big endian). */
//          SF_FORMAT_RAW          = 0x040000,     /* RAW PCM data. */
//          SF_FORMAT_PAF          = 0x050000,     /* Ensoniq PARIS file format. */
//          SF_FORMAT_SVX          = 0x060000,     /* Amiga IFF / SVX8 / SV16 format. */
//          SF_FORMAT_NIST         = 0x070000,     /* Sphere NIST format. */
//          SF_FORMAT_VOC          = 0x080000,     /* VOC files. */
//          SF_FORMAT_IRCAM        = 0x0A0000,     /* Berkeley/IRCAM/CARL */
//          SF_FORMAT_W64          = 0x0B0000,     /* Sonic Foundry's 64 bit RIFF/WAV */
//          SF_FORMAT_MAT4         = 0x0C0000,     /* Matlab (tm) V4.2 / GNU Octave 2.0 */
//          SF_FORMAT_MAT5         = 0x0D0000,     /* Matlab (tm) V5.0 / GNU Octave 2.1 */
//          SF_FORMAT_PVF          = 0x0E0000,     /* Portable Voice Format */
//          SF_FORMAT_XI           = 0x0F0000,     /* Fasttracker 2 Extended Instrument */
//          SF_FORMAT_HTK          = 0x100000,     /* HMM Tool Kit format */
//          SF_FORMAT_SDS          = 0x110000,     /* Midi Sample Dump Standard */
//          SF_FORMAT_AVR          = 0x120000,     /* Audio Visual Research */
//          SF_FORMAT_WAVEX        = 0x130000,     /* MS WAVE with WAVEFORMATEX */
//          SF_FORMAT_SD2          = 0x160000,     /* Sound Designer 2 */
//          SF_FORMAT_FLAC         = 0x170000,     /* FLAC lossless file format */
//          SF_FORMAT_CAF          = 0x180000,     /* Core Audio File format */
//	    SF_FORMAT_OGG
///bits
//          SF_FORMAT_PCM_S8       = 0x0001,       /* Signed 8 bit data */
//          SF_FORMAT_PCM_16       = 0x0002,       /* Signed 16 bit data */
//          SF_FORMAT_PCM_24       = 0x0003,       /* Signed 24 bit data */
//          SF_FORMAT_PCM_32       = 0x0004,       /* Signed 32 bit data */
///used for ogg
//          SF_FORMAT_VORBIS

	if ( !sf_format_check( &soundInfo ) ) {
		__ERRORLOG( "Error in soundInfo" );
		return nullptr;
	}


	SNDFILE* m_file = sf_open( pDriver->m_sFilename.toLocal8Bit(), SFM_WRITE, &soundInfo );

	float *pData = new float[ pDriver->m_nBufferSize * 2 ];	// always stereo

	float *pData_L = pDriver->m_pOut_L;
	float *pData_R = pDriver->m_pOut_R;


	Hydrogen* pHydrogen = Hydrogen::get_instance();
	auto pSong = pHydrogen->getSong();

	std::vector<PatternList*> *pPatternColumns = pSong->getPatternGroupVector();
	int nColumns = pPatternColumns->size();
	
	int nPatternSize;
	float fBpm;
	float fTicksize = 0;
	for ( int patternPosition = 0; patternPosition < nColumns; ++patternPosition ) {

		
		PatternList *pColumn = ( *pPatternColumns )[ patternPosition ];
		if ( pColumn->size() != 0 ) {
			nPatternSize = pColumn->longest_pattern_length();
		} else {
			nPatternSize = MAX_NOTES;
		}

		fBpm = AudioEngine::getBpmAtColumn( patternPosition );
		fTicksize = AudioEngine::computeTickSize( pDriver->m_nSampleRate, fBpm,
												  pSong->getResolution() );
		
		//here we have the pattern length in frames dependent from bpm and samplerate
		unsigned patternLengthInFrames = fTicksize * nPatternSize;
		unsigned frameNumber = 0;
		int lastRun = 0;
		while ( frameNumber < patternLengthInFrames ) {
			
			int usedBuffer = pDriver->m_nBufferSize;
			
			//this will calculate the size from -last- (end of pattern) used frame buffer,
			//which is mostly smaller than pDriver->m_nBufferSize
			if( patternLengthInFrames - frameNumber <  pDriver->m_nBufferSize ){
				lastRun = patternLengthInFrames - frameNumber;
				usedBuffer = lastRun;
			};
			
			frameNumber += usedBuffer;
			
			//pDriver->m_transport.m_nFrames = frameNumber;
			
			int ret = pDriver->m_processCallback( usedBuffer, nullptr );
			while( ret != 0) {
				ret = pDriver->m_processCallback( usedBuffer, nullptr );
			}
			
			for ( unsigned i = 0; i < usedBuffer; i++ ) {
				if(pData_L[i] > 1){
					pData[i * 2] = 1;
				}
				else if(pData_L[i] < -1){
					pData[i * 2] = -1;
				}else
				{
					pData[i * 2] = pData_L[i];
				}
				
				if(pData_R[i] > 1){
					pData[i * 2 + 1] = 1;
				}
				else if(pData_R[i] < -1){
					pData[i * 2 + 1] = -1;
				}else
				{
					pData[i * 2 + 1] = pData_R[i];
				}
			}
			int res = sf_writef_float( m_file, pData, usedBuffer );
			if ( res != ( int )usedBuffer ) {
				__ERRORLOG( "Error during sf_write_float" );
			}
		}
		
		// this progress bar method is not exact but ok enough to give users a usable visible progress feedback
		float fPercent = ( float )(patternPosition +1) / ( float )nColumns * 100.0;
		EventQueue::get_instance()->push_event( EVENT_PROGRESS, ( int )fPercent );
	}
	delete[] pData;
	pData = nullptr;

	sf_close( m_file );

	__INFOLOG( "DiskWriterDriver thread end" );

	pthread_exit( nullptr );
	return nullptr;
}



DiskWriterDriver::DiskWriterDriver( audioProcessCallback processCallback, unsigned nSamplerate, int nSampleDepth )
		: AudioOutput()
		, m_nSampleRate( nSamplerate )
		, m_nSampleDepth ( nSampleDepth )
		, m_processCallback( processCallback )
		, m_nBufferSize( 0 )
		, m_pOut_L( nullptr )
		, m_pOut_R( nullptr ) {
}



DiskWriterDriver::~DiskWriterDriver() {
}



int DiskWriterDriver::init( unsigned nBufferSize )
{
	INFOLOG( QString( "Init, buffer size: %1" ).arg( nBufferSize ) );

	m_nBufferSize = nBufferSize;

	return 0;
}


///
/// Connect
/// return 0: Ok
///
int DiskWriterDriver::connect()
{
	INFOLOG( "" );
	
	m_pOut_L = new float[ m_nBufferSize ];
	m_pOut_R = new float[ m_nBufferSize ];
	
	pthread_attr_t attr;
	pthread_attr_init( &attr );

	pthread_create( &diskWriterDriverThread, &attr, diskWriterDriver_thread, this );
	
	return 0;
}


/// disconnect
void DiskWriterDriver::disconnect()
{
	INFOLOG( "" );
	delete[] m_pOut_L;
	m_pOut_L = nullptr;

	delete[] m_pOut_R;
	m_pOut_R = nullptr;

}

unsigned DiskWriterDriver::getSampleRate()
{
	return m_nSampleRate;
}
};
