 /* fre:ac - free audio converter
  * Copyright (C) 2001-2020 Robert Kausch <robert.kausch@freac.org>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the "GNU General Public License".
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#include <dialogs/cddb/submit.h>
#include <joblist.h>
#include <dllinterfaces.h>
#include <utilities.h>

#include <freac.h>
#include <resources.h>

#include <cddb/cddblocal.h>
#include <cddb/cddbremote.h>
#include <cddb/cddbbatch.h>
#include <cddb/cddbcache.h>

freac::cddbSubmitDlg::cddbSubmitDlg()
{
	currentConfig	= freac::currentConfig;

	activedrive	= currentConfig->cdrip_activedrive;
	updateJoblist	= currentConfig->update_joblist;

	submitLater	= !currentConfig->enable_remote_cddb;

	dontUpdateInfo	= False;

	Point	 pos;
	Size	 size;

	mainWnd			= new Window(freac::i18n->TranslateString("CDDB data"), currentConfig->wndPos + Point(40, 40), Size(502, 453));
	mainWnd->SetRightToLeft(freac::i18n->IsActiveLanguageRightToLeft());

	mainWnd_titlebar	= new Titlebar(TB_CLOSEBUTTON);
	divbar			= new Divider(45, OR_HORZ | OR_BOTTOM);

	pos.x = 175;
	pos.y = 32;
	size.cx = 0;
	size.cy = 0;

	btn_cancel		= new Button(freac::i18n->TranslateString("Cancel"), NIL, pos, size);
	btn_cancel->onAction.Connect(&cddbSubmitDlg::Cancel, this);
	btn_cancel->SetOrientation(OR_LOWERRIGHT);

	pos.x -= 88;

	btn_submit		= new Button(freac::i18n->TranslateString("Submit"), NIL, pos, size);
	btn_submit->onAction.Connect(&cddbSubmitDlg::Submit, this);
	btn_submit->SetOrientation(OR_LOWERRIGHT);

	if (!currentConfig->enable_remote_cddb) btn_submit->SetText(freac::i18n->TranslateString("Save entry"));

	pos.x = 3;
	pos.y = 39;

	check_updateJoblist	= new CheckBox(freac::i18n->TranslateString("Update joblist with this information"), pos, size, &updateJoblist);
	check_updateJoblist->SetOrientation(OR_LOWERLEFT);

	pos.y -= 19;

	check_submitLater	= new CheckBox(freac::i18n->TranslateString("Submit to online database later"), pos, size, &submitLater);
	check_submitLater->onAction.Connect(&cddbSubmitDlg::ToggleSubmitLater, this);
	check_submitLater->SetOrientation(OR_LOWERLEFT);

	check_updateJoblist->SetWidth(Math::Max(check_updateJoblist->GetUnscaledTextWidth(), check_submitLater->GetUnscaledTextWidth()) + 21);
	check_submitLater->SetWidth(Math::Max(check_updateJoblist->GetUnscaledTextWidth(), check_submitLater->GetUnscaledTextWidth()) + 21);

	pos.x = 7;
	pos.y = 11;
	size.cx = 480;
	size.cy = 43;

	group_drive	= new GroupBox(freac::i18n->TranslateString("Active CD-ROM drive"), pos, size);

	pos.x = 17;
	pos.y = 23;
	size.cx = 260;
	size.cy = 0;

	combo_drive	= new ComboBox(pos, size);

	for (int j = 0; j < currentConfig->cdrip_numdrives; j++)
	{
		combo_drive->AddEntry(currentConfig->cdrip_drives.GetNth(j));
	}

	combo_drive->SelectNthEntry(activedrive);
	combo_drive->onSelectEntry.Connect(&cddbSubmitDlg::ChangeDrive, this);

	pos.x += 267;
	pos.y += 3;

	text_cdstatus	= new Text(String(freac::i18n->TranslateString("Status")).Append(":"), pos);

	pos.x = 7;
	pos.y = 65;

	text_artist	= new Text(String(freac::i18n->TranslateString("Artist")).Append(":"), pos);

	pos.y += 27;

	text_album	= new Text(String(freac::i18n->TranslateString("Album")).Append(":"), pos);

	pos.x += (7 + (Int) Math::Max(text_artist->GetUnscaledTextWidth(), text_album->GetUnscaledTextWidth()));
	pos.y -= 30;
	size.cx = 200 - (Int) Math::Max(text_artist->GetUnscaledTextWidth(), text_album->GetUnscaledTextWidth());
	size.cy = 0;

	edit_artist	= new EditBox(NIL, pos, size, 0);

	list_artist	= new List();
	list_artist->AddEntry(freac::i18n->TranslateString("Various artists"));

	edit_artist->SetDropDownList(list_artist);
	edit_artist->onInput.Connect(&cddbSubmitDlg::SetArtist, this);

	pos.y += 27;

	edit_album	= new EditBox(NIL, pos, size, 0);

	list_genre	= new ListBox(pos, size);
	Utilities::FillGenreList(list_genre);

	pos.x = 221;
	pos.y = 65;

	text_year	= new Text(String(freac::i18n->TranslateString("Year")).Append(":"), pos);

	pos.y += 27;

	text_disccomment= new Text(String(freac::i18n->TranslateString("Comment")).Append(":"), pos);

	pos.x = 228 + Math::Max(text_year->GetUnscaledTextWidth(), text_disccomment->GetUnscaledTextWidth());
	pos.y -= 30;
	size.cx = 31;

	edit_year	= new EditBox(NIL, pos, size, 4);
	edit_year->SetFlags(EDB_NUMERIC);

	pos.x += 38;
	pos.y += 3;

	text_genre	= new Text(String(freac::i18n->TranslateString("Genre")).Append(":"), pos);

	pos.x += (7 + text_genre->GetUnscaledTextWidth());
	pos.y -= 3;
	size.cx = 214 - text_genre->GetUnscaledTextWidth() - Math::Max(text_year->GetUnscaledTextWidth(), text_disccomment->GetUnscaledTextWidth());
	size.cy = 0;

	edit_genre	= new EditBox(NIL, pos, size, 0);
	edit_genre->SetDropDownList(list_genre);

	pos.x = 228 + Math::Max(text_year->GetUnscaledTextWidth(), text_disccomment->GetUnscaledTextWidth());
	pos.y += 27;
	size.cx = 259 - Math::Max(text_year->GetUnscaledTextWidth(), text_disccomment->GetUnscaledTextWidth());
	size.cy = 34;

	edit_disccomment= new MultiEdit(NIL, pos, size, 0);

	pos.x = 7;
	pos.y += 42;
	size.cx = 480;
	size.cy = 140;

	list_tracks	= new ListBox(pos, size);
	list_tracks->SetFlags(LF_ALLOWRESELECT);
	list_tracks->AddTab(freac::i18n->TranslateString("Track"), 50);
	list_tracks->AddTab(freac::i18n->TranslateString("Title"));
	list_tracks->onSelectEntry.Connect(&cddbSubmitDlg::SelectTrack, this);

	pos.x -= 1;
	pos.y += 151;

	text_track	= new Text(String(freac::i18n->TranslateString("Track")).Append(":"), pos);

	pos.x += (7 + text_track->GetUnscaledTextWidth());
	pos.y -= 3;
	size.cx = 25;
	size.cy = 0;

	edit_track	= new EditBox(NIL, pos, size, 3);
	edit_track->SetFlags(EDB_NUMERIC);
	edit_track->Deactivate();

	pos.x += 32;
	pos.y += 3;

	text_trackartist= new Text(String(freac::i18n->TranslateString("Artist")).Append(":"), pos);

	pos.y += 27;

	text_title	= new Text(String(freac::i18n->TranslateString("Title")).Append(":"), pos);

	pos.y += 27;

	text_comment	= new Text(String(freac::i18n->TranslateString("Comment")).Append(":"), pos);

	pos.x += (7 + Math::Max(text_title->GetUnscaledTextWidth(), text_comment->GetUnscaledTextWidth()));
	pos.y -= 57;
	size.cx = 435 - Math::Max(text_title->GetUnscaledTextWidth(), text_comment->GetUnscaledTextWidth()) - text_track->GetUnscaledTextWidth();

	edit_trackartist= new EditBox(NIL, pos, size, 0);
	edit_trackartist->Deactivate();
	edit_trackartist->onInput.Connect(&cddbSubmitDlg::UpdateTrack, this);
	edit_trackartist->onEnter.Connect(&cddbSubmitDlg::FinishArtist, this);

	pos.y += 27;

	edit_title	= new EditBox(NIL, pos, size, 0);
	edit_title->Deactivate();
	edit_title->onInput.Connect(&cddbSubmitDlg::UpdateTrack, this);
	edit_title->onEnter.Connect(&cddbSubmitDlg::FinishTrack, this);

	pos.y += 27;
	size.cy = 34;

	edit_comment	= new MultiEdit(NIL, pos, size, 0);
	edit_comment->Deactivate();
	edit_comment->onInput.Connect(&cddbSubmitDlg::UpdateComment, this);

	pos.x = 7;
	pos.y = 28;

	text_status	= new Text(NIL, pos);
	text_status->SetOrientation(OR_LOWERLEFT);

	SetArtist();

	Add(mainWnd);

	mainWnd->Add(btn_submit);
	mainWnd->Add(btn_cancel);
	mainWnd->Add(check_updateJoblist);
	mainWnd->Add(check_submitLater);
	mainWnd->Add(combo_drive);
	mainWnd->Add(group_drive);
	mainWnd->Add(text_artist);
	mainWnd->Add(edit_artist);
	mainWnd->Add(list_artist);
	mainWnd->Add(text_album);
	mainWnd->Add(edit_album);
	mainWnd->Add(text_year);
	mainWnd->Add(edit_year);
	mainWnd->Add(text_genre);
	mainWnd->Add(edit_genre);
	mainWnd->Add(text_disccomment);
	mainWnd->Add(edit_disccomment);
	mainWnd->Add(text_track);
	mainWnd->Add(edit_track);
	mainWnd->Add(text_trackartist);
	mainWnd->Add(edit_trackartist);
	mainWnd->Add(text_title);
	mainWnd->Add(edit_title);
	mainWnd->Add(text_comment);
	mainWnd->Add(edit_comment);
	mainWnd->Add(list_tracks);
	mainWnd->Add(text_cdstatus);
	mainWnd->Add(text_status);
	mainWnd->Add(mainWnd_titlebar);
	mainWnd->Add(divbar);

	mainWnd->SetFlags(mainWnd->GetFlags() | WF_NOTASKBUTTON);
	mainWnd->SetIcon(ImageLoader::Load("icons/freac.png"));
}

freac::cddbSubmitDlg::~cddbSubmitDlg()
{
	DeleteObject(mainWnd_titlebar);
	DeleteObject(mainWnd);
	DeleteObject(divbar);
	DeleteObject(combo_drive);
	DeleteObject(group_drive);
	DeleteObject(text_artist);
	DeleteObject(edit_artist);
	DeleteObject(list_artist);
	DeleteObject(text_album);
	DeleteObject(edit_album);
	DeleteObject(text_year);
	DeleteObject(edit_year);
	DeleteObject(text_genre);
	DeleteObject(edit_genre);
	DeleteObject(list_genre);
	DeleteObject(text_disccomment);
	DeleteObject(edit_disccomment);
	DeleteObject(list_tracks);
	DeleteObject(text_track);
	DeleteObject(edit_track);
	DeleteObject(text_trackartist);
	DeleteObject(edit_trackartist);
	DeleteObject(text_title);
	DeleteObject(edit_title);
	DeleteObject(text_comment);
	DeleteObject(edit_comment);
	DeleteObject(text_cdstatus);
	DeleteObject(text_status);
	DeleteObject(check_updateJoblist);
	DeleteObject(check_submitLater);
	DeleteObject(btn_submit);
	DeleteObject(btn_cancel);
}

const Error &freac::cddbSubmitDlg::ShowDialog()
{
	ChangeDrive();

	mainWnd->Stay();

	return error;
}

Void freac::cddbSubmitDlg::Submit()
{
	if (!IsDataValid())
	{
		Utilities::ErrorMessage("Please fill all fields and track titles before submitting.");

		return;
	}

	cddbInfo.dArtist	= (edit_artist->GetText() == freac::i18n->TranslateString("Various artists") ? String("Various") : edit_artist->GetText());

	cddbInfo.dTitle		= edit_album->GetText();
	cddbInfo.dYear		= edit_year->GetText().ToInt();
	cddbInfo.dGenre		= edit_genre->GetText();
	cddbInfo.comment	= edit_disccomment->GetText();

	cddbInfo.trackArtists.RemoveAll();
	cddbInfo.trackTitles.RemoveAll();
	cddbInfo.trackComments.RemoveAll();

	for (Int j = 0; j < artists.Length(); j++) cddbInfo.trackArtists.Add(artists.GetNth(j));
	for (Int k = 0; k < titles.Length(); k++) cddbInfo.trackTitles.Add((titles.GetNth(k) == freac::i18n->TranslateString("Data track") ? String("Data track") : titles.GetNth(k)));
	for (Int l = 0; l < comments.Length(); l++) cddbInfo.trackComments.Add(comments.GetNth(l));

	if (cddbInfo.category == NIL) cddbInfo.category = GetCDDBGenre(edit_genre->GetText());

	cddbInfo.revision++;

	check_updateJoblist->Hide();
	check_submitLater->Hide();
	text_status->SetText(String(freac::i18n->TranslateString("Submitting CD information")).Append("..."));

	Int	 revision = cddbInfo.revision;

	if (currentConfig->enable_local_cddb)
	{
		CDDBLocal	 cddb(currentConfig);

		cddb.SetActiveDrive(activedrive);
		cddb.Submit(cddbInfo);
	}

	if (submitLater)
	{
		CDDBBatch	 cddb(currentConfig);

		cddbInfo.revision = revision;

		cddb.SetActiveDrive(activedrive);
		cddb.AddSubmit(cddbInfo);
	}
	else if (currentConfig->enable_remote_cddb)
	{
		CDDBRemote	 cddb(currentConfig);

		cddbInfo.revision = revision;

		cddb.SetActiveDrive(activedrive);

		if (!cddb.Submit(cddbInfo))
		{
			Utilities::ErrorMessage("Some error occurred trying to connect to the freedb server.");

			text_status->SetText(NIL);
			check_updateJoblist->Show();
			check_submitLater->Show();

			cddbInfo.revision--;

			return;
		}
	}

	// Save modified entry to CDDB cache
	currentConfig->cddbCache->AddCacheEntry(cddbInfo);

	text_status->SetText(NIL);

	if (updateJoblist)
	{
		for (Int l = 0; l < currentConfig->appMain->joblist->GetNOfTracks(); l++)
		{
			Track	*trackInfo = currentConfig->appMain->joblist->GetNthTrack(l);

			if (trackInfo->discid != cddbInfo.DiscIDToString()) continue;

			for (Int m = 0; m < titles.Length(); m++)
			{
				if (trackInfo->cdTrack == list_tracks->GetNthEntry(m)->GetText().ToInt())
				{
					if (edit_artist->GetText() == freac::i18n->TranslateString("Various artists") || edit_artist->GetText() == "Various") trackInfo->artist = artists.GetNth(m);
					else															trackInfo->artist = edit_artist->GetText();

					trackInfo->title	= titles.GetNth(m);
					trackInfo->album	= edit_album->GetText();
					trackInfo->year		= edit_year->GetText().ToInt();
					trackInfo->genre	= edit_genre->GetText();
					trackInfo->comment	= comments.GetNth(m);

					String	 jlEntry;

					if (trackInfo->artist == NIL && trackInfo->title == NIL) jlEntry = String(trackInfo->origFilename).Append(ListEntry::tabDelimiter);
					else							 jlEntry = String(trackInfo->artist.Length() > 0 ? trackInfo->artist : freac::i18n->TranslateString("unknown artist")).Append(" - ").Append(trackInfo->title.Length() > 0 ? trackInfo->title : freac::i18n->TranslateString("unknown title")).Append(ListEntry::tabDelimiter);

					jlEntry.Append(trackInfo->track > 0 ? (trackInfo->track < 10 ? String("0").Append(String::FromInt(trackInfo->track)) : String::FromInt(trackInfo->track)) : String()).Append(ListEntry::tabDelimiter).Append(trackInfo->lengthString).Append(ListEntry::tabDelimiter).Append(trackInfo->fileSizeString);

					if (currentConfig->appMain->joblist->GetNthEntry(l)->GetText() != jlEntry) currentConfig->appMain->joblist->GetNthEntry(l)->SetText(jlEntry);
				}
			}
		}
	}

	currentConfig->update_joblist = updateJoblist;

	mainWnd->Close();
}

Void freac::cddbSubmitDlg::Cancel()
{
	mainWnd->Close();
}

Void freac::cddbSubmitDlg::SetArtist()
{
	if (dontUpdateInfo) return;

	if ((edit_artist->GetText() == freac::i18n->TranslateString("Various artists") || edit_artist->GetText() == "Various") && !edit_trackartist->IsActive())
	{
		if (list_tracks->GetSelectedEntry() != NIL)
		{
			edit_trackartist->SetText(artists.Get(list_tracks->GetSelectedEntry()->GetHandle()));
			edit_trackartist->Activate();
		}
	}
	else if ((edit_artist->GetText() != freac::i18n->TranslateString("Various artists") && edit_artist->GetText() != "Various") && edit_trackartist->IsActive())
	{
		edit_trackartist->SetText(NIL);
		edit_trackartist->Deactivate();
	}

	UpdateTrackList();
}

Void freac::cddbSubmitDlg::UpdateTrackList()
{
	for (Int i = 0; i < list_tracks->Length(); i++)
	{
		if (edit_artist->GetText() == freac::i18n->TranslateString("Various artists") || edit_artist->GetText() == "Various") list_tracks->GetNthEntry(i)->SetText(String(i < 9 ? "0" : NIL).Append(String::FromInt(i + 1)).Append(ListEntry::tabDelimiter).Append(artists.GetNth(i).Length() > 0 ? artists.GetNth(i) : freac::i18n->TranslateString("unknown artist")).Append(" - ").Append(titles.GetNth(i).Length() > 0 ? titles.GetNth(i) : freac::i18n->TranslateString("unknown title")));
		else														      list_tracks->GetNthEntry(i)->SetText(String(i < 9 ? "0" : NIL).Append(String::FromInt(i + 1)).Append(ListEntry::tabDelimiter).Append(titles.GetNth(i).Length() > 0 ? titles.GetNth(i) : freac::i18n->TranslateString("unknown title")));
	}
}

Void freac::cddbSubmitDlg::ChangeDrive()
{
	activedrive = combo_drive->GetSelectedEntryNumber();

	ex_CR_SetActiveCDROM(activedrive);
	ex_CR_ReadToc();

	Int	 numTocEntries = ex_CR_GetNumTocEntries();
	Int	 numAudioTracks = numTocEntries;

	for (Int i = 0; i < numTocEntries; i++)
	{
		TOCENTRY entry = ex_CR_GetTocEntry(i);

		if (entry.btFlag & CDROMDATAFLAG || entry.btTrackNumber != i + 1) numAudioTracks--;
	}

	if (numAudioTracks <= 0)
	{
		text_cdstatus->SetText(String(freac::i18n->TranslateString("Status")).Append(": ").Append(freac::i18n->TranslateString("No audio CD in drive!")));

		dontUpdateInfo = True;

		edit_artist->SetText(NIL);
		edit_album->SetText(NIL);
		edit_year->SetText(NIL);
		edit_genre->SetText(NIL);
		edit_disccomment->SetText(NIL);

		list_tracks->RemoveAllEntries();
		titles.RemoveAll();
		artists.RemoveAll();
		comments.RemoveAll();

		edit_track->SetText(NIL);
		edit_trackartist->SetText(NIL);
		edit_title->SetText(NIL);
		edit_comment->SetText(NIL);

		edit_trackartist->Deactivate();
		edit_title->Deactivate();
		edit_comment->Deactivate();

		dontUpdateInfo = False;

		btn_submit->Deactivate();

		return;
	}
	else
	{
		text_cdstatus->SetText(String(freac::i18n->TranslateString("Status")).Append(": ").Append(freac::i18n->TranslateString("Successfully read CD!")));

		btn_submit->Activate();
	}

	cdText.ReadCDText();
	cdPlayerInfo.ReadCDInfo();

	Int	 oDrive = currentConfig->cdrip_activedrive;

	currentConfig->cdrip_activedrive = activedrive;

	CDDBRemote	 cddb(currentConfig);
	Int		 iDiscid = cddb.ComputeDiscID();
	CDDBInfo	 cdInfo;

	if (currentConfig->enable_cddb_cache) cdInfo = currentConfig->cddbCache->GetCacheEntry(iDiscid);

	if (cdInfo == NIL)
	{
		cdInfo = currentConfig->appMain->GetCDDBData();

		if (cdInfo != NIL) currentConfig->cddbCache->AddCacheEntry(cdInfo);
	}

	currentConfig->cdrip_activedrive = oDrive;

	dontUpdateInfo = True;

	list_tracks->RemoveAllEntries();
	titles.RemoveAll();
	artists.RemoveAll();
	comments.RemoveAll();

	if (cdInfo != NIL)
	{
		if (cdInfo.dArtist == "Various") edit_artist->SetText(freac::i18n->TranslateString("Various artists"));
		else				 edit_artist->SetText(cdInfo.dArtist);

		edit_album->SetText(cdInfo.dTitle);
		edit_year->SetText(String::FromInt(cdInfo.dYear) == "0" ? String() : String::FromInt(cdInfo.dYear));
		edit_genre->SetText(cdInfo.dGenre);
		edit_disccomment->SetText(cdInfo.comment);

		list_tracks->RemoveAllEntries();

		edit_track->SetText(NIL);
		edit_trackartist->SetText(NIL);
		edit_title->SetText(NIL);
		edit_comment->SetText(NIL);

		if (cdInfo.dArtist == "Various") edit_trackartist->Activate();
		else				 edit_trackartist->Deactivate();

		edit_title->Deactivate();
		edit_comment->Deactivate();

		for (Int j = 0; j < cdInfo.trackTitles.Length(); j++)
		{
			Int	 handle = list_tracks->AddEntry(NIL)->GetHandle();

			artists.Add(cdInfo.trackArtists.GetNth(j), handle);
			titles.Add(cdInfo.trackTitles.GetNth(j), handle);
			comments.Add(cdInfo.trackComments.GetNth(j), handle);
		}

		cddbInfo = cdInfo;
	}
	else if (cdText.GetCDInfo().GetArtist() != NIL)
	{
		if (cdText.GetCDInfo().GetArtist() == "Various") edit_artist->SetText(freac::i18n->TranslateString("Various artists"));
		else						 edit_artist->SetText(cdText.GetCDInfo().GetArtist());

		edit_album->SetText(cdText.GetCDInfo().GetTitle());
		edit_year->SetText(NIL);
		edit_genre->SetText(NIL);
		edit_disccomment->SetText(NIL);

		list_tracks->RemoveAllEntries();

		edit_track->SetText(NIL);
		edit_trackartist->SetText(NIL);
		edit_title->SetText(NIL);
		edit_comment->SetText(NIL);

		if (cdText.GetCDInfo().GetArtist() == "Various") edit_trackartist->Activate();
		else						 edit_trackartist->Deactivate();

		edit_title->Deactivate();
		edit_comment->Deactivate();

		cddbInfo = NIL;

		cddbInfo.discID = iDiscid;
		cddbInfo.revision = -1;

		cddbInfo.dArtist = cdText.GetCDInfo().GetArtist();
		cddbInfo.dTitle = cdText.GetCDInfo().GetTitle();

		for (Int j = 0; j < numTocEntries; j++)
		{
			TOCENTRY entry = ex_CR_GetTocEntry(j);

			cddbInfo.trackOffsets.Add(entry.dwStartSector + 150, j);
			cddbInfo.trackArtists.Add(NIL, j);
			cddbInfo.trackTitles.Add(cdText.GetCDInfo().GetTrackTitle(entry.btTrackNumber), j);
			cddbInfo.trackComments.Add(NIL, j);

			if (entry.btFlag & CDROMDATAFLAG)
			{
				Int	 handle = list_tracks->AddEntry(NIL)->GetHandle();

				artists.Add(NIL, handle);
				titles.Add(freac::i18n->TranslateString("Data track"), handle);
				comments.Add(NIL, handle);
			}
			else
			{
				Int	 handle = list_tracks->AddEntry(NIL)->GetHandle();

				artists.Add(NIL, handle);
				titles.Add(cdText.GetCDInfo().GetTrackTitle(entry.btTrackNumber), handle);
				comments.Add(NIL, handle);
			}
		}

		cddbInfo.discLength = ex_CR_GetTocEntry(numTocEntries).dwStartSector / 75 + 2;
	}
	else if (cdPlayerInfo.GetCDInfo().GetArtist() != NIL)
	{
		if (cdPlayerInfo.GetCDInfo().GetArtist() == "Various")	edit_artist->SetText(freac::i18n->TranslateString("Various artists"));
		else							edit_artist->SetText(cdPlayerInfo.GetCDInfo().GetArtist());

		edit_album->SetText(cdPlayerInfo.GetCDInfo().GetTitle());
		edit_year->SetText(NIL);
		edit_genre->SetText(NIL);
		edit_disccomment->SetText(NIL);

		list_tracks->RemoveAllEntries();

		edit_track->SetText(NIL);
		edit_trackartist->SetText(NIL);
		edit_title->SetText(NIL);
		edit_comment->SetText(NIL);

		if (cdPlayerInfo.GetCDInfo().GetArtist() == "Various")	edit_trackartist->Activate();
		else							edit_trackartist->Deactivate();

		edit_title->Deactivate();
		edit_comment->Deactivate();

		cddbInfo = NIL;

		cddbInfo.discID = iDiscid;
		cddbInfo.revision = -1;

		cddbInfo.dArtist = cdPlayerInfo.GetCDInfo().GetArtist();
		cddbInfo.dTitle = cdPlayerInfo.GetCDInfo().GetTitle();

		for (Int j = 0; j < numTocEntries; j++)
		{
			TOCENTRY entry = ex_CR_GetTocEntry(j);

			cddbInfo.trackOffsets.Add(entry.dwStartSector + 150, j);
			cddbInfo.trackArtists.Add(NIL, j);
			cddbInfo.trackTitles.Add(cdPlayerInfo.GetCDInfo().GetTrackTitle(entry.btTrackNumber), j);
			cddbInfo.trackComments.Add(NIL, j);

			if (entry.btFlag & CDROMDATAFLAG)
			{
				Int	 handle = list_tracks->AddEntry(NIL)->GetHandle();

				artists.Add(NIL, handle);
				titles.Add(freac::i18n->TranslateString("Data track"), handle);
				comments.Add(NIL, handle);
			}
			else
			{
				Int	 handle = list_tracks->AddEntry(NIL)->GetHandle();

				artists.Add(NIL, handle);
				titles.Add(cdPlayerInfo.GetCDInfo().GetTrackTitle(entry.btTrackNumber), handle);
				comments.Add(NIL, handle);
			}
		}

		cddbInfo.discLength = ex_CR_GetTocEntry(numTocEntries).dwStartSector / 75 + 2;
	}
	else
	{
		edit_artist->SetText(NIL);
		edit_album->SetText(NIL);
		edit_year->SetText(NIL);
		edit_genre->SetText(NIL);
		edit_disccomment->SetText(NIL);

		list_tracks->RemoveAllEntries();

		edit_track->SetText(NIL);
		edit_trackartist->SetText(NIL);
		edit_title->SetText(NIL);
		edit_comment->SetText(NIL);

		edit_trackartist->Deactivate();
		edit_title->Deactivate();
		edit_comment->Deactivate();

		cddbInfo = NIL;

		cddbInfo.discID = iDiscid;
		cddbInfo.revision = -1;

		for (Int j = 0; j < numTocEntries; j++)
		{
			TOCENTRY entry = ex_CR_GetTocEntry(j);

			cddbInfo.trackOffsets.Add(entry.dwStartSector + 150, j);
			cddbInfo.trackArtists.Add(NIL, j);
			cddbInfo.trackTitles.Add(NIL, j);
			cddbInfo.trackComments.Add(NIL, j);

			if (entry.btFlag & CDROMDATAFLAG)
			{
				Int	 handle = list_tracks->AddEntry(NIL)->GetHandle();

				artists.Add(NIL, handle);
				titles.Add(freac::i18n->TranslateString("Data track"), handle);
				comments.Add(NIL, handle);
			}
			else
			{
				Int	 handle = list_tracks->AddEntry(NIL)->GetHandle();

				artists.Add(NIL, handle);
				titles.Add(NIL, handle);
				comments.Add(NIL, handle);
			}
		}

		cddbInfo.discLength = ex_CR_GetTocEntry(numTocEntries).dwStartSector / 75 + 2;
	}

	/* Update information from joblist.
	 */
	for (Int l = 0; l < currentConfig->appMain->joblist->GetNOfTracks(); l++)
	{
		Track	*trackInfo = currentConfig->appMain->joblist->GetNthTrack(l);

		if (trackInfo->discid != CDDB::DiscIDToString(cddb.ComputeDiscID())) continue;

		if (list_tracks->GetNthEntry(trackInfo->cdTrack - 1) != NIL)
		{
			if (trackInfo->artist != NIL && trackInfo->artist != artists.GetNth(trackInfo->cdTrack - 1))
			{
				artists.Set(artists.GetNthIndex(trackInfo->cdTrack - 1), trackInfo->artist);
				cddbInfo.trackArtists.Set(cddbInfo.trackArtists.GetNthIndex(trackInfo->cdTrack - 1), trackInfo->artist);
			}

			if (trackInfo->title != NIL && trackInfo->title != titles.GetNth(trackInfo->cdTrack - 1))
			{
				titles.Set(titles.GetNthIndex(trackInfo->cdTrack - 1), trackInfo->title);
				cddbInfo.trackTitles.Set(cddbInfo.trackTitles.GetNthIndex(trackInfo->cdTrack - 1), trackInfo->title);
			}

			if (trackInfo->album != NIL && trackInfo->album != edit_album->GetText() && trackInfo->cdTrack == 1)
			{
				edit_album->SetText(trackInfo->album);
				cddbInfo.dTitle = trackInfo->album;
			}

			if (trackInfo->genre != NIL && trackInfo->genre != edit_genre->GetText() && trackInfo->cdTrack == 1)
			{
				edit_genre->SetText(trackInfo->genre);
				cddbInfo.dGenre = trackInfo->genre;
			}

			if (trackInfo->year > 0 && trackInfo->year != edit_year->GetText().ToInt() && trackInfo->cdTrack == 1)
			{
				edit_year->SetText(String::FromInt(trackInfo->year));
				cddbInfo.dYear = trackInfo->year;
			}

			if (trackInfo->comment != NIL && trackInfo->comment != comments.GetNth(trackInfo->cdTrack - 1))
			{
				comments.Set(comments.GetNthIndex(trackInfo->cdTrack - 1), trackInfo->comment);
				cddbInfo.trackComments.Set(cddbInfo.trackComments.GetNthIndex(trackInfo->cdTrack - 1), trackInfo->comment);
			}
		}
	}

	/* Set disc artist field.
	 */
	edit_artist->SetText(artists.GetFirst());

	for (Int m = 1; m < list_tracks->Length(); m++)
	{
		if (artists.GetNth(m) != artists.GetNth(m - 1) && titles.GetNth(m) != freac::i18n->TranslateString("Data track") && titles.GetNth(m) != "Data track")
		{
			edit_artist->SetText(freac::i18n->TranslateString("Various artists"));
			edit_trackartist->Activate();

			break;
		}
	}

	UpdateTrackList();

	dontUpdateInfo = False;
}

