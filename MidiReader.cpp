// MidiReader.cpp : This file contains the 'main' function. Program execution begins and ends there.
// These websites were somewhat helpful
// https://ccrma.stanford.edu/~craig/14q/midifile/MidiFileFormat.html#:~:text=A%20standard%20MIDI%20file%20is,chunk%20defines%20a%20logical%20track.
// https://www.mixagesoftware.com/en/midikit/help/
// http://www.somascape.org/midi/tech/mfile.html#mthd
// https://www.recordingblogs.com/wiki/midi-meta-messages
// http://midi.teragonaudio.com/tech/midispec/id.htm
// http://www.midisolutions.com/EP_Guide.htm
// https://wiki.fourthwoods.com/midi_file_format
// https://usa.yamaha.com/files/download/other_assets/5/328165/psr3000_en2.pdf
// https://en.wikipedia.org/wiki/MIDI_timecode#Time_code_format

#include "Windows.h"
#include <iostream>

bool ProcessMeta();
bool ReadTrack();

void LoadBigEndian(BYTE* str, BYTE* v, int size, int start)
{
	v += size - 1;

	for (int i = start; i < (start + size); i++)
	{
		*v = str[i];
		v--;
	}
}

void WriteVarLen(FILE* file, long value)
{
	long buffer = value & 0x7f;
	
	while ((value >>= 7) > 0)
	{
		buffer <<= 8;
		buffer |= 0x80;
		buffer += (value & 0x7f);
	}
	
	while (true)
	{
		putc(buffer, file);

		if (buffer & 0x80)
		{
			buffer >>= 8;
		}
		else
		{
			break;
		}
	}
}

int ReadVarLen(FILE* file)
{
	int value;
	BYTE c;
	
	if ((value = getc(file)) & 0x80)
	{
		value &= 0x7f;
		
		do
		{
			value = (value << 7) + ((c = getc(file)) & 0x7f);
		} 
		while (c & 0x80);
	}
	
	return (value);
}

class CGeneralMidiNames
{
public:

	struct Table
	{
		BYTE number;

		const char* name;
	};

	static const int length = 128;

	const Table instrumentNameTable[length] =
	{
		0x00, {"Grand Piano   "},
		0x01, {"Bright Piano  "},
		0x02, {"E.Grand       "},
		0x03, {"Honky - tonk  "},
		0x04, {"E.Piano 1     "},
		0x05, {"E.Piano 2     "},
		0x06, {"Harpsichord   "},
		0x07, {"Clavi.        "},
		0x08, {"Celesta       "},
		0x09, {"Glockenspiel  "},
		0x0A, {"Music Box     "},
		0x0B, {"Vibraphone    "},
		0x0C, {"Marimba       "},
		0x0D, {"Xylophone     "},
		0x0E, {"Tubular Bells "},
		0x0F, {"Dulcimer      "},
		0x10, {"DrawbarOrgan  "},
		0x11, {"Perc.Organ    "},
		0x12, {"Rock Organ    "},
		0x13, {"Church Organ  "},
		0x14, {"Reed Organ    "},
		0x15, {"Accordion     "},
		0x16, {"Harmonica     "},
		0x17, {"TangoAccord.  "},
		0x18, {"Nylonstr.Gtr  "},
		0x19, {"Steelstr.Gtr  "},
		0x1A, {"Jazz Guitar   "},
		0x1B, {"Clean Guitar  "},
		0x1C, {"Muted Guitar  "},
		0x1D, {"Overdrive Gtr "},
		0x1E, {"Distortion Gtr"},
		0x1F, {"GtrHarmonics  "},
		0x20, {"Acoustic Bass "},
		0x21, {"Fingered Bass "},
		0x22, {"Picked Bass   "},
		0x23, {"Fretless Bass "},
		0x24, {"Slap Bass 1   "},
		0x25, {"Slap Bass 2   "},
		0x26, {"Synth Bass 1  "},
		0x27, {"Synth Bass 2  "},
		0x28, {"Violin        "},
		0x29, {"Viola         "},
		0x2A, {"Cello         "},
		0x2B, {"Contrabass    "},
		0x2C, {"Tremolo Str.  "},
		0x2D, {"Pizzicato Str "},
		0x2E, {"Harp          "},
		0x2F, {"Timpani       "},
		0x30, {"Strings 1     "},
		0x31, {"Strings 2     "},
		0x32, {"Syn.Strings 1 "},
		0x33, {"Syn.Strings 2 "},
		0x34, {"Choir Aahs    "},
		0x35, {"Voice Oohs    "},
		0x36, {"Synth Voice   "},
		0x37, {"Orchestra Hit "},
		0x38, {"Trumpet       "},
		0x39, {"Trombone      "},
		0x3A, {"Tuba          "},
		0x3B, {"Muted Trumpet "},
		0x3C, {"French Horn   "},
		0x3D, {"Brass Section "},
		0x3E, {"Synth Brass1  "},
		0x3F, {"Synth Brass2  "},
		0x40, {"Soprano Sax   "},
		0x41, {"Alto Sax      "},
		0x42, {"Tenor Sax     "},
		0x43, {"Baritone Sax  "},
		0x44, {"Oboe          "},
		0x45, {"English Horn  "},
		0x46, {"Bassoon       "},
		0x47, {"Clarinet      "},
		0x48, {"Piccolo       "},
		0x49, {"Flute         "},
		0x4A, {"Recorder      "},
		0x4B, {"Pan Flute     "},
		0x4C, {"Blown Bottle  "},
		0x4D, {"Shakuhachi    "},
		0x4E, {"Whistle       "},
		0x4F, {"Ocarina       "},
		0x50, {"Square Lead   "},
		0x51, {"Saw.Lead      "},
		0x52, {"Calliope Lead "},
		0x53, {"Chiff Lead    "},
		0x54, {"Charang Lead  "},
		0x55, {"Voice Lead    "},
		0x56, {"Fifth Lead    "},
		0x57, {"Bass & Lead   "},
		0x58, {"New Age Pad   "},
		0x59, {"Warm Pad      "},
		0x5A, {"Polysynth Pad "},
		0x5B, {"Choir Pad     "},
		0x5C, {"Bowed Pad     "},
		0x5D, {"Metallic Pad  "},
		0x5E, {"Halo Pad      "},
		0x5F, {"Sweep Pad     "},
		0x60, {"Rain          "},
		0x61, {"Soundtrack    "},
		0x62, {"Crystal       "},
		0x63, {"Atmosphere    "},
		0x64, {"Brightness    "},
		0x65, {"Goblins       "},
		0x66, {"Echoes        "},
		0x67, {"Sci - Fi      "},
		0x68, {"Sitar         "},
		0x69, {"Banjo         "},
		0x6A, {"Shamisen      "},
		0x6B, {"Koto          "},
		0x6C, {"Kalimba       "},
		0x6D, {"Bagpipe       "},
		0x6E, {"Fiddle        "},
		0x6F, {"Shanai        "},
		0x70, {"Tinkle Bell   "},
		0x71, {"Agogo         "},
		0x72, {"Steel Drums   "},
		0x73, {"Woodblock     "},
		0x74, {"Taiko Drum    "},
		0x75, {"Melodic Tom   "},
		0x76, {"Synth Drum    "},
		0x77, {"Rev.Cymbal    "},
		0x78, {"GtrFretNoise  "},
		0x79, {"Breath Noise  "},
		0x7A, {"Seashore      "},
		0x7B, {"Bird Tweet    "},
		0x7C, {"Telephone     "},
		0x7D, {"Helicopter    "},
		0x7E, {"Applause      "},
		0x7F, {"Gunshot       "}
	};

