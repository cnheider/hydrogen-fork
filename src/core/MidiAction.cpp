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
#include <QObject>

#include <core/AudioEngine/AudioEngine.h>
#include <core/EventQueue.h>
#include <core/CoreActionController.h>
#include <core/Hydrogen.h>

#include <core/Basics/Instrument.h>
#include <core/Basics/InstrumentComponent.h>
#include <core/Basics/InstrumentLayer.h>
#include <core/Basics/InstrumentList.h>
#include <core/Basics/Playlist.h>
#include <core/Basics/Song.h>
#include <core/Basics/PatternList.h>

#include <core/Preferences.h>
#include <core/MidiAction.h>

#include <core/Basics/Drumkit.h>

// #include <QFileInfo>

#include <sstream>

using namespace H2Core;

/**
* @class MidiAction
*
* @brief This class represents a midi action.
*
* This class represents actions which can be executed
* after a midi event occurred. An example is the "MUTE"
* action, which mutes the outputs of hydrogen.
*
* An action can be linked to an event. If this event occurs,
* the action gets triggered. The handling of events takes place
* in midi_input.cpp .
*
* Each action has two independent parameters. The two parameters are optional and
* can be used to carry additional information, which mean
* only something to this very Action. They can have totally different meanings for other Actions.
* Example: parameter1 is the Mixer strip and parameter 2 a multiplier for the volume change on this strip
*
* @author Sebastian Moors
*
*/

Action::Action( QString typeString ) {
	m_sType = typeString;
	m_sParameter1 = "0";
	m_sParameter2 = "0";
	m_sParameter3 = "0";
	m_sValue = "0";
}

/**
* @class MidiActionManager
*
* @brief The MidiActionManager cares for the execution of MidiActions
*
*
* The MidiActionManager handles the execution of midi actions. The class
* includes the names and implementations of all possible actions.
*
*
* @author Sebastian Moors
*
*/
MidiActionManager* MidiActionManager::__instance = nullptr;

