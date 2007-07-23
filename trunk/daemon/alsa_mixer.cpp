/***************************************************************************
 *   Copyright (C) 2007 by Jakub Schmidtke                                 *
 *   sjakub@users.berlios.de                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#include "alsa_mixer.h"

SIDInfo::SIDInfo(snd_mixer_t* handle, snd_mixer_selem_id_t* sid) :
	hasMute(false), hasVolume(false),
	_min(0), _max(0), _isEmulMuted(false), _lastVol(0),
	_lastReal(0), _lastNorm(0),
	_handle(handle), _sid(sid)
{
	if (!_sid) return;

	snd_mixer_elem_t *elem;

	elem = snd_mixer_find_selem(_handle, _sid);

	if (!elem) return;

	if (snd_mixer_selem_has_playback_volume(elem))
	{
		snd_mixer_selem_get_playback_volume_range(elem, &_min, &_max);

		if (_max > _min) hasVolume = true;
		else _min = _max = 0;
	}

	_lastVol = 0;

	if (snd_mixer_selem_has_playback_switch(elem))
	{
		hasMute = true;
	}
}

SIDInfo::~SIDInfo()
{
	if (_sid) free(_sid);
}

long SIDInfo::getRealVolume()
{
	if (!hasVolume || !_sid) return _min;

	snd_mixer_elem_t* elem = snd_mixer_find_selem(_handle, _sid);

	if (!elem) return _min;

	long vleft, vright;

	snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &vleft);

	vright = vleft;

	if (!snd_mixer_selem_is_playback_mono(elem))
		snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_RIGHT, &vright);

	return (vleft + vright)/2;
}

uint SIDInfo::getVolume()
{
	if (!hasVolume || !_sid) return 0;

	long r = getRealVolume();
	uint val = (uint) (r - _min)*100/(_max - _min);

	if (r == _lastReal) return _lastNorm;

	return val;
}

bool SIDInfo::setRealVolume(long val)
{
	if (!hasVolume || !_sid) return false;

	snd_mixer_elem_t* elem = snd_mixer_find_selem(_handle, _sid);

	if (!elem) return false;

	snd_mixer_selem_set_playback_volume_all(elem, val);

	return true;
}

bool SIDInfo::setVolume(uint val)
{
	if (!hasVolume || !_sid) return false;

	if (val > 0) _isEmulMuted = false;

	long rVol = val*(_max - _min)/100;

	_lastReal = rVol;
	_lastNorm = val;

	return setRealVolume(rVol);
}

bool SIDInfo::getMute()
{
	if (!hasMute || !_sid) return false;

	snd_mixer_elem_t* elem = snd_mixer_find_selem(_handle, _sid);

	if (!elem) return false;

	int swr, swl;

	snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_FRONT_LEFT, &swl);
	snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_FRONT_RIGHT, &swr);

	return !(swl || swr);
}

bool SIDInfo::setMute(bool mute)
{
	if (!hasMute || !_sid) return false;

	snd_mixer_elem_t* elem = snd_mixer_find_selem(_handle, _sid);

	if (!elem) return false;

	snd_mixer_selem_set_playback_switch_all(elem, !mute);

	return true;
}

bool SIDInfo::setEmulMute(bool mute)
{
	if (!hasVolume || !_sid) return false;

	snd_mixer_elem_t* elem = snd_mixer_find_selem(_handle, _sid);

	if (!elem) return false;

	if (mute == _isEmulMuted) return true;

	if (mute)
	{
		_isEmulMuted = true;
		_lastVol = getVolume();
		return setVolume(0);
	}
	else
	{
		_isEmulMuted = false;
		bool ret = setVolume(_lastVol);
		_lastVol = 0;
		return ret;
	}
}

LapsusAlsaMixer::LapsusAlsaMixer(): LapsusMixer("alsa"),
	_handle(0), _count(0), _fds(0), _sns(0),
	_curVolume(0), _curMute(false)
{
	for (int i = 0; i < ID_LAST; ++i)
		sids[i] = 0;

	_isValid = init();
}

LapsusAlsaMixer::~LapsusAlsaMixer()
{
	if (_sns)
	{
		for ( int i = 0; i < _count; i++ )
		{
			delete _sns[i];
		}

		delete [] _sns;
	}

	if (_fds) free(_fds);

	for (int i = 0; i < ID_LAST; ++i)
	{
		if (sids[i]) delete sids[i];
	}
}

bool LapsusAlsaMixer::init()
{
	int err;
	snd_ctl_t *ctl_handle;
	snd_ctl_card_info_t *hw_info;

	snd_ctl_card_info_alloca(&hw_info);

	if ((err = snd_ctl_open (&ctl_handle, "default", 0)) < 0)
		return false;
	if ((err = snd_ctl_card_info (ctl_handle, hw_info)) < 0)
		return false;

	snd_ctl_close (ctl_handle);

	if ((err = snd_mixer_open (&_handle, 0)) < 0)
		return false;
	if ((err = snd_mixer_attach (_handle, "default")) < 0)
		return false;
	if ((err = snd_mixer_selem_register (_handle, 0, 0)) < 0)
		return false;

	if ((err = snd_mixer_load (_handle)) < 0)
		return false;

	// printf("Card name: '%s'\n", snd_ctl_card_info_get_name(hw_info));
	// printf("Device name: '%s'\n", snd_ctl_card_info_get_mixername(hw_info));

	snd_mixer_elem_t *elem;
	snd_mixer_selem_id_t *tmp_sid = 0;

	for (elem = snd_mixer_first_elem(_handle); elem; elem = snd_mixer_elem_next(elem))
	{
		if (snd_mixer_selem_is_active(elem))
		{
			if (!tmp_sid) tmp_sid = (snd_mixer_selem_id_t*) malloc(snd_mixer_selem_id_sizeof());

			snd_mixer_selem_get_id(elem, tmp_sid);

			const char *name = snd_mixer_selem_id_get_name( tmp_sid );

			if (!strcasecmp(name, "Headphone"))
			{
				if (sids[ID_HP]) delete sids[ID_HP];
				sids[ID_HP] = new SIDInfo(_handle, tmp_sid);
				tmp_sid = 0;
			}
			else if (!strcasecmp(name, "Front"))
			{
				if (sids[ID_F]) delete sids[ID_F];
				sids[ID_F] = new SIDInfo(_handle, tmp_sid);
				tmp_sid = 0;
			}
			else if (!strcasecmp(name, "Master"))
			{
				if (sids[ID_M]) delete sids[ID_M];
				sids[ID_M] = new SIDInfo(_handle, tmp_sid);
				tmp_sid = 0;
			}
			else if (!strcasecmp(name, "PCM"))
			{
				if (sids[ID_PCM]) delete sids[ID_PCM];
				sids[ID_PCM] = new SIDInfo(_handle, tmp_sid);
				tmp_sid = 0;
			}
		}
	}

	if (tmp_sid) free(tmp_sid);

	bool foundAny = false;

	for (int i = 0; i < ID_LAST; ++i)
	{
		if (sids[i] && (sids[i]->hasMute || sids[i]->hasVolume))
		{
			foundAny = true;
		}
	}

	if (!foundAny) return false;

	if ((_count = snd_mixer_poll_descriptors_count(_handle)) < 0)
		return false;

	_fds = (struct pollfd*) calloc(_count, sizeof(struct pollfd));

	if (!_fds) return false;

	if ((err = snd_mixer_poll_descriptors(_handle, _fds, _count)) < 0)
		return false;

	if (err != _count) return 0;

	_sns = new QSocketNotifier*[_count];

	for ( int i = 0; i < _count; ++i )
	{
		_sns[i] = new QSocketNotifier(_fds[i].fd, QSocketNotifier::Read);
		connect(_sns[i], SIGNAL(activated(int)), this, SLOT(alsaEvent()));
	}

	// To read our current parameters. Signals will be emited, but those signals
	// are not yet connected to anything - this method is called from constructor
	// only.
	getVolume();
	mixerIsMuted();

	return true;
}

void LapsusAlsaMixer::alsaEvent()
{
	if (!_fds || _count < 1) return;

	snd_mixer_handle_events(_handle);

	// Those functions will emit any signals needed
	getVolume();
	mixerIsMuted();
}

bool LapsusAlsaMixer::hardwareDetected()
{
	return _isValid;
}

bool LapsusAlsaMixer::hasVolume()
{
	for (int i = 0; i < ID_LAST; ++i)
	{
		if (sids[i] && sids[i]->hasVolume) return true;
	}

	return false;
}

bool LapsusAlsaMixer::hasMute()
{
	for (int i = 0; i < ID_LAST; ++i)
	{
		if (sids[i] && sids[i]->hasMute) return true;
	}

	return false;
}

int LapsusAlsaMixer::getVolume()
{
	for (int i = 0; i < ID_LAST; ++i)
	{
		if (sids[i] && sids[i]->hasVolume)
		{
			long v = sids[i]->getVolume();

			if (v != _curVolume)
			{
				_curVolume = v;
				volumeChanged((int) v);
			}

			return (int) v;
		}
	}

	return 0;
}

bool LapsusAlsaMixer::setVolume(int val)
{
	bool wasSet = false;

	for (int i = 0; i < ID_LAST; ++i)
	{
		// We try to set the volume only if nothing has been set yet, or
		// current channel is HeadPhones - we set both the first available channel,
		// like Master or Front or PCM, AND HP channel.
		// HP channel should have the greates ID = ID_LAST - 1.
		if ((!wasSet || i == ID_HP)
			&& sids[i] && sids[i]->hasVolume && sids[i]->setVolume(val))
		{
			// Signal should be emited by getVolume in alsaEvent
			wasSet = true;
		}
	}

	if (wasSet && val > _curVolume) setMuted(false);

	return wasSet;
}

bool LapsusAlsaMixer::mixerIsMuted()
{
	for (int i = 0; i < ID_LAST; ++i)
	{
		if (sids[i] && sids[i]->hasMute)
		{
			bool ret = sids[i]->getMute();

			if (ret != _curMute)
			{
			 	_curMute = ret;
			 	muteChanged(ret);
			}

			return ret;
		}
	}

	return false;
}

bool LapsusAlsaMixer::setMuted(bool mState)
{
	bool wasSet = false;
	
	for (int i = 0; i < ID_HP; ++i)
	{
		if (sids[i] && sids[i]->hasMute && sids[i]->setMute(mState))
		{
			// Signal should be emited by getMuted in alsaEvent
			wasSet = true;
		}
	}

	bool wasHPSet = false;

	if (sids[ID_HP] && sids[ID_HP]->hasMute && sids[ID_HP]->setMute(mState))
	{
		// Signal should be emited by getMuted in alsaEvent
		wasHPSet = true;
	}

	if (wasSet && wasHPSet) return true;

	// We haven't found anyt sids with mute capabilities,
	// so let's try to use emulated mute switch which depends on
	// volume control capabilities
	if (!wasSet)
	{
		for (int i = 0; i < ID_HP; ++i)
		{
			if (!wasSet && sids[i] && sids[i]->hasVolume && sids[i]->setEmulMute(mState))
			{
				_curMute = mState;
	
				// Signal should be emited by getVolume in alsaEvent
				return true;
			}
		}
	}

	if (!wasHPSet && sids[ID_HP] && sids[ID_HP]->hasVolume && sids[ID_HP]->setEmulMute(mState))
	{
		_curMute = mState;

		// Signal should be emited by getVolume in alsaEvent
		wasHPSet = true;
	}

	return wasSet || wasHPSet;
}

bool LapsusAlsaMixer::toggleMuted()
{
	return setMuted(!_curMute);
}