	static const int drumLength = 6;

	const Table drumNameTable[drumLength] =
	{
		0x00, {"Standard Kit  "},
		0x08, {"Room Kit      "},
		0x10, {"Power Kit     "},
		0x18, {"Electronic Kit"},
		0x19, {"TR-808 Kit    "},
		0x20, {"Jazz Kit      "}
	};

	static const int drumNoteLength = 61;

	const Table drumNoteNameTable[drumNoteLength] =
	{
		0x18, {"Sequencer Click  "},
		0x19, {"Brush Tap		 "},
		0x1A, {"Brush Swirl	     "},
		0x1B, {"Brush Slap		 "},
		0x1C, {"Brush Tap Swirl  "},
		0x1D, {"Snare Roll		 "},
		0x1E, {"Castanet Hi	     "},
		0x1F, {"Snare Soft		 "},
		0x20, {"Sticks			 "},
		0x21, {"Kick Soft		 "},
		0x22, {"Open Rim Shot	 "},
		0x23, {"Kick Tight		 "},
		0x24, {"Kick			 "},
		0x25, {"Side Stick		 "},
		0x26, {"Snare Snare	     "},
		0x27, {"Hand Clap		 "},
		0x28, {"Snare Tight	     "},
		0x29, {"Floor Tom Low	 "},
		0x2A, {"Hi-Hat Closed    "},
		0x2B, {"Floor Tom Hi	 "},
		0x2C, {"Hi-Hat Pedal	 "},
		0x2D, {"Low,Tom		     "},
		0x2E, {"Hi-Hat Open	     "},
		0x2F, {"Mid Tom Low	     "},
		0x30, {"Mid Tom Hi		 "},
		0x31, {"Crash Cymbal 1	 "},
		0x32, {"High Tom		 "},
		0x33, {"Ride Cymbal 1	 "},
		0x34, {"Chinese Cymbal	 "},
		0x35, {"Ride Cymbal Cup  "},
		0x36, {"Tambourine		 "},
		0x37, {"Splash Cymbal	 "},
		0x38, {"Cowbell		     "},
		0x39, {"Crash Cymbal 2	 "},
		0x3A, {"Vibraslap		 "},
		0x3B, {"Ride Cymbal 2	 "},
		0x3C, {"Bongo Hi		 "},
		0x3D, {"Bongo Low		 "},
		0x3E, {"Conga H Mute	 "},
		0x3F, {"Conga H Open	 "},
		0x40, {"Conga Low		 "},
		0x41, {"Timbale Hi		 "},
		0x42, {"Timbale Low	     "},
		0x43, {"Agogo Hi		 "},
		0x44, {"Agogo Low		 "},
		0x45, {"Cabasa			 "},
		0x46, {"Maracas		     "},
		0x47, {"Samba Whistle Hi "},
		0x48, {"Samba Whistle Low"},
		0x49, {"Guiro Short	     "},
		0x4A, {"Guiro Long		 "},
		0x4B, {"Claves			 "},
		0x4C, {"Wood Block Hi	 "},
		0x4D, {"Wood Block Low	 "},
		0x4E, {"Cuica Mute		 "},
		0x4F, {"Cuica Open		 "},
		0x50, {"Triangle Mute	 "},
		0x51, {"Triangle Open	 "},
		0x52, {"Shaker			 "},
		0x53, {"Jingle Bells	 "},
		0x54, {"Bell Tree		 "}
	};