Void freac::cddbSubmitDlg::SelectTrack()
{
	if (list_tracks->GetSelectedEntry() == NIL) return;

	String	 artist = artists.Get(list_tracks->GetSelectedEntry()->GetHandle());
	String	 title = titles.Get(list_tracks->GetSelectedEntry()->GetHandle());
	String	 comment = comments.Get(list_tracks->GetSelectedEntry()->GetHandle());
	Int	 track = list_tracks->GetSelectedEntry()->GetText().ToInt();

	dontUpdateInfo = True;

	edit_title->SetText(title);
	edit_comment->SetText(comment);
	edit_track->SetText(NIL);

	edit_title->Activate();
	edit_comment->Activate();

	if	(track > 0 && track < 10) edit_track->SetText(String("0").Append(String::FromInt(track)));
	else if (track >= 10)		  edit_track->SetText(String::FromInt(track));

	if (edit_artist->GetText() == freac::i18n->TranslateString("Various artists") || edit_artist->GetText() == "Various")
	{
		edit_trackartist->SetText(artist);
		edit_trackartist->Activate();
		edit_trackartist->MarkAll();
	}
	else
	{
		edit_trackartist->SetText(NIL);
		edit_trackartist->Deactivate();
		edit_title->MarkAll();
	}

	dontUpdateInfo = False;
}