MidiActionManager::MidiActionManager() {
	__instance = this;

	m_nLastBpmChangeCCParameter = -1;
	/*
		the actionMap holds all Action identifiers which hydrogen is able to interpret.
		it holds pointer to member function
	*/
	actionMap.insert(std::make_pair("PLAY", &MidiActionManager::play ));
	actionMap.insert(std::make_pair("PLAY/STOP_TOGGLE", &MidiActionManager::play_stop_pause_toggle ));
	actionMap.insert(std::make_pair("PLAY/PAUSE_TOGGLE", &MidiActionManager::play_stop_pause_toggle ));
	actionMap.insert(std::make_pair("STOP", &MidiActionManager::stop ));
	actionMap.insert(std::make_pair("PAUSE", &MidiActionManager::pause ));
	actionMap.insert(std::make_pair("RECORD_READY", &MidiActionManager::record_ready ));
	actionMap.insert(std::make_pair("RECORD/STROBE_TOGGLE", &MidiActionManager::record_strobe_toggle ));
	actionMap.insert(std::make_pair("RECORD_STROBE", &MidiActionManager::record_strobe ));
	actionMap.insert(std::make_pair("RECORD_EXIT", &MidiActionManager::record_exit ));
	actionMap.insert(std::make_pair("MUTE", &MidiActionManager::mute ));
	actionMap.insert(std::make_pair("UNMUTE", &MidiActionManager::unmute ));
	actionMap.insert(std::make_pair("MUTE_TOGGLE", &MidiActionManager::mute_toggle ));
	actionMap.insert(std::make_pair("STRIP_MUTE_TOGGLE", &MidiActionManager::strip_mute_toggle ));
	actionMap.insert(std::make_pair("STRIP_SOLO_TOGGLE", &MidiActionManager::strip_solo_toggle ));	
	actionMap.insert(std::make_pair(">>_NEXT_BAR", &MidiActionManager::next_bar ));
	actionMap.insert(std::make_pair("<<_PREVIOUS_BAR", &MidiActionManager::previous_bar ));
	actionMap.insert(std::make_pair("BPM_INCR", &MidiActionManager::bpm_increase ));
	actionMap.insert(std::make_pair("BPM_DECR", &MidiActionManager::bpm_decrease ));
	actionMap.insert(std::make_pair("BPM_CC_RELATIVE", &MidiActionManager::bpm_cc_relative ));
	actionMap.insert(std::make_pair("BPM_FINE_CC_RELATIVE", &MidiActionManager::bpm_fine_cc_relative ));
	actionMap.insert(std::make_pair("MASTER_VOLUME_RELATIVE", &MidiActionManager::master_volume_relative ));
	actionMap.insert(std::make_pair("MASTER_VOLUME_ABSOLUTE", &MidiActionManager::master_volume_absolute ));
	actionMap.insert(std::make_pair("STRIP_VOLUME_RELATIVE", &MidiActionManager::strip_volume_relative ));
	actionMap.insert(std::make_pair("STRIP_VOLUME_ABSOLUTE", &MidiActionManager::strip_volume_absolute ));
	actionMap.insert(std::make_pair("EFFECT_LEVEL_ABSOLUTE", &MidiActionManager::effect_level_absolute ));
	actionMap.insert(std::make_pair("EFFECT_LEVEL_RELATIVE", &MidiActionManager::effect_level_relative ));
	actionMap.insert(std::make_pair("GAIN_LEVEL_ABSOLUTE", &MidiActionManager::gain_level_absolute ));
	actionMap.insert(std::make_pair("PITCH_LEVEL_ABSOLUTE", &MidiActionManager::pitch_level_absolute ));
	actionMap.insert(std::make_pair("SELECT_NEXT_PATTERN", &MidiActionManager::select_next_pattern ));
	actionMap.insert(std::make_pair("SELECT_ONLY_NEXT_PATTERN", &MidiActionManager::select_only_next_pattern ));
	actionMap.insert(std::make_pair("SELECT_NEXT_PATTERN_CC_ABSOLUTE", &MidiActionManager::select_next_pattern_cc_absolute ));
	actionMap.insert(std::make_pair("SELECT_NEXT_PATTERN_RELATIVE", &MidiActionManager::select_next_pattern_relative ));
	actionMap.insert(std::make_pair("SELECT_AND_PLAY_PATTERN", &MidiActionManager::select_and_play_pattern ));
	actionMap.insert(std::make_pair("PAN_RELATIVE", &MidiActionManager::pan_relative ));
	actionMap.insert(std::make_pair("PAN_ABSOLUTE", &MidiActionManager::pan_absolute ));
	actionMap.insert(std::make_pair("FILTER_CUTOFF_LEVEL_ABSOLUTE", &MidiActionManager::filter_cutoff_level_absolute ));
	actionMap.insert(std::make_pair("BEATCOUNTER", &MidiActionManager::beatcounter ));
	actionMap.insert(std::make_pair("TAP_TEMPO", &MidiActionManager::tap_tempo ));
	actionMap.insert(std::make_pair("PLAYLIST_SONG", &MidiActionManager::playlist_song ));
	actionMap.insert(std::make_pair("PLAYLIST_NEXT_SONG", &MidiActionManager::playlist_next_song ));
	actionMap.insert(std::make_pair("PLAYLIST_PREV_SONG", &MidiActionManager::playlist_previous_song ));
	actionMap.insert(std::make_pair("TOGGLE_METRONOME", &MidiActionManager::toggle_metronome ));
	actionMap.insert(std::make_pair("SELECT_INSTRUMENT", &MidiActionManager::select_instrument ));
	actionMap.insert(std::make_pair("UNDO_ACTION", &MidiActionManager::undo_action ));
	actionMap.insert(std::make_pair("REDO_ACTION", &MidiActionManager::redo_action ));
	/*
	  the actionList holds all Action identfiers which hydrogen is able to interpret.
	*/
	actionList <<"";
	for(std::map<std::string, action_f>::const_iterator actionIterator = actionMap.begin();
	    actionIterator != actionMap.end();
	    ++actionIterator) {
		actionList << actionIterator->first.c_str();
	}

	eventList << ""
			  << "MMC_PLAY"
			  << "MMC_DEFERRED_PLAY"
			  << "MMC_STOP"
			  << "MMC_FAST_FORWARD"
			  << "MMC_REWIND"
			  << "MMC_RECORD_STROBE"
			  << "MMC_RECORD_EXIT"
			  << "MMC_RECORD_READY"
			  << "MMC_PAUSE"
			  << "NOTE"
			  << "CC"
			  << "PROGRAM_CHANGE";
}