	static const int noteLength = 88;

	const Table noteNameTable[noteLength] =
	{
		0x15, {"A "},
		0x16, {"A# "},
		0x17, {"B0"},
		0x18, {"C0 "},
		0x19, {"C#0"},
		0x1A, {"D0 "},
		0x1B, {"D#0"},
		0x1C, {"E0 "},
		0x1D, {"F0 "},
		0x1E, {"F#0"},
		0x1F, {"G0 "},
		0x20, {"G#0"},
		0x21, {"A0 "},
		0x22, {"A#0"},
		0x23, {"B0 "},
		0x24, {"C1 "},
		0x25, {"C#1"},
		0x26, {"D1 "},
		0x27, {"D#1"},
		0x28, {"E1 "},
		0x29, {"F1 "},
		0x2A, {"F#1"},
		0x2B, {"G1 "},
		0x2C, {"G#1"},
		0x2D, {"A1 "},
		0x2E, {"A#1"},
		0x2F, {"B1 "},
		0x30, {"C2 "},
		0x31, {"C#2"},
		0x32, {"D2 "},
		0x33, {"D#2"},
		0x34, {"E2 "},
		0x35, {"F2 "},
		0x36, {"F#2"},
		0x37, {"G2 "},
		0x38, {"G#2"},
		0x39, {"A2 "},
		0x3A, {"A#2"},
		0x3B, {"B2 "},
		0x3C, {"C3 "},
		0x3D, {"C#3"},
		0x3E, {"D3 "},
		0x3F, {"D#3"},
		0x40, {"E3 "},
		0x41, {"F3 "},
		0x42, {"F#3"},
		0x43, {"G3 "},
		0x44, {"G#3"},
		0x45, {"A3 "},
		0x46, {"A#3"},
		0x47, {"B3 "},
		0x48, {"C4 "},
		0x49, {"C#4"},
		0x4A, {"D4 "},
		0x4B, {"D#4"},
		0x4C, {"E4 "},
		0x4D, {"F4 "},
		0x4E, {"F#4"},
		0x4F, {"G4 "},
		0x50, {"G#4"},
		0x51, {"A4 "},
		0x52, {"A#4"},
		0x53, {"B4 "},
		0x54, {"C5 "},
		0x55, {"C#5"},
		0x56, {"D5 "},
		0x57, {"D#5"},
		0x58, {"E5 "},
		0x59, {"F5 "},
		0x5A, {"F#5"},
		0x5B, {"G5 "},
		0x5C, {"G#5"},
		0x5D, {"A5 "},
		0x5E, {"A#5"},
		0x5F, {"B5 "},
		0x60, {"C5 "},
		0x61, {"C#6"},
		0x62, {"D6 "},
		0x63, {"D#6"},
		0x64, {"E6 "},
		0x65, {"F6 "},
		0x66, {"F#6"},
		0x67, {"G6 "},
		0x68, {"G#6"},
		0x69, {"A6 "},
		0x6A, {"A#6"},
		0x6B, {"B6 "},
		0x6C, {"C6 "}
	};

	const Table* GetName(BYTE s)
	{
		for (int i = 0; i < length; i++)
		{
			if (instrumentNameTable[i].number == s)
			{
				return &instrumentNameTable[i];
			}
		}

		return nullptr;
	}

	const Table* GetDrumName(BYTE s)
	{
		for (int i = 0; i < drumLength; i++)
		{
			if (drumNameTable[i].number == s)
			{
				return &drumNameTable[i];
			}
		}

		return nullptr;
	}

	const Table* GetDrumNoteName(BYTE s)
	{
		for (int i = 0; i < drumNoteLength; i++)
		{
			if (drumNoteNameTable[i].number == s)
			{
				return &drumNoteNameTable[i];
			}
		}

		return nullptr;
	}

	const Table* GetNoteName(BYTE s)
	{
		for (int i = 0; i < noteLength; i++)
		{
			if (noteNameTable[i].number == s)
			{
				return &noteNameTable[i];
			}
		}

		return nullptr;
	}
};

class CControllerNames
{
public:

	struct Table
	{
		BYTE event;
		BYTE values;

		const char* name;
	};

	static const int length = 110;

