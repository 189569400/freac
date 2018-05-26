 /* fre:ac - free audio converter
  * Copyright (C) 2001-2018 Robert Kausch <robert.kausch@freac.org>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the "GNU General Public License".
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#include <main.h>
#include <dllinterfaces.h>

#include <joblist.h>
#include <utilities.h>

#include <smooth/io/drivers/driver_zero.h>

#include <input/filter-in-cdrip.h>

using namespace smooth::Threads;

Void freac::freacGUI::PlayItem(Int entry)
{
	if (entry < 0) return;

	if (encoding)
	{
		Utilities::ErrorMessage("Cannot play a file while encoding!");

		return;
	}

	if (playing && paused && player_entry == entry)
	{
		DLLInterfaces::winamp_out_modules.GetNth(player_plugin)->Pause(0);

		paused = False;

		return;
	}

	if (playing) StopPlayback();

	Font	 font = joblist->GetNthEntry(entry)->GetFont();

	font.SetColor(RGB(255, 0, 0));

	joblist->GetNthEntry(entry)->SetFont(font);

	play_thread = new Thread();
	play_thread->threadMain.Connect(&freacGUI::PlayThread, this);

	playing = True;
	paused = False;
	player_entry = entry;
	stop_playback = False;

	play_thread->SetFlags(THREAD_WAITFLAG_START);
	play_thread->Start();
}

Void freac::freacGUI::PlaySelectedItem()
{
	PlayItem(joblist->GetSelectedEntryNumber());
}

Int freac::freacGUI::PlayThread(Thread *thread)
{
	Track	*trackInfo = joblist->GetNthTrack(player_entry);

	if (trackInfo == NIL)
	{
		playing = false;

		return Error();
	}

	String		 in_filename = trackInfo->origFilename;

	Int		 activeDrive = currentConfig->cdrip_activedrive;

	InStream	*f_in;
	Driver		*driver_in   = new DriverZero();
	InputFilter	*filter_in   = NIL;

	if (trackInfo->isCDTrack)
	{
		currentConfig->cdrip_activedrive = trackInfo->drive;

		f_in		= new InStream(STREAM_DRIVER, driver_in);
		filter_in	= new FilterInCDRip(currentConfig, trackInfo);

		((FilterInCDRip *) filter_in)->SetTrack(trackInfo->cdTrack);

		f_in->AddFilter(filter_in);
	}
	else
	{
		filter_in = Utilities::CreateInputFilter(in_filename, trackInfo);

		f_in = new InStream(STREAM_FILE, in_filename, IS_READ);
		f_in->SetPackageSize(6144);

		if (filter_in != NIL)
		{
			filter_in->SetFileSize(f_in->Size());

			f_in->AddFilter(filter_in);
		}
		else
		{
			delete f_in;
		}
	}

	if (filter_in != NIL)
	{
		Int64		 position = 0;
		UnsignedInt	 samples_size = 1024;
		Int64		 n_loops = (trackInfo->length + samples_size - 1) / samples_size;

		player_plugin = currentConfig->output_plugin;

		Out_Module	*out = DLLInterfaces::winamp_out_modules.GetNth(currentConfig->output_plugin);
		Int		 latency = out->Open(trackInfo->rate, trackInfo->channels, 16, 0, 0);

		if (latency >= 0 && trackInfo->length >= 0)
		{
			Int	 sample = 0;
			short	*sample_buffer = new short [samples_size];

			for (Int loop = 0; loop < n_loops; loop++)
			{
				Int	 step = samples_size;

				if (position + step > trackInfo->length) step = trackInfo->length - position;

				for (Int i = 0; i < step; i++)
				{
					if (trackInfo->order == BYTE_INTEL)	sample = f_in->InputNumberIntel(int16(trackInfo->bits / 8));
					else if (trackInfo->order == BYTE_RAW)	sample = f_in->InputNumberRaw(int16(trackInfo->bits / 8));

					if (sample == -1 && f_in->GetLastError() != IO_ERROR_NODATA) { step = i; break; }

					if (trackInfo->bits == 8)	sample_buffer[i] = (sample - 128) * 256;
					else if (trackInfo->bits == 16)	sample_buffer[i] = sample;
					else if (trackInfo->bits == 24)	sample_buffer[i] = sample / 256;
					else if (trackInfo->bits == 32)	sample_buffer[i] = sample / 65536;
				}

				position += step;

				while (out->CanWrite() < (2 * step))
				{
					if (stop_playback) break;

					Sleep(10);
				}

				if (stop_playback) break;

				out->Write((char *) sample_buffer, 2 * step);
			}

			delete [] sample_buffer;
		}
		else if (latency >= 0 && trackInfo->length == -1)
		{
			Int	 sample = 0;
			short	*sample_buffer = new short [samples_size];

			while (sample != -1)
			{
				Int	 step = samples_size;

				for (Int i = 0; i < step; i++)
				{
					if (trackInfo->order == BYTE_INTEL)	sample = f_in->InputNumberIntel(int16(trackInfo->bits / 8));
					else if (trackInfo->order == BYTE_RAW)	sample = f_in->InputNumberRaw(int16(trackInfo->bits / 8));

					if (sample == -1 && f_in->GetLastError() != IO_ERROR_NODATA) { step = i; break; }

					if (sample != -1)
					{
						if (trackInfo->bits == 8)	sample_buffer[i] = (sample - 128) * 256;
						else if (trackInfo->bits == 16)	sample_buffer[i] = sample;
						else if (trackInfo->bits == 24)	sample_buffer[i] = sample / 256;
						else if (trackInfo->bits == 32)	sample_buffer[i] = sample / 65536;
					}
					else
					{
						i--;
					}
				}

				while (out->CanWrite() < (2 * step))
				{
					if (stop_playback) break;

					Sleep(10);
				}

				if (stop_playback) break;

				out->Write((char *) sample_buffer, 2 * step);
			}

			delete [] sample_buffer;
		}

		if (!stop_playback) while (out->IsPlaying()) Sleep(20);

		out->Close();

		delete f_in;
		delete driver_in;
		delete filter_in;
	}

	currentConfig->cdrip_activedrive = activeDrive;

	Font	 font = joblist->GetNthEntry(player_entry)->GetFont();

	font.SetColor(Setup::ClientTextColor);

	joblist->GetNthEntry(player_entry)->SetFont(font);

	playing = false;

	return Success();
}

Void freac::freacGUI::PausePlayback()
{
	if (!playing) return;

	if (paused) DLLInterfaces::winamp_out_modules.GetNth(player_plugin)->Pause(0);
	else	    DLLInterfaces::winamp_out_modules.GetNth(player_plugin)->Pause(1);

	paused = !paused;
}

Void freac::freacGUI::StopPlayback()
{
	if (!playing) return;

	stop_playback = True;

	while (playing) Sleep(10);

	delete play_thread;

	play_thread = NIL;
}

Void freac::freacGUI::PlayPrevious()
{
	if (!playing) return;

	if (player_entry > 0) PlayItem(player_entry - 1);
}

Void freac::freacGUI::PlayNext()
{
	if (!playing) return;

	if (player_entry < joblist->GetNOfTracks() - 1) PlayItem(player_entry + 1);
}

Void freac::freacGUI::OpenCDTray()
{
	ex_CR_SetActiveCDROM(currentConfig->cdrip_activedrive);
 	ex_CR_EjectCD(True);
}