MidiActionManager::~MidiActionManager() {
	//INFOLOG( "ActionManager delete" );
	__instance = nullptr;
}

void MidiActionManager::create_instance() {
	if ( __instance == nullptr ) {
		__instance = new MidiActionManager;
	}
}

bool MidiActionManager::play(Action * , Hydrogen* pHydrogen ) {
	if ( pHydrogen->getAudioEngine()->getState() == AudioEngine::State::Ready ) {
		pHydrogen->sequencer_play();
	}
	return true;
}

bool MidiActionManager::pause(Action * , Hydrogen* pHydrogen ) {
	pHydrogen->sequencer_stop();
	return true;
}

bool MidiActionManager::stop(Action * , Hydrogen* pHydrogen ) {
	pHydrogen->sequencer_stop();
	pHydrogen->getCoreActionController()->locateToColumn( 0 );
	return true;
}

bool MidiActionManager::play_stop_pause_toggle(Action * pAction, Hydrogen* pHydrogen ) {
	QString sActionString = pAction->getType();
	switch ( pHydrogen->getAudioEngine()->getState() )
	{
	case AudioEngine::State::Ready:
		pHydrogen->sequencer_play();
		break;

	case AudioEngine::State::Playing:
		if( sActionString == "PLAY/STOP_TOGGLE" ) {
			pHydrogen->getCoreActionController()->locateToColumn( 0 );
		}
		pHydrogen->sequencer_stop();
		break;

	default:
		ERRORLOG( "[Hydrogen::ActionManager(PLAY): Unhandled case" );
		break;
	}

	return true;
}

//mutes the master, not a single strip
bool MidiActionManager::mute(Action * , Hydrogen* pHydrogen ) {
	pHydrogen->getCoreActionController()->setMasterIsMuted( true );
	return true;
}

bool MidiActionManager::unmute(Action * , Hydrogen* pHydrogen ) {
	pHydrogen->getCoreActionController()->setMasterIsMuted( false );
	return true;
}

bool MidiActionManager::mute_toggle(Action * , Hydrogen* pHydrogen ) {
	pHydrogen->getCoreActionController()->setMasterIsMuted( !pHydrogen->getSong()->getIsMuted() );
	return true;
}

bool MidiActionManager::strip_mute_toggle(Action * pAction, Hydrogen* pHydrogen ) {
	
	bool ok;
	bool bSucccess = true;
	
	int nLine = pAction->getParameter1().toInt(&ok,10);

	std::shared_ptr<Song> pSong = pHydrogen->getSong();
	InstrumentList *pInstrList = pSong->getInstrumentList();
	
	if ( pInstrList->is_valid_index( nLine ) ) {
		auto pInstr = pInstrList->get( nLine );
		
		if ( pInstr ) {
			pHydrogen->getCoreActionController()->setStripIsMuted( nLine, !pInstr->is_muted() );
		} else {
			bSucccess = false;
		}
	} else {
		bSucccess = false;
	}

	return bSucccess;
}

bool MidiActionManager::strip_solo_toggle(Action * pAction, Hydrogen* pHydrogen ) {
	
	bool ok;
	bool bSucccess = true;
	
	int nLine = pAction->getParameter1().toInt(&ok,10);

	std::shared_ptr<Song> pSong = pHydrogen->getSong();
	InstrumentList *pInstrList = pSong->getInstrumentList();

	if ( pInstrList->is_valid_index( nLine ) ) {
		auto pInstr = pInstrList->get( nLine );
		
		if ( pInstr ) {
			pHydrogen->getCoreActionController()->setStripIsSoloed( nLine, !pInstr->is_soloed() );
		} else {
			bSucccess = false;
		}
	} else {
		bSucccess = false;
	}
	
	return bSucccess;
}

bool MidiActionManager::beatcounter(Action * , Hydrogen* pHydrogen ) {
	pHydrogen->handleBeatCounter();
	return true;
}

bool MidiActionManager::tap_tempo(Action * , Hydrogen* pHydrogen ) {
	pHydrogen->onTapTempoAccelEvent();
	return true;
}