	const Table eventNameTable[length] =
	{
		0x00, 0x01, { "BankSelectMSB       " },
		0x01, 0x01, { "ModulationWheel     " },
		0x02, 0x01, { "Breath              " },
		0x04, 0x01, { "Foot                " },
		0x05, 0x01, { "PortamentoTime      " },
		0x06, 0x01, { "DataEntryMSB        " },
		0x07, 0x01, { "Volume              " },
		0x08, 0x01, { "Balance             " },
		0x0A, 0x01, { "Pan                 " },
		0x0B, 0x01, { "Expression          " },
		0x0C, 0x01, { "Effectcontrol1      " },
		0x0D, 0x01, { "Effectcontrol2      " },
		0x10, 0x01, { "GeneralPurpose1     " },
		0x11, 0x01, { "GeneralPurpose2     " },
		0x12, 0x01, { "GeneralPurpose3     " },
		0x13, 0x01, { "GeneralPurpose4     " },
		0x20, 0x01, { "BankSelectLSB       " },
		0x21, 0x01, { "ModulationWheelLSB  " },
		0x22, 0x01, { "BreathLSB           " },
		0x24, 0x01, { "FootLSB             " },
		0x25, 0x01, { "PortamentoTimeLSB   " },
		0x26, 0x01, { "DataEntryLSB        " },
		0x27, 0x01, { "VolumeLSB           " },
		0x28, 0x01, { "BalanceLSB          " },
		0x2A, 0x01, { "PanLSB              " },
		0x2B, 0x01, { "ExpressionLSB       " },
		0x2C, 0x01, { "Effectcontrol1LSB   " },
		0x2D, 0x01, { "Effectcontrol2LSB   " },
		0x30, 0x01, { "GeneralPurpose1LSB  " },
		0x31, 0x01, { "GeneralPurpose2LSB  " },
		0x32, 0x01, { "GeneralPurpose3LSB  " },
		0x33, 0x01, { "GeneralPurpose4LSB  " },
		0x40, 0x01, { "HoldPedal1          " },
		0x41, 0x01, { "Portamento          " },
		0x42, 0x01, { "Sostenuto           " },
		0x43, 0x01, { "SoftPedal           " },
		0x44, 0x01, { "LegatoPedal         " },
		0x45, 0x01, { "HoldPedal2          " },
		0x46, 0x01, { "SoundVariation      " },
		0x47, 0x01, { "SoundTimbre         " },
		0x48, 0x01, { "SoundReleaseTime    " },
		0x49, 0x01, { "SoundAttackTime     " },
		0x4A, 0x01, { "SoundBrightness     " },
		0x4B, 0x01, { "SoundControl6       " },
		0x4C, 0x01, { "SoundControl7       " },
		0x4D, 0x01, { "SoundControl8       " },
		0x4E, 0x01, { "SoundControl9       " },
		0x4F, 0x01, { "SoundControl10      " },
		0x50, 0x01, { "GPControl5          " },
		0x51, 0x01, { "GPControl6          " },
		0x52, 0x01, { "GPControl7          " },
		0x53, 0x01, { "GPControl8          " },
		0x54, 0x01, { "PortamentoControl   " },
		0x5B, 0x01, { "ReverbLevel         " },
		0x5C, 0x01, { "TremoloDepth        " },
		0x5D, 0x01, { "ChorusLevel         " },
		0x5E, 0x01, { "CelesteDepth        " },
		0x5F, 0x01, { "PhaserDepth         " },
		0x60, 0x01, { "DataIncrement       " },
		0x61, 0x01, { "DataDecrement       " },
		0x62, 0x01, { "NRPNParameterLSB    " },
		0x63, 0x01, { "NRPNParameterMSB    " },
		0x64, 0x01, { "RPNParameterLSB     " },
		0x65, 0x01, { "RPNParameterMSB     " },
		0x78, 0x01, { "AllSoundOff         " },
		0x79, 0x01, { "ResetAllControllers " },
		0x7A, 0x01, { "LocalOnOff          " },
		0x7B, 0x01, { "AllNotesOff         " },
		0x7C, 0x01, { "OmniModeOff         " },
		0x7D, 0x01, { "OmniModeOn          " },
		0x7E, 0x01, { "MonoModeOn          " },
		0x7F, 0x01, { "PolyModeOn          " },
		0x80, 0x02, { "NoteOff C0          " },
		0x81, 0x02, { "NoteOff C1          " },
		0x82, 0x02, { "NoteOff C2          " },
		0x83, 0x02, { "NoteOff C3          " },
		0x84, 0x02, { "NoteOff C4          " },
		0x85, 0x02, { "NoteOff C5          " },
		0x86, 0x02, { "NoteOff C6          " },
		0x87, 0x02, { "NoteOff C7          " },
		0x88, 0x02, { "NoteOff C8          " },
		0x89, 0x02, { "NoteOff C9          " },
		0x8A, 0x02, { "NoteOff CA          " },
		0x8B, 0x02, { "NoteOff CB          " },
		0x8C, 0x02, { "NoteOff CC          " },
		0x8D, 0x02, { "NoteOff CD          " },
		0x8E, 0x02, { "NoteOff CE          " },
		0x8F, 0x02, { "NoteOff CF          " },
		0x90, 0x02, { "NoteOn C0           " },
		0x91, 0x02, { "NoteOn C1           " },
		0x92, 0x02, { "NoteOn C2           " },
		0x93, 0x02, { "NoteOn C3           " },
		0x94, 0x02, { "NoteOn C4           " },
		0x95, 0x02, { "NoteOn C5           " },
		0x96, 0x02, { "NoteOn C6           " },
		0x97, 0x02, { "NoteOn C7           " },
		0x98, 0x02, { "NoteOn C8           " },
		0x99, 0x02, { "NoteOn C9           " },
		0x9A, 0x02, { "NoteOn CA           " },
		0x9B, 0x02, { "NoteOn CB           " },
		0x9C, 0x02, { "NoteOn CC           " },
		0x9D, 0x02, { "NoteOn CD           " },
		0x9E, 0x02, { "NoteOn CE           " },
		0x9F, 0x02, { "NoteOn CF           " },
		0xA0, 0x02, { "NoteAftertouch      " },
		0xB0, 0x02, { "ControllerNumber    " },
		0xC0, 0x01, { "ProgramChange       " },
		0xD0, 0x01, { "ControllerAftertouch" },
		0xE0, 0x02, { "PitchBend           " },
		0xFF, 0x00, { "Meta                " }
	};

	const Table* GetName(BYTE s)
	{
		for (int i = 0; i < length; i++)
		{
			if (eventNameTable[i].event == s)
			{
				return &eventNameTable[i];
			}
		}

		return nullptr;
	}

};

