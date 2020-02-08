 /* fre:ac - free audio converter
  * Copyright (C) 2001-2020 Robert Kausch <robert.kausch@freac.org>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the "GNU General Public License".
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#include <dialogs/adddirectory.h>
#include <resources.h>

using namespace smooth::GUI::Dialogs;

freac::AddDirectoryDialog::AddDirectoryDialog()
{
	currentConfig = freac::currentConfig;

	Point	 pos;
	Size	 size;

	mainWnd			= new Window(freac::i18n->TranslateString("Add folder"), currentConfig->wndPos + Point(40, 40), Size(402, 128));
	mainWnd->SetRightToLeft(freac::i18n->IsActiveLanguageRightToLeft());

	mainWnd_titlebar	= new Titlebar(TB_NONE);
	divbar			= new Divider(39, OR_HORZ | OR_BOTTOM);

	pos.x = 175;
	pos.y = 29;
	size.cx = 0;
	size.cy = 0;

	btn_cancel		= new Button(freac::i18n->TranslateString("Cancel"), NIL, pos, size);
	btn_cancel->onAction.Connect(&AddDirectoryDialog::Cancel, this);
	btn_cancel->SetOrientation(OR_LOWERRIGHT);

	pos.x -= 88;

	btn_ok			= new Button(freac::i18n->TranslateString("OK"), NIL, pos, size);
	btn_ok->onAction.Connect(&AddDirectoryDialog::OK, this);
	btn_ok->SetOrientation(OR_LOWERRIGHT);

	pos.x = 7;
	pos.y = 11;
	size.cx = 380;
	size.cy = 37;

	group_directory	= new GroupBox(freac::i18n->TranslateString("Folder"), pos, size);

	pos.x += 9;
	pos.y += 12;

	text_directory	= new Text(freac::i18n->TranslateString("Choose folder:"), pos);

	pos.x += (text_directory->GetUnscaledTextWidth() + 7);
	pos.y -= 3;
	size.cx = 268 - text_directory->GetUnscaledTextWidth();
	size.cy = 0;

	edit_directory	= new EditBox(currentConfig->lastAddedDir, pos, size);

	pos.x += (size.cx + 8);
	pos.y -= 1;
	size.cx = 80;

	btn_browse	= new Button(freac::i18n->TranslateString("Browse"), NIL, pos, size);
	btn_browse->onAction.Connect(&AddDirectoryDialog::Browse, this);

	Add(mainWnd);

	mainWnd->Add(btn_ok);
	mainWnd->Add(btn_cancel);
	mainWnd->Add(btn_browse);
	mainWnd->Add(group_directory);
	mainWnd->Add(text_directory);
	mainWnd->Add(edit_directory);
	mainWnd->Add(mainWnd_titlebar);
	mainWnd->Add(divbar);

	mainWnd->SetFlags(mainWnd->GetFlags() | WF_NOTASKBUTTON);
	mainWnd->SetIcon(ImageLoader::Load("icons/freac.png"));
}

freac::AddDirectoryDialog::~AddDirectoryDialog()
{
	DeleteObject(mainWnd_titlebar);
	DeleteObject(mainWnd);
	DeleteObject(divbar);
	DeleteObject(group_directory);
	DeleteObject(text_directory);
	DeleteObject(edit_directory);
	DeleteObject(btn_browse);
	DeleteObject(btn_ok);
	DeleteObject(btn_cancel);
}

const Error &freac::AddDirectoryDialog::ShowDialog()
{
	mainWnd->Stay();

	return error;
}

String freac::AddDirectoryDialog::GetDirectory()
{
	return edit_directory->GetText();
}

Void freac::AddDirectoryDialog::OK()
{
	currentConfig->lastAddedDir = edit_directory->GetText();

	mainWnd->Close();
}

Void freac::AddDirectoryDialog::Cancel()
{
	error = Error();

	mainWnd->Close();
}

Void freac::AddDirectoryDialog::Browse()
{
	DirSelection	*dialog = new DirSelection();

	dialog->SetParentWindow(mainWnd);
	dialog->SetCaption(String("\n").Append(freac::i18n->TranslateString("Select the folder to add to the joblist:")));
	dialog->SetDirName(edit_directory->GetText());

	if (dialog->ShowDialog() == Success())
	{
		edit_directory->SetText(dialog->GetDirName());
	}

	DeleteObject(dialog);
}