bool MidiActionManager::select_next_pattern(Action * pAction, Hydrogen* pHydrogen ) {
	bool ok;
	int row = pAction->getParameter1().toInt(&ok,10);
	if( row > pHydrogen->getSong()->getPatternList()->size() - 1 ||
		row < 0 ) {
		return false;
	}
	if(Preferences::get_instance()->patternModePlaysSelected()) {
		pHydrogen->setSelectedPatternNumber( row );
	}
	else {
		pHydrogen->sequencer_setNextPattern( row );
	}
	return true;
}

bool MidiActionManager::select_only_next_pattern(Action * pAction, Hydrogen* pHydrogen ) {
	bool ok;
	int row = pAction->getParameter1().toInt(&ok,10);
	if( row > pHydrogen->getSong()->getPatternList()->size() -1 ||
		row < 0 ) {
		return false;
	}
	if(Preferences::get_instance()->patternModePlaysSelected())
	{
		return true;
	}
	
	pHydrogen->sequencer_setOnlyNextPattern( row );
	return true; 
}

bool MidiActionManager::select_next_pattern_relative(Action * pAction, Hydrogen* pHydrogen ) {
	bool ok;
	if(!Preferences::get_instance()->patternModePlaysSelected()) {
		return true;
	}
	int row = pHydrogen->getSelectedPatternNumber() + pAction->getParameter1().toInt(&ok,10);
	if( row > pHydrogen->getSong()->getPatternList()->size() - 1 ||
		row < 0 ) {
		return false;
	}
	
	pHydrogen->setSelectedPatternNumber( row );
	return true;
}

bool MidiActionManager::select_next_pattern_cc_absolute(Action * pAction, Hydrogen* pHydrogen ) {
	bool ok;
	int row = pAction->getValue().toInt(&ok,10);
	
	if( row > pHydrogen->getSong()->getPatternList()->size() - 1 ||
		row < 0 ) {
		return false;
	}
	
	if(Preferences::get_instance()->patternModePlaysSelected()) {
		pHydrogen->setSelectedPatternNumber( row );
	}
	else {
		return true;// only usefully in normal pattern mode
	}
	
	return true;
}

bool MidiActionManager::select_and_play_pattern(Action * pAction, Hydrogen* pHydrogen ) {
	if ( ! select_next_pattern( pAction, pHydrogen ) ) {
		return false;
	}

	if ( pHydrogen->getAudioEngine()->getState() == AudioEngine::State::Ready ) {
		pHydrogen->sequencer_play();
	}

	return true;
}

bool MidiActionManager::select_instrument(Action * pAction, Hydrogen* pHydrogen ) {
	bool ok;
	int  nInstrumentNumber = pAction->getValue().toInt(&ok,10) ;
	

	if ( pHydrogen->getSong()->getInstrumentList()->size() < nInstrumentNumber ) {
		nInstrumentNumber = pHydrogen->getSong()->getInstrumentList()->size() -1;
	} else if ( nInstrumentNumber < 0 ) {
		nInstrumentNumber = 0;
	}
	
	pHydrogen->setSelectedInstrumentNumber( nInstrumentNumber );
	return true;
}

bool MidiActionManager::effect_level_absolute(Action * pAction, Hydrogen* pHydrogen) {
	bool ok;
	bool bSuccess = true;
	int nLine = pAction->getParameter1().toInt(&ok,10);
	int fx_param = pAction->getValue().toInt(&ok,10);
	int fx_id = pAction->getParameter2().toInt(&ok,10);

	std::shared_ptr<Song> pSong = pHydrogen->getSong();
	InstrumentList *pInstrList = pSong->getInstrumentList();
	
	if ( pInstrList->is_valid_index( nLine) )
	{
		auto pInstr = pInstrList->get( nLine );
		
		if ( pInstr ) {
			if( fx_param != 0 ) {
				pInstr->set_fx_level(  ( (float) (fx_param / 127.0 ) ), fx_id );
			} else {
				pInstr->set_fx_level( 0 , fx_id );
			}
			
			pHydrogen->setSelectedInstrumentNumber( nLine );			
		} else {
			bSuccess = false;
		}
	
	}

	return bSuccess;
}