class CMetaNames
{
public:

	struct Table
	{
		BYTE event;
		
		const char* name;
	};

	static const int length = 32;

	const Table eventNameTable[length] =
	{
		0x00, { "Sequence number                " },
		0x01, { "Text event                     " },
		0x02, { "Copyright notice               " },
		0x03, { "Sequence or track name         " },
		0x04, { "Instrument name                " },
		0x05, { "Lyric text                     " },
		0x06, { "Marker text                    " },
		0x07, { "Cue point                      " },
		0x20, { "Channel prefix assignment      " },
		0x21, { "Port or cable number           " },
		0x2F, { "End of track                   " },
		0x51, { "Tempo setting                  " },
		0x54, { "SMPTE offset                   " },
		0x58, { "Time signature                 " },
		0x59, { "Key signature                  " },
		0x7F, { "Manufacturer-specific event    " },
		0xF0, { "Start system exclusive event   " },
		0xF1, { "Time code                      " },
		0xF2, { "Song position pointer          " },
		0xF3, { "Song select                    " },
		0xF4, { "(Unused)                       " },
		0xF5, { "(Unused)                       " },
		0xF6, { "Tune request                   " },
		0xF7, { "End system exclusive event(EOX)" },
		0xF8, { "Timing clock                   " },
		0xF9, { "(Unused)                       " },
		0xFA, { "Start playback                 " },
		0xFB, { "Continue playback              " },
		0xFC, { "Stop playback                  " },
		0xFD, { "(Unused)                       " },
		0xFE, { "Active sensing                 " },
		0xFF, { "System reset                   " }
	};

	const Table* GetName(BYTE s)
	{
		for (int i = 0; i < length; i++)
		{
			if (eventNameTable[i].event == s)
			{
				return &eventNameTable[i];
			}
		}

		return nullptr;
	}
};

class CHeader
{
public:

	BYTE type[5];

	int	length;
	
	short format;
	short trackCount;
	short division;
	
	int	ticksPerQuarterNote;

	CHeader() { memset(this, 0x00, sizeof(CHeader)); }

	void PrintData(bool newLine)
	{
		if (newLine)
		{
			printf("\n\nHeader %s", type);
			printf("\nlength %i", length);

			return;
		}

		printf("Header %s", type);
		printf("\nlength %i", length);
		printf("\nformat %i", format);
		printf("\ntrackCount %i", trackCount);
		printf("\ndivision %i", division);
	}
};

class CTrack
{
public:

	int	length;
	int	vTime;
	int	absTime;
	
	BYTE type;
	
	BYTE* data;

	CControllerNames controllerNames;

	void PrintData()
	{
		const CControllerNames::Table* n = controllerNames.GetName(type);

		printf("\n%08i\t%08i\t0x%02x\t%s", absTime, vTime, type, n->name);
	}
};

class CMeta
{
public:

	int	vLength;
	
	BYTE type;
	
	BYTE* data;

	CMetaNames metaNames;

	void PrintData()
	{
		const CMetaNames::Table* n = metaNames.GetName(type);

		if (n == nullptr)
		{
			printf("\t0x%02x\t0x%02x\tNo event name", vLength, type);

			return;
		}
		else
		{
			printf("\t0x%02x\t0x%02x\t%s\t", vLength, type, n->name);
		}

		for (int i = 0; i < vLength; i++)
		{
			printf("0x%02x ", data[i]);
		}
		
		printf("||");

		for (int i = 0; i < vLength; i++)
		{
			printf("%c", (char)data[i]);
		}
	}
};

class CController
{
public:

	int vTime;
	int absTime;

	BYTE type;
	BYTE first;
	BYTE second;

	CControllerNames controllerNames;

	CGeneralMidiNames	instrumentNames;

	void ReadOneValue(FILE* file)
	{
		fread_s(&first, 1, sizeof(BYTE), 1, file);
	}

	void ReadTwoValues(FILE* file)
	{
		fread_s(&first, 1, sizeof(BYTE), 1, file);
		fread_s(&second, 1, sizeof(BYTE), 1, file);
	}

	void PrintData()
	{
		const CControllerNames::Table* n = controllerNames.GetName(type);

		if (n == nullptr)
		{
			printf("\n%08i\t%08i\t0x%02x\tNo Controller Name", absTime, vTime, type);

			return;
		}
		else if (n->values == 0x00)
		{
			printf("\n%08i\t%08i\t0x%02x\t%s", absTime, vTime, type, n->name);
		}
		else if (n->values == 0x01)
		{
			printf("\n%08i\t%08i\t0x%02x\t%s\t0x%02x", absTime, vTime, type, n->name, first);
		}
		else if (n->values == 0x02)
		{
			printf("\n%08i\t%08i\t0x%02x\t%s\t0x%02x\t0x%02x", absTime, vTime, type, n->name, first, second);
		}

		if (type == 0x99 || type == 0x89)
		{
			const CGeneralMidiNames::Table* n;

			n = instrumentNames.GetDrumNoteName(first);

			printf("\t%s", n->name);
		}
		else if (type >= 0x80 && type < 0xA0)
		{
			const CGeneralMidiNames::Table* n;

			n = instrumentNames.GetNoteName(first);

			printf("\t%s", n->name);
		}

		if (type >= 0xC0 && type < 0xD0)
		{
			const CGeneralMidiNames::Table* n;

			if (type == 0xC9)
			{
				n = instrumentNames.GetDrumName(first);
			}
			else
			{
				n = instrumentNames.GetName(first);
			}

			printf("\t%s", n->name);
		}
	}
};