Void freac::cddbSubmitDlg::UpdateTrack()
{
	if (dontUpdateInfo) return;
	if (list_tracks->GetSelectedEntry() == NIL) return;

	Int	 track = edit_track->GetText().ToInt();

	if (edit_artist->GetText() == freac::i18n->TranslateString("Various artists") || edit_artist->GetText() == "Various") list_tracks->GetSelectedEntry()->SetText(String(track < 10 ? "0" : NIL).Append(String::FromInt(track)).Append(ListEntry::tabDelimiter).Append(edit_trackartist->GetText() == NIL ? freac::i18n->TranslateString("unknown artist") : edit_trackartist->GetText()).Append(" - ").Append(edit_title->GetText() == NIL ? freac::i18n->TranslateString("unknown title") : edit_title->GetText()));
	else														      list_tracks->GetSelectedEntry()->SetText(String(track < 10 ? "0" : NIL).Append(String::FromInt(track)).Append(ListEntry::tabDelimiter).Append(edit_title->GetText() == NIL ? freac::i18n->TranslateString("unknown title") : edit_title->GetText()));

	artists.Set(list_tracks->GetSelectedEntry()->GetHandle(), edit_trackartist->GetText());
	titles.Set(list_tracks->GetSelectedEntry()->GetHandle(), edit_title->GetText());
}