bool MidiActionManager::effect_level_relative(Action * , Hydrogen* ) {
	//empty ?
	return true;
}

//sets the volume of a master output to a given level (percentage)
bool MidiActionManager::master_volume_absolute(Action * pAction, Hydrogen* pHydrogen ) {

	bool ok;
	int vol_param = pAction->getValue().toInt(&ok,10);

	std::shared_ptr<Song> song = pHydrogen->getSong();

	if( vol_param != 0 ){
		song->setVolume( 1.5* ( (float) (vol_param / 127.0 ) ));
	} else {
		song->setVolume( 0 );
	}

	return true;
}

//increments/decrements the volume of the whole song
bool MidiActionManager::master_volume_relative(Action * pAction, Hydrogen* pHydrogen ) {

	bool ok;
	int vol_param = pAction->getValue().toInt(&ok,10);

	std::shared_ptr<Song> song = pHydrogen->getSong();

	if( vol_param != 0 ) {
		if ( vol_param == 1 && song->getVolume() < 1.5 ) {
			song->setVolume( song->getVolume() + 0.05 );
		} else {
			if( song->getVolume() >= 0.0 ) {
				song->setVolume( song->getVolume() - 0.05 );
			}
		}
	} else {
		song->setVolume( 0 );
	}

	return true;
}

//sets the volume of a mixer strip to a given level (percentage)
bool MidiActionManager::strip_volume_absolute(Action * pAction, Hydrogen* pHydrogen ) {

	bool ok;
	int nLine = pAction->getParameter1().toInt(&ok,10);
	int vol_param = pAction->getValue().toInt(&ok,10);

	std::shared_ptr<Song> pSong = pHydrogen->getSong();
	InstrumentList *pInstrList = pSong->getInstrumentList();
	
	if ( pInstrList->is_valid_index( nLine) )
	{
		auto pInstr = pInstrList->get( nLine );
	
		if ( pInstr == nullptr) {
			return false;
		}
	
		if( vol_param != 0 ) {
			pInstr->set_volume( 1.5* ( (float) (vol_param / 127.0 ) ));
		} else {
			pInstr->set_volume( 0 );
		}
	
		pHydrogen->setSelectedInstrumentNumber(nLine);
	}

	return true;
}

//increments/decrements the volume of one mixer strip
bool MidiActionManager::strip_volume_relative(Action * pAction, Hydrogen* pHydrogen ) {

	bool ok;
	int nLine = pAction->getParameter1().toInt(&ok,10);
	int vol_param = pAction->getValue().toInt(&ok,10);

	std::shared_ptr<Song> pSong = pHydrogen->getSong();
	InstrumentList *pInstrList = pSong->getInstrumentList();

	if ( pInstrList->is_valid_index( nLine) )
	{
		auto pInstr = pInstrList->get( nLine );
	
		if ( pInstr == nullptr) {
			return false;
		}
	
		if( vol_param != 0 ) {
			if ( vol_param == 1 && pInstr->get_volume() < 1.5 ) {
				pInstr->set_volume( pInstr->get_volume() + 0.1 );
			} else {
				if( pInstr->get_volume() >= 0.0 ){
					pInstr->set_volume( pInstr->get_volume() - 0.1 );
				}
			}
		} else {
			pInstr->set_volume( 0 );
		}
	
		pHydrogen->setSelectedInstrumentNumber(nLine);
	}

	return true;
}

// sets the absolute panning of a given mixer channel
bool MidiActionManager::pan_absolute(Action * pAction, Hydrogen* pHydrogen ) {

	bool ok;
	int nLine = pAction->getParameter1().toInt(&ok,10);
	int pan_param = pAction->getValue().toInt(&ok,10);

	std::shared_ptr<Song> pSong = pHydrogen->getSong();
	InstrumentList *pInstrList = pSong->getInstrumentList();
	
	if( pInstrList->is_valid_index( nLine ) ) {
		pHydrogen->setSelectedInstrumentNumber( nLine );
	
		auto pInstr = pInstrList->get( nLine );
	
		if( pInstr == nullptr ) {
			return false;
		}

		pInstr->setPanWithRangeFrom0To1( (float) pan_param / 127.f );
	
		pHydrogen->setSelectedInstrumentNumber(nLine);
	}

	return true;
}