FILE* file;

size_t	result;

CHeader		header;
CTrack		track;
CMeta		meta;
CController	controller;

BYTE buffer[10];

#include <mmeapi.h>
#pragma comment(lib, "Winmm.lib")

char wcstombsBuffer[32];
size_t wcstombsNbrReturned;
UINT deviceCount;
bool isRunning;

MMRESULT	mmr;
HMIDI		midi;
HMIDIIN		midiIn;
HMIDIOUT	midiOut;
HMIDISTRM	midiStrm;
MIDIINCAPS	midiInCaps;
MIDIOUTCAPS	midiOutCaps;
UINT		midiInDeviceId;
UINT		midiOutDeviceId;
MIDIHDR		midiHdr;

BYTE midiMessageBuffer[32];

void CALLBACK midiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	switch (wMsg)
	{
	case MIM_OPEN:
	{
		printf("MIM_OPEN: %i hh0x%02hhx lh0x%02hhx hl0x%02hhx ll0x%02hhx hh0x%02hhx lh0x%02hhx hl0x%02hhx ll0x%02hhx\n", wMsg,
			HIBYTE(HIWORD(dwParam1)), LOBYTE(HIWORD(dwParam1)), HIBYTE(LOWORD(dwParam1)), LOBYTE(LOWORD(dwParam1)),
			HIBYTE(HIWORD(dwParam2)), LOBYTE(HIWORD(dwParam2)), HIBYTE(LOWORD(dwParam2)), LOBYTE(LOWORD(dwParam2)));

		break;
	}
	case MIM_CLOSE:
	{
		printf("MIM_CLOSE: %i hh0x%02hhx lh0x%02hhx hl0x%02hhx ll0x%02hhx hh0x%02hhx lh0x%02hhx hl0x%02hhx ll0x%02hhx\n", wMsg,
			HIBYTE(HIWORD(dwParam1)), LOBYTE(HIWORD(dwParam1)), HIBYTE(LOWORD(dwParam1)), LOBYTE(LOWORD(dwParam1)),
			HIBYTE(HIWORD(dwParam2)), LOBYTE(HIWORD(dwParam2)), HIBYTE(LOWORD(dwParam2)), LOBYTE(LOWORD(dwParam2)));

		break;
	}
	case MIM_DATA:
	{
		if (LOBYTE(HIWORD(dwParam1)) > 0)
		{
			
			printf("MIM_DATA: %i hh0x%02hhx lh0x%02hhx hl0x%02hhx ll0x%02hhx hh0x%02hhx lh0x%02hhx hl0x%02hhx ll0x%02hhx\n", wMsg,
				HIBYTE(HIWORD(dwParam1)), LOBYTE(HIWORD(dwParam1)), HIBYTE(LOWORD(dwParam1)), LOBYTE(LOWORD(dwParam1)),
				HIBYTE(HIWORD(dwParam2)), LOBYTE(HIWORD(dwParam2)), HIBYTE(LOWORD(dwParam2)), LOBYTE(LOWORD(dwParam2)));
				
			midiMessageBuffer[0] = LOBYTE(LOWORD(dwParam1));
			midiMessageBuffer[1] = HIBYTE(LOWORD(dwParam1));
			midiMessageBuffer[2] = LOBYTE(HIWORD(dwParam1));

			midiHdr.dwBufferLength = 3;

			mmr = midiOutLongMsg(midiOut, &midiHdr, sizeof(MIDIHDR));
		}

		break;
	}
	case MIM_LONGDATA:
	{
		printf("MIM_LONGDATA: %i hh0x%02hhx lh0x%02hhx hl0x%02hhx ll0x%02hhx hh0x%02hhx lh0x%02hhx hl0x%02hhx ll0x%02hhx\n", wMsg,
			HIBYTE(HIWORD(dwParam1)), LOBYTE(HIWORD(dwParam1)), HIBYTE(LOWORD(dwParam1)), LOBYTE(LOWORD(dwParam1)),
			HIBYTE(HIWORD(dwParam2)), LOBYTE(HIWORD(dwParam2)), HIBYTE(LOWORD(dwParam2)), LOBYTE(LOWORD(dwParam2)));

		break;
	}
	case MIM_ERROR:
	{
		printf("MIM_ERROR: %i hh0x%02hhx lh0x%02hhx hl0x%02hhx ll0x%02hhx hh0x%02hhx lh0x%02hhx hl0x%02hhx ll0x%02hhx\n", wMsg,
			HIBYTE(HIWORD(dwParam1)), LOBYTE(HIWORD(dwParam1)), HIBYTE(LOWORD(dwParam1)), LOBYTE(LOWORD(dwParam1)),
			HIBYTE(HIWORD(dwParam2)), LOBYTE(HIWORD(dwParam2)), HIBYTE(LOWORD(dwParam2)), LOBYTE(LOWORD(dwParam2)));

		break;
	}
	case MIM_LONGERROR:
	{
		printf("MIM_LONGERROR: %i hh0x%02hhx lh0x%02hhx hl0x%02hhx ll0x%02hhx hh0x%02hhx lh0x%02hhx hl0x%02hhx ll0x%02hhx\n", wMsg,
			HIBYTE(HIWORD(dwParam1)), LOBYTE(HIWORD(dwParam1)), HIBYTE(LOWORD(dwParam1)), LOBYTE(LOWORD(dwParam1)),
			HIBYTE(HIWORD(dwParam2)), LOBYTE(HIWORD(dwParam2)), HIBYTE(LOWORD(dwParam2)), LOBYTE(LOWORD(dwParam2)));

		break;
	}
	}
}