Void freac::cddbSubmitDlg::FinishArtist()
{
	edit_title->MarkAll();
}

Void freac::cddbSubmitDlg::FinishTrack()
{
	for (Int i = 0; i < list_tracks->Length() - 1; i++)
	{
		if (list_tracks->GetSelectedEntry() == list_tracks->GetNthEntry(i))
		{
			list_tracks->SelectEntry(list_tracks->GetNthEntry(i + 1));

			SelectTrack();

			break;
		}
	}
}

Void freac::cddbSubmitDlg::UpdateComment()
{
	if (dontUpdateInfo) return;
	if (list_tracks->GetSelectedEntry() == NIL) return;

	comments.Set(list_tracks->GetSelectedEntry()->GetHandle(), edit_comment->GetText());
}

Void freac::cddbSubmitDlg::ToggleSubmitLater()
{
	if (!submitLater && currentConfig->enable_remote_cddb)	btn_submit->SetText(freac::i18n->TranslateString("Submit"));
	else							btn_submit->SetText(freac::i18n->TranslateString("Save entry"));
}

Bool freac::cddbSubmitDlg::IsDataValid()
{
	Bool	 sane = True;

	if (!IsStringValid(edit_artist->GetText()) ||
	    !IsStringValid(edit_album->GetText())) sane = False;

	for (Int i = 0; i < titles.Length(); i++)
	{
		if ((edit_artist->GetText() == freac::i18n->TranslateString("Various artists") ||
		     edit_artist->GetText() == "Various")				       &&
		    !IsStringValid(artists.GetNth(i))) sane = False;

		if (!IsStringValid(titles.GetNth(i))) sane = False;
	}

	return sane;
}