// changes the panning of a given mixer channel
// this is useful if the panning is set by a rotary control knob
bool MidiActionManager::pan_relative(Action * pAction, Hydrogen* pHydrogen ) {

	bool ok;
	int nLine = pAction->getParameter1().toInt(&ok,10);
	int pan_param = pAction->getValue().toInt(&ok,10);

	std::shared_ptr<Song> pSong = pHydrogen->getSong();
	InstrumentList *pInstrList = pSong->getInstrumentList();
	
	if( pInstrList->is_valid_index( nLine ) ) {	
		pHydrogen->setSelectedInstrumentNumber( nLine );

		auto pInstr = pInstrList->get( nLine );

		if( pInstr == nullptr ) {
			return false;
		}
	
		float fPan = pInstr->getPan();

		if( pan_param == 1 && fPan < 1.f ) {
			pInstr->setPan( fPan + 0.1 );
		} else if( pan_param != 1 && fPan > -1.f ) {
			pInstr->setPan( fPan - 0.1 );
		}

		pHydrogen->setSelectedInstrumentNumber(nLine);
	}

	return true;
}

bool MidiActionManager::gain_level_absolute(Action * pAction, Hydrogen* pHydrogen ) {
	bool ok;
	int nLine = pAction->getParameter1().toInt(&ok,10);
	int gain_param = pAction->getValue().toInt(&ok,10);
	int component_id = pAction->getParameter2().toInt(&ok,10);
	int layer_id = pAction->getParameter3().toInt(&ok,10);

	std::shared_ptr<Song> pSong = pHydrogen->getSong();
	InstrumentList *pInstrList = pSong->getInstrumentList();
	
	if( pInstrList->is_valid_index( nLine ) )
	{
		auto pInstr = pInstrList->get( nLine );
		if( pInstr == nullptr ) {
			return false;
		}
	
		auto pComponent =  pInstr->get_component( component_id );
		if( pComponent == nullptr) {
			return false;
		}
	
		auto pLayer = pComponent->get_layer( layer_id );
		if( pLayer == nullptr ) {
			return false;
		}
	
		if( gain_param != 0 ) {
			pLayer->set_gain( 5.0* ( (float) (gain_param / 127.0 ) ) );
		} else {
			pLayer->set_gain( 0 );
		}
	
		pHydrogen->setSelectedInstrumentNumber( nLine );
	
		pHydrogen->refreshInstrumentParameters( nLine );
	}
	
	return true;
}

bool MidiActionManager::pitch_level_absolute(Action * pAction, Hydrogen* pHydrogen ) {
	bool ok;
	int nLine = pAction->getParameter1().toInt(&ok,10);
	int pitch_param = pAction->getValue().toInt(&ok,10);
	int component_id = pAction->getParameter2().toInt(&ok,10);
	int layer_id = pAction->getParameter3().toInt(&ok,10);

	std::shared_ptr<Song> pSong = pHydrogen->getSong();
	InstrumentList *pInstrList = pSong->getInstrumentList();

	if( pInstrList->is_valid_index( nLine ) )
	{
		auto pInstr = pInstrList->get( nLine );
		if( pInstr == nullptr ) {
			return false;
		}
	
		auto pComponent =  pInstr->get_component( component_id );
		if( pComponent == nullptr) {
			return false;
		}
	
		auto pLayer = pComponent->get_layer( layer_id );
		if( pLayer == nullptr ) {
			return false;
		}
	
		if( pitch_param != 0 ){
			pLayer->set_pitch( 49* ( (float) (pitch_param / 127.0 ) ) -24.5 );
		} else {
			pLayer->set_pitch( -24.5 );
		}
	
		pHydrogen->setSelectedInstrumentNumber( nLine );
	
		pHydrogen->refreshInstrumentParameters( nLine );
	}

	return true;
}