void CALLBACK midiOutProc(HMIDIOUT hMidiOut, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	switch (wMsg)
	{
	case MOM_OPEN:
	{
		printf("MOM_OPEN: %i hh0x%02hhx lh0x%02hhx hl0x%02hhx ll0x%02hhx hh0x%02hhx lh0x%02hhx hl0x%02hhx ll0x%02hhx\n", wMsg,
			HIBYTE(HIWORD(dwParam1)), LOBYTE(HIWORD(dwParam1)), HIBYTE(LOWORD(dwParam1)), LOBYTE(LOWORD(dwParam1)),
			HIBYTE(HIWORD(dwParam2)), LOBYTE(HIWORD(dwParam2)), HIBYTE(LOWORD(dwParam2)), LOBYTE(LOWORD(dwParam2)));

		break;
	}
	case MOM_CLOSE:
	{
		printf("MOM_CLOSE: %i hh0x%02hhx lh0x%02hhx hl0x%02hhx ll0x%02hhx hh0x%02hhx lh0x%02hhx hl0x%02hhx ll0x%02hhx\n", wMsg,
			HIBYTE(HIWORD(dwParam1)), LOBYTE(HIWORD(dwParam1)), HIBYTE(LOWORD(dwParam1)), LOBYTE(LOWORD(dwParam1)),
			HIBYTE(HIWORD(dwParam2)), LOBYTE(HIWORD(dwParam2)), HIBYTE(LOWORD(dwParam2)), LOBYTE(LOWORD(dwParam2)));

		break;
	}
	case MOM_DONE:
	{
		printf("MOM_DONE: %i hh0x%02hhx lh0x%02hhx hl0x%02hhx ll0x%02hhx hh0x%02hhx lh0x%02hhx hl0x%02hhx ll0x%02hhx\n", wMsg,
			HIBYTE(HIWORD(dwParam1)), LOBYTE(HIWORD(dwParam1)), HIBYTE(LOWORD(dwParam1)), LOBYTE(LOWORD(dwParam1)),
			HIBYTE(HIWORD(dwParam2)), LOBYTE(HIWORD(dwParam2)), HIBYTE(LOWORD(dwParam2)), LOBYTE(LOWORD(dwParam2)));

		break;
	}
	}

	return;
}

void CALLBACK midiStrmProc(HMIDIOUT hMidiOut, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	printf("midiStrmProc: %i\n", wMsg);

	return;
}

