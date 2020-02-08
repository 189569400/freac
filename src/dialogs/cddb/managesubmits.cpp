 /* fre:ac - free audio converter
  * Copyright (C) 2001-2020 Robert Kausch <robert.kausch@freac.org>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the "GNU General Public License".
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#include <dialogs/cddb/managesubmits.h>

#include <freac.h>
#include <resources.h>

freac::cddbManageSubmitsDlg::cddbManageSubmitsDlg()
{
	currentConfig	= freac::currentConfig;

	Point	 pos;
	Size	 size;

	mainWnd			= new Window(freac::i18n->TranslateString("CDDB data"), currentConfig->wndPos + Point(40, 40), Size(552, 352));
	mainWnd->SetRightToLeft(freac::i18n->IsActiveLanguageRightToLeft());

	mainWnd_titlebar	= new Titlebar(TB_CLOSEBUTTON);
	divbar			= new Divider(39, OR_HORZ | OR_BOTTOM);

	pos.x = 87;
	pos.y = 29;
	size.cx = 0;
	size.cy = 0;

	btn_cancel	= new Button(freac::i18n->TranslateString("Close"), NIL, pos, size);
	btn_cancel->onAction.Connect(&cddbManageSubmitsDlg::Cancel, this);
	btn_cancel->SetOrientation(OR_LOWERRIGHT);

	pos.x = 7;
	pos.y = 10;

	text_entries	= new Text(freac::i18n->TranslateString("CDDB entries to submit:"), pos);

	pos.y += 19;
	size.cx = 261;
	size.cy = 213;

	list_entries	= new ListBox(pos, size);
	list_entries->AddTab(freac::i18n->TranslateString("Category"), 65);
	list_entries->AddTab(freac::i18n->TranslateString("Disc name"), 0);
	list_entries->onSelectEntry.Connect(&cddbManageSubmitsDlg::SelectEntry, this);

	pos.x += 269;
	pos.y -= 19;

	text_preview	= new Text(String(freac::i18n->TranslateString("Preview")).Append(":"), pos);

	pos.y += 19;

	size.cx = 261;
	size.cy = 213;

	edit_preview	= new MultiEdit(NIL, pos, size, 0);
	edit_preview->Deactivate();

	pos.x = 188;
	pos.y = 69;
	size.cx = 0;
	size.cy = 0;

	btn_delete	= new Button(freac::i18n->TranslateString("Remove entry"), NIL, pos, size);
	btn_delete->onAction.Connect(&cddbManageSubmitsDlg::DeleteEntry, this);
	btn_delete->SetOrientation(OR_LOWERLEFT);

	pos.x = 369;

	btn_send	= new Button(freac::i18n->TranslateString("Submit"), NIL, pos, size);
	btn_send->onAction.Connect(&cddbManageSubmitsDlg::SendEntry, this);
	btn_send->SetOrientation(OR_LOWERLEFT);

	pos.x += 88;

	btn_send_all	= new Button(freac::i18n->TranslateString("Submit all"), NIL, pos, size);
	btn_send_all->onAction.Connect(&cddbManageSubmitsDlg::SendAllEntries, this);
	btn_send_all->SetOrientation(OR_LOWERLEFT);

	pos.x = 7;
	pos.y = 26;

	text_status	= new Text(NIL, pos);
	text_status->SetOrientation(OR_LOWERLEFT);

	btn_delete->Deactivate();
	btn_send->Deactivate();

	cddbBatch = new CDDBBatch(currentConfig);

	ReadEntries();

	Add(mainWnd);

	mainWnd->Add(btn_cancel);
	mainWnd->Add(mainWnd_titlebar);
	mainWnd->Add(divbar);
	mainWnd->Add(text_entries);
	mainWnd->Add(list_entries);
	mainWnd->Add(text_preview);
	mainWnd->Add(edit_preview);
	mainWnd->Add(btn_delete);
	mainWnd->Add(btn_send);
	mainWnd->Add(btn_send_all);
	mainWnd->Add(text_status);

	mainWnd->SetFlags(mainWnd->GetFlags() | WF_NOTASKBUTTON);
	mainWnd->SetIcon(ImageLoader::Load("icons/freac.png"));
}

freac::cddbManageSubmitsDlg::~cddbManageSubmitsDlg()
{
	DeleteObject(mainWnd_titlebar);
	DeleteObject(mainWnd);
	DeleteObject(divbar);
	DeleteObject(btn_cancel);

	DeleteObject(text_entries);
	DeleteObject(list_entries);

	DeleteObject(text_preview);
	DeleteObject(edit_preview);

	DeleteObject(btn_delete);
	DeleteObject(btn_send);
	DeleteObject(btn_send_all);

	DeleteObject(text_status);

	delete cddbBatch;
}

const Error &freac::cddbManageSubmitsDlg::ShowDialog()
{
	mainWnd->Stay();

	return error;
}

Void freac::cddbManageSubmitsDlg::Cancel()
{
	mainWnd->Close();
}

Void freac::cddbManageSubmitsDlg::SelectEntry()
{
	const CDDBInfo	&cddbInfo = cddbBatch->GetSubmits().GetNth(list_entries->GetSelectedEntryNumber());
	String		 preview = String(cddbInfo.dArtist).Append(" - ").Append(cddbInfo.dTitle).Append("\n\n");

	for (Int i = 0; i < cddbInfo.trackTitles.Length(); i++)
	{
		preview.Append(i < 9 ? "0" : NIL).Append(String::FromInt(i + 1)).Append(": ").Append(cddbInfo.dArtist == "Various" ? String(cddbInfo.trackArtists.GetNth(i)).Append(" - ") : String()).Append(cddbInfo.trackTitles.GetNth(i)).Append("\n");
	}

	edit_preview->SetText(preview);

	btn_delete->Activate();
	btn_send->Activate();
}

Void freac::cddbManageSubmitsDlg::DeleteEntry()
{
	cddbBatch->DeleteSubmit(cddbBatch->GetSubmits().GetNth(list_entries->GetSelectedEntryNumber()));

	list_entries->Remove(list_entries->GetSelectedEntry());

	edit_preview->SetText(NIL);

	btn_delete->Deactivate();
	btn_send->Deactivate();
}

Void freac::cddbManageSubmitsDlg::ReadEntries()
{
	// Read all entries from the freedb queue

	const Array<CDDBInfo> &entries = cddbBatch->GetSubmits();

	for (Int i = 0; i < entries.Length(); i++)
	{
		const CDDBInfo	&cddbInfo = entries.GetNth(i);

		list_entries->AddEntry(String(cddbInfo.category).Append(ListEntry::tabDelimiter).Append(cddbInfo.dArtist).Append(" - ").Append(cddbInfo.dTitle));
	}
}

Void freac::cddbManageSubmitsDlg::SendEntry()
{
	// Submit selected entry to online CDDB

	text_status->SetText(String(freac::i18n->TranslateString("Submitting CD information")).Append("..."));

	if (cddbBatch->Submit(cddbBatch->GetSubmits().GetNth(list_entries->GetSelectedEntryNumber())))
	{
		list_entries->Remove(list_entries->GetSelectedEntry());

		edit_preview->SetText(NIL);

		btn_delete->Deactivate();
		btn_send->Deactivate();
	}

	text_status->SetText(NIL);
}

Void freac::cddbManageSubmitsDlg::SendAllEntries()
{
	// Submit all entries to online CDDB

	text_status->SetText(String(freac::i18n->TranslateString("Submitting CD information")).Append("..."));

	if (cddbBatch->SubmitAll()) mainWnd->Close();

	text_status->SetText(NIL);
}