Bool freac::cddbSubmitDlg::IsStringValid(const String &text)
{
	Bool	 valid = False;

	for (Int i = 0; i < text.Length(); i++)
	{
		if (text[i] != ' '  &&
		    text[i] != '\t' &&
		    text[i] != '\n' &&
		    text[i] != '\r')
		{
			valid = True;

			break;
		}
	}

	if ( text.ToLower() == "new artist"				     ||
	     text.ToLower() == "new title"				     ||
	     text == "-"						     ||
	     text == "--"						     ||
	    (text.ToLower().StartsWith("audiotrack") && text.Length() <= 13) ||
	    (text.ToLower().StartsWith("track")	     && text.Length() <= 8)) valid = False;

	return valid;
}

String freac::cddbSubmitDlg::GetCDDBGenre(const String &genre)
{
	String	 cddbGenre = "misc";

	if (genre == "Alt. Rock")		cddbGenre = "rock";
	if (genre == "Anime")			cddbGenre = "soundtrack";
	if (genre == "Big Band")		cddbGenre = "jazz";
	if (genre == "Black Metal")		cddbGenre = "rock";
	if (genre == "Blues")			cddbGenre = "blues";
	if (genre == "BritPop")			cddbGenre = "rock";
	if (genre == "Celtic")			cddbGenre = "folk";
	if (genre == "Chamber Music")		cddbGenre = "classical";
	if (genre == "Christian Rock")		cddbGenre = "rock";
	if (genre == "Classic Rock")		cddbGenre = "rock";
	if (genre == "Classical")		cddbGenre = "classical";
	if (genre == "Country")			cddbGenre = "country";
	if (genre == "Death Metal")		cddbGenre = "rock";
	if (genre == "Ethnic")			cddbGenre = "folk";
	if (genre == "Folk")			cddbGenre = "folk";
	if (genre == "Folk/Rock")		cddbGenre = "folk";
	if (genre == "Folklore")		cddbGenre = "folk";
	if (genre == "Garage Rock")		cddbGenre = "rock";
	if (genre == "Gothic Rock")		cddbGenre = "rock";
	if (genre == "Hard Rock")		cddbGenre = "rock";
	if (genre == "Heavy Metal")		cddbGenre = "rock";
	if (genre == "Indie Rock")		cddbGenre = "rock";
	if (genre == "Instrumental Pop")	cddbGenre = "rock";
	if (genre == "Instrumental Rock")	cddbGenre = "rock";
	if (genre == "Jazz")			cddbGenre = "jazz";
	if (genre == "Jazz+Funk")		cddbGenre = "jazz";
	if (genre == "JPop")			cddbGenre = "rock";
	if (genre == "Krautrock")		cddbGenre = "rock";
	if (genre == "Metal")			cddbGenre = "rock";
	if (genre == "National Folk")		cddbGenre = "folk";
	if (genre == "Native American")		cddbGenre = "folk";
	if (genre == "New Age")			cddbGenre = "newage";
	if (genre == "Pop")			cddbGenre = "rock";
	if (genre == "Pop/Funk")		cddbGenre = "rock";
	if (genre == "Pop-Folk")		cddbGenre = "folk";
	if (genre == "Progressive Rock")	cddbGenre = "rock";
	if (genre == "Psychedelic Rock")	cddbGenre = "rock";
	if (genre == "Punk")			cddbGenre = "rock";
	if (genre == "Punk Rock")		cddbGenre = "rock";
	if (genre == "Reggae")			cddbGenre = "reggae";
	if (genre == "Rock")			cddbGenre = "rock";
	if (genre == "Rock & Roll")		cddbGenre = "rock";
	if (genre == "Slow Rock")		cddbGenre = "rock";
	if (genre == "Soundtrack")		cddbGenre = "soundtrack";
	if (genre == "Southern Rock")		cddbGenre = "rock";
	if (genre == "Symphonic Rock")		cddbGenre = "rock";
	if (genre == "Symphony")		cddbGenre = "classical";
	if (genre == "Thrash-Metal")		cddbGenre = "rock";
	if (genre == "Top 40")			cddbGenre = "rock";
	if (genre == "Tribal")			cddbGenre = "folk";

	return cddbGenre;
}