int main(int argc, char* argv[])
{
	/*
	deviceCount = midiInGetNumDevs();

	for (UINT i = 0; i < deviceCount; i++)
	{
		mmr = midiInGetDevCaps(i, &midiInCaps, sizeof(MIDIINCAPS));
		
		memset(wcstombsBuffer, 0x00, 32);
		wcstombs_s(&wcstombsNbrReturned, wcstombsBuffer, midiInCaps.szPname, 32);
		printf("midiInCaps:%i %s version: %i %i\n", i, wcstombsBuffer, HIBYTE(midiInCaps.vDriverVersion), LOBYTE(midiInCaps.vDriverVersion));
		
		midiInDeviceId = i;
	}

	// last driver seems bogus
	deviceCount = midiOutGetNumDevs() - 1;

	for (UINT i = 0; i < deviceCount; i++)
	{
		mmr = midiOutGetDevCaps(0, &midiOutCaps, sizeof(MIDIOUTCAPS)); MIDICAPS_CACHE; MOD_MIDIPORT;
		
		memset(wcstombsBuffer, 0x00, 32);
		wcstombs_s(&wcstombsNbrReturned, wcstombsBuffer, midiOutCaps.szPname, 32);
		printf("midiOutCaps:%i %s version: %i %i\n", i, wcstombsBuffer, HIBYTE(midiOutCaps.vDriverVersion), LOBYTE(midiOutCaps.vDriverVersion));
		
		midiOutDeviceId = i;
	}

	mmr = midiInOpen(&midiIn, midiInDeviceId, (DWORD_PTR)midiInProc, 0, CALLBACK_FUNCTION);

	mmr = midiOutOpen(&midiOut, midiOutDeviceId, (DWORD_PTR)midiOutProc, 0, CALLBACK_FUNCTION);

	
	memset(&midiHdr, 0x00, sizeof(MIDIHDR));

	midiHdr.lpData = (CHAR*)midiMessageBuffer;

	mmr = midiOutPrepareHeader(midiOut, &midiHdr, sizeof(MIDIHDR));


	//mmr = midiStreamOpen(&midiStrm, &midiInDeviceId, 1, (DWORD_PTR)midiStrmProc, 0, CALLBACK_FUNCTION);

	//mmr = midiConnect(midi, midiOut, 0);

	isRunning = true;

	while (isRunning)
	{
		if (GetAsyncKeyState(VK_SPACE))
		{
			Sleep(250);

			isRunning = false;
		}

		//P
		if (GetAsyncKeyState(0x50))
		{
			Sleep(250);

			printf("Program Change\n");

			midiMessageBuffer[0] = 0xC0;
			midiMessageBuffer[1] = 0x02;

			midiHdr.dwBufferLength = 2;

			mmr = midiOutLongMsg(midiOut, &midiHdr, sizeof(MIDIHDR));
		}
		//S
		if (GetAsyncKeyState(0x53))
		{
			Sleep(250);

			mmr = midiInStart(midiIn);
			printf("mmr = midiInStart(midiIn);\n");
		}
		//D
		if (GetAsyncKeyState(0x44))
		{
			Sleep(250);

			mmr = midiInStop(midiIn);
			printf("mmr = midiInStop(midiIn);\n");
		}
	}

	midiInClose(midiIn);

	midiOutClose(midiOut);

	return 1;
	*/
	
	errno_t result = fopen_s(&file, argv[1], "rb");

	if (result != 0)
	{
		return -1;
	}

	// handle MThd, MTrk

	// "MThd" 4 bytes the literal string MThd, or in hexadecimal notation : 0x4d546864. These four characters at the start of the MIDI file indicate that this is a MIDI file.
	// <header_length> 4 bytes length of the header chunk(always 6 bytes long& emdash; the size of the next three fields).
	// <format> 2 bytes
	// 0 = single track file format
	// 1 = multiple track file format
	// 2 = multiple song file format
	// <n> 2 bytes number of tracks that follow
	// <division> 2 bytes unit of time for delta timing.
	//		If the value is positive, then it represents the units per beat.
	//		For example, +96 would mean 96 ticks per beat.
	//		If the value is negative, delta times are in SMPTE compatible units.

	// "MTrk" 4 bytes the literal string MTrk.This marks the beginning of a track.
	// <length> 4 bytes the number of bytes in the track chunk following this number.
	// <track_event> a sequenced track event.

	while (fread_s(header.type, 4, sizeof(BYTE), 4, file))
	{
		if (header.type[0] == 'M' && header.type[1] == 'T' && header.type[2] == 'h' && header.type[3] == 'd')
		{
			fread_s(buffer, 10, sizeof(BYTE), 10, file);

			LoadBigEndian(buffer, (BYTE*)&header.length, 4, 0);
			LoadBigEndian(buffer, (BYTE*)&header.format, 2, 4);
			LoadBigEndian(buffer, (BYTE*)&header.trackCount, 2, 6);
			LoadBigEndian(buffer, (BYTE*)&header.division, 2, 8);

			byte res = header.division >> 15;
			// SMPTE and MIDI time code
			if ((res) == 1)
			{
				BYTE format = 0x00;
				BYTE ticksPerFrame = 0x00;

				BYTE* a = (BYTE*)&header.division;
				BYTE* f = &format;
				BYTE* t = &ticksPerFrame;

				// extract left part of division
				*f = a[1];

				// extract right part of division
				*t = a[0];

				// 2's compliment left hand of division
				format = 0xFF - format + 1;

				int framesPerSecond = 0;

				//-24, -25, -29, or -30
				switch (format)
				{
					case 24:
					{
						framesPerSecond = 24;

						break;
					}
					case 25:
					{
						framesPerSecond = 25;

						break;
					}
					case 29:
					{
						framesPerSecond = 29;

						break;
					}
					case 30:
					{
						framesPerSecond = 30;

						break;
					}
					default:
					{
						printf("\nCheck smpte programming");
						
						break;
					}
				}

				header.ticksPerQuarterNote = framesPerSecond * ticksPerFrame;
			}
			else
			{
				header.ticksPerQuarterNote = header.division;
			}

			header.PrintData(false);
		}
		else if (header.type[0] == 'M' && header.type[1] == 'T' && header.type[2] == 'r' && header.type[3] == 'k')
		{
			// reading size of entire track chunk
			fread_s(buffer, 4, sizeof(BYTE), 4, file);

			LoadBigEndian(buffer, (BYTE*)&header.length, 4, 0);

			header.PrintData(true);

			bool processingTrack = true;

			track.absTime = 0;

			// reading the event type
			while (processingTrack)
			{
				if (!ReadTrack())
				{
					break;
				}

				if (track.type == 0xFF)
				{
					if (!ProcessMeta())
					{
						break;
					}
				}
				else
				{
					controller.vTime = track.vTime;
					controller.absTime = track.absTime;
					controller.type = track.type;

					BYTE res = track.type >> 4;
					res = res << 4;

					// see CControllerNames::Table::eventNameTable for complete list
					switch (res)
					{
						// Two values in event
						case 0x80:
						case 0x90:
						case 0xA0:
						case 0xB0:
						case 0xE0:
						{
							controller.ReadTwoValues(file);

							break;
						}

						// Single value in event
						default:
						{
							controller.ReadOneValue(file);
						
							break;
						}
					}

					controller.PrintData();
				}
			}
		}
	}

	fclose(file);
}

bool ReadTrack()
{
	// reading track's varying length
	track.vTime = ReadVarLen(file);
	track.absTime += track.vTime;

	result = fread_s(&track.type, 1, sizeof(BYTE), 1, file);

	if (feof(file))
	{
		return false;
	}

	return true;
}

bool ProcessMeta()
{
	track.PrintData();

	fread_s(&meta.type, 1, sizeof(BYTE), 1, file);

	meta.vLength = ReadVarLen(file);

	// end-of-track
	if (meta.type == 0x2F)
	{
		meta.PrintData();

		return false;
	}

	meta.data = (BYTE*)malloc(meta.vLength);

	if (meta.data != 0)
	{
		fread_s(meta.data, meta.vLength, sizeof(BYTE), meta.vLength, file);

		meta.PrintData();

		free(meta.data);
	}

	return true;
}