bool MidiActionManager::filter_cutoff_level_absolute(Action * pAction, Hydrogen* pHydrogen ) {
	bool ok;
	int nLine = pAction->getParameter1().toInt(&ok,10);
	int filter_cutoff_param = pAction->getValue().toInt(&ok,10);

	std::shared_ptr<Song> pSong = pHydrogen->getSong();
	InstrumentList *pInstrList = pSong->getInstrumentList();

	if( pInstrList->is_valid_index( nLine ) )
	{
		auto pInstr = pInstrList->get( nLine );
		if( pInstr == nullptr ) {
			return false;
		}
	
		pInstr->set_filter_active( true );
		if( filter_cutoff_param != 0 ) {
			pInstr->set_filter_cutoff( ( (float) (filter_cutoff_param / 127.0 ) ) );
		} else {
			pInstr->set_filter_cutoff( 0 );
		}
	
		pHydrogen->setSelectedInstrumentNumber( nLine );
	
		pHydrogen->refreshInstrumentParameters( nLine );
	}
	
	return true;
}


/*
 * increments/decrements the BPM
 * this is useful if the bpm is set by a rotary control knob
 */
bool MidiActionManager::bpm_cc_relative(Action * pAction, Hydrogen* pHydrogen ) {

	pHydrogen->getAudioEngine()->lock( RIGHT_HERE );

	//this Action should be triggered only by CC commands

	bool ok;
	int mult = pAction->getParameter1().toInt(&ok,10);
	//this value should be 1 to decrement and something other then 1 to increment the bpm
	int cc_param = pAction->getValue().toInt(&ok,10);

	if( m_nLastBpmChangeCCParameter == -1) {
		m_nLastBpmChangeCCParameter = cc_param;
	}

	std::shared_ptr<Song> pSong = pHydrogen->getSong();

	if ( m_nLastBpmChangeCCParameter >= cc_param && pSong->getBpm()  < 300) {
		pHydrogen->setBPM( pSong->getBpm() - 1*mult );
	}

	if ( m_nLastBpmChangeCCParameter < cc_param && pSong->getBpm()  > 40 ) {
		pHydrogen->setBPM( pSong->getBpm() + 1*mult );
	}

	m_nLastBpmChangeCCParameter = cc_param;

	pHydrogen->getAudioEngine()->unlock();

	return true;
}

/*
 * increments/decrements the BPM
 * this is useful if the bpm is set by a rotary control knob
 */
bool MidiActionManager::bpm_fine_cc_relative(Action * pAction, Hydrogen* pHydrogen ) {

	pHydrogen->getAudioEngine()->lock( RIGHT_HERE );

	//this Action should be triggered only by CC commands
	bool ok;
	int mult = pAction->getParameter1().toInt(&ok,10);
	//this value should be 1 to decrement and something other then 1 to increment the bpm
	int cc_param = pAction->getValue().toInt(&ok,10);

	if( m_nLastBpmChangeCCParameter == -1) {
		m_nLastBpmChangeCCParameter = cc_param;
	}

	std::shared_ptr<Song> pSong = pHydrogen->getSong();

	if ( m_nLastBpmChangeCCParameter >= cc_param && pSong->getBpm()  < 300) {
		pHydrogen->setBPM( pSong->getBpm() - 0.01*mult );
	}

	if ( m_nLastBpmChangeCCParameter < cc_param && pSong->getBpm()  > 40 ) {
		pHydrogen->setBPM( pSong->getBpm() + 0.01*mult );
	}

	m_nLastBpmChangeCCParameter = cc_param;

	pHydrogen->getAudioEngine()->unlock();

	return true;
}

bool MidiActionManager::bpm_increase(Action * pAction, Hydrogen* pHydrogen ) {
	pHydrogen->getAudioEngine()->lock( RIGHT_HERE );

	bool ok;
	int mult = pAction->getParameter1().toInt(&ok,10);

	std::shared_ptr<Song> pSong = pHydrogen->getSong();
	pHydrogen->setBPM( pSong->getBpm() + 1*mult );

	pHydrogen->getAudioEngine()->unlock();
	
	EventQueue::get_instance()->push_event( EVENT_TEMPO_CHANGED, -1 );

	return true;
}

bool MidiActionManager::bpm_decrease(Action * pAction, Hydrogen* pHydrogen ) {
	pHydrogen->getAudioEngine()->lock( RIGHT_HERE );

	bool ok;
	int mult = pAction->getParameter1().toInt(&ok,10);

	std::shared_ptr<Song> pSong = pHydrogen->getSong();
	pHydrogen->setBPM( pSong->getBpm() - 1*mult );
	
	pHydrogen->getAudioEngine()->unlock();
	
	EventQueue::get_instance()->push_event( EVENT_TEMPO_CHANGED, -1 );

	return true;
}

