#ifndef IHMNAMESPACE_H_
#define IHMNAMESPACE_H_

namespace IHM
{
	enum StateButton
	{
		Default,
		Focused,
		Pressed,
		Released,
		Embedded
	};
	
	enum ButtonType
	{
		normal,
		focused,
		pressed
	};
	
	enum NameButton
	{
		Play,
		Pause,
		Stop,
		Previous,
		Next,
		Eject,
		Mute,
		Unmute,
		Loop,
		LoopTrack,
		LoopDisc,
		Random,
		TrackList
	};
}
#endif /*IHMNAMESPACE_H_*/