bool MidiActionManager::next_bar(Action * , Hydrogen* pHydrogen ) {
	pHydrogen->getCoreActionController()->locateToColumn( pHydrogen->getAudioEngine()->getColumn() +1 );
	return true;
}


bool MidiActionManager::previous_bar(Action * , Hydrogen* pHydrogen ) {
	pHydrogen->getCoreActionController()->locateToColumn( pHydrogen->getAudioEngine()->getColumn() -1 );
	return true;
}

bool setSong( int songnumber, Hydrogen * pHydrogen ) {
	int asn = Playlist::get_instance()->getActiveSongNumber();
	if(asn != songnumber && songnumber >= 0 && songnumber <= Playlist::get_instance()->size() - 1 ) {
		Playlist::get_instance()->setNextSongByNumber( songnumber );
	}
	return true;
}

bool MidiActionManager::playlist_song(Action * pAction, Hydrogen* pHydrogen ) {
	bool ok;
	int songnumber = pAction->getParameter1().toInt(&ok,10);
	return setSong( songnumber, pHydrogen );
}

bool MidiActionManager::playlist_next_song(Action * pAction, Hydrogen* pHydrogen ) {
	int songnumber = Playlist::get_instance()->getActiveSongNumber();
	return setSong( ++songnumber, pHydrogen );
}

bool MidiActionManager::playlist_previous_song(Action * pAction, Hydrogen* pHydrogen ) {
	int songnumber = Playlist::get_instance()->getActiveSongNumber();
	return setSong( --songnumber, pHydrogen );
}

bool MidiActionManager::record_ready(Action * pAction, Hydrogen* pHydrogen ) {
	if ( pHydrogen->getAudioEngine()->getState() != AudioEngine::State::Playing ) {
		if (!Preferences::get_instance()->getRecordEvents()) {
			Preferences::get_instance()->setRecordEvents(true);
		}
		else {
			Preferences::get_instance()->setRecordEvents(false);
		}
	}
	return true;
}

bool MidiActionManager::record_strobe_toggle(Action * , Hydrogen* ) {
	if (!Preferences::get_instance()->getRecordEvents()) {
		Preferences::get_instance()->setRecordEvents(true);
	}
	else {
		Preferences::get_instance()->setRecordEvents(false);
	}
	return true;
}

bool MidiActionManager::record_strobe(Action * , Hydrogen* ) {
	if (!Preferences::get_instance()->getRecordEvents()) {
		Preferences::get_instance()->setRecordEvents(true);
	}
	return true;
}

bool MidiActionManager::record_exit(Action * , Hydrogen* ) {
	if (Preferences::get_instance()->getRecordEvents()) {
		Preferences::get_instance()->setRecordEvents(false);
	}
	return true;
}

bool MidiActionManager::toggle_metronome(Action * , Hydrogen* ) {
	Preferences::get_instance()->m_bUseMetronome = !Preferences::get_instance()->m_bUseMetronome;
	return true;
}

bool MidiActionManager::undo_action(Action * , Hydrogen* ) {
	EventQueue::get_instance()->push_event( EVENT_UNDO_REDO, 0);// 0 = undo
	return true;
}

bool MidiActionManager::redo_action(Action * , Hydrogen* ) {
	EventQueue::get_instance()->push_event( EVENT_UNDO_REDO, 1);// 1 = redo
	return true;
}

bool MidiActionManager::handleAction( Action * pAction ) {

	Hydrogen *pHydrogen = Hydrogen::get_instance();
	/*
		return false if action is null
		(for example if no Action exists for an event)
	*/
	if( pAction == nullptr ) {
		return false;
	}

	QString sActionString = pAction->getType();

	std::map<std::string,action_f>::const_iterator foundAction = actionMap.find(sActionString.toStdString());
	if( foundAction != actionMap.end() ) {
		action_f action = foundAction->second;
		return (this->*action)(pAction, pHydrogen);
	}

	return false;
}
