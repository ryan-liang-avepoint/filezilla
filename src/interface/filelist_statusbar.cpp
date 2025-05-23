#include "filezilla.h"
#include "filelist_statusbar.h"
#include "Options.h"

#include "../include/sizeformatting.h"

BEGIN_EVENT_TABLE(CFilelistStatusBar, CFilelistStatusBarBase)
EVT_TIMER(wxID_ANY, CFilelistStatusBar::OnTimer)
END_EVENT_TABLE()

CFilelistStatusBar::CFilelistStatusBar(wxWindow* pParent, COptionsBase & options)
	: CFilelistStatusBarBase(pParent, wxID_ANY, 0)
	, COptionChangeEventHandler(this)
    , options_(options)
{
	m_updateTimer.SetOwner(this);

	m_empty_string = _("Empty directory.");
	m_offline_string = _("Not connected.");

	UpdateText();

#ifdef __WXMSW__
	if (GetLayoutDirection() != wxLayout_RightToLeft) {
		SetDoubleBuffered(true);
	}
#endif

	options_.watch(OPTION_SIZE_FORMAT, this);
	options_.watch(OPTION_SIZE_USETHOUSANDSEP, this);
	options_.watch(OPTION_SIZE_DECIMALPLACES, this);
}

CFilelistStatusBar::~CFilelistStatusBar()
{
	options_.unwatch_all(this);
}

void CFilelistStatusBar::UpdateText()
{
	wxString text;
	if (!m_connected) {
		text = m_offline_string;
	}
	else if (m_count_selected_files || m_count_selected_dirs) {
		if (!m_count_selected_files) {
			text = wxString::Format(wxPLURAL("Selected %d directory.", "Selected %d directories.", m_count_selected_dirs), m_count_selected_dirs);
		}
		else if (!m_count_selected_dirs) {
			const wxString size = SizeFormatter(options_).Format(m_total_selected_size, SizeFormatPurpose::in_line);
			if (m_unknown_selected_size) {
				text = wxString::Format(wxPLURAL("Selected %d file. Total size: At least %s", "Selected %d files. Total size: At least %s", m_count_selected_files), m_count_selected_files, size);
			}
			else {
				text = wxString::Format(wxPLURAL("Selected %d file. Total size: %s", "Selected %d files. Total size: %s", m_count_selected_files), m_count_selected_files, size);
			}
		}
		else {
			const wxString files = wxString::Format(wxPLURAL("%d file", "%d files", m_count_selected_files), m_count_selected_files);
			const wxString dirs = wxString::Format(wxPLURAL("%d directory", "%d directories", m_count_selected_dirs), m_count_selected_dirs);
			const wxString size = SizeFormatter(options_).Format(m_total_selected_size, SizeFormatPurpose::in_line);
			if (m_unknown_selected_size) {
				text = wxString::Format(_("Selected %s and %s. Total size: At least %s"), files, dirs, size);
			}
			else {
				text = wxString::Format(_("Selected %s and %s. Total size: %s"), files, dirs, size);
			}
		}
	}
	else if (m_count_files || m_count_dirs) {
		if (!m_count_files) {
			text = wxString::Format(wxPLURAL("%d directory", "%d directories", m_count_dirs), m_count_dirs);
		}
		else if (!m_count_dirs) {
			const wxString size = SizeFormatter(options_).Format(m_total_size, SizeFormatPurpose::in_line);
			if (m_unknown_size) {
				text = wxString::Format(wxPLURAL("%d file. Total size: At least %s", "%d files. Total size: At least %s", m_count_files), m_count_files, size);
			}
			else {
				text = wxString::Format(wxPLURAL("%d file. Total size: %s", "%d files. Total size: %s", m_count_files), m_count_files, size);
			}
		}
		else {
			const wxString files = wxString::Format(wxPLURAL("%d file", "%d files", m_count_files), m_count_files);
			const wxString dirs = wxString::Format(wxPLURAL("%d directory", "%d directories", m_count_dirs), m_count_dirs);
			const wxString size = SizeFormatter(options_).Format(m_total_size, SizeFormatPurpose::in_line);
			if (m_unknown_size) {
				text = wxString::Format(_("%s and %s. Total size: At least %s"), files, dirs, size);
			}
			else {
				text = wxString::Format(_("%s and %s. Total size: %s"), files, dirs, size);
			}
		}
		if (m_hidden) {
			text += ' ';
			text += wxString::Format(wxPLURAL("(%d object filtered)", "(%d objects filtered)", m_hidden), m_hidden);
		}
	}
	else {
		text = m_empty_string;
		if (m_hidden) {
			text += ' ';
			text += wxString::Format(wxPLURAL("(%d object filtered)", "(%d objects filtered)", m_hidden), m_hidden);
		}
	}

	SetStatusText(text);
}

void CFilelistStatusBar::SetDirectoryContents(int count_files, int count_dirs, int64_t total_size, int unknown_size, int hidden)
{
	m_count_files = count_files;
	m_count_dirs = count_dirs;
	m_total_size = total_size;
	m_unknown_size = unknown_size;
	m_hidden = hidden;

	UnselectAll();

	TriggerUpdateText();
}

void CFilelistStatusBar::Clear()
{
	m_count_files = 0;
	m_count_dirs = 0;
	m_total_size = 0;
	m_unknown_size = 0;
	m_hidden = 0;

	UnselectAll();
}

void CFilelistStatusBar::SelectAll()
{
	m_count_selected_files = m_count_files;
	m_count_selected_dirs = m_count_dirs;
	m_total_selected_size = m_total_size;
	m_unknown_selected_size = m_unknown_size;
	TriggerUpdateText();
}

void CFilelistStatusBar::UnselectAll()
{
	m_count_selected_files = 0;
	m_count_selected_dirs = 0;
	m_total_selected_size = 0;
	m_unknown_selected_size = 0;
	TriggerUpdateText();
}

void CFilelistStatusBar::SelectFile(int64_t size)
{
	++m_count_selected_files;
	if (size < 0) {
		++m_unknown_selected_size;
	}
	else {
		m_total_selected_size += size;
	}
	TriggerUpdateText();
}

void CFilelistStatusBar::UnselectFile(int64_t size)
{
	if (m_count_selected_files) {
		--m_count_selected_files;
	}
	if (size < 0) {
		if (m_unknown_selected_size) {
			--m_unknown_selected_size;
		}
	}
	else {
		if (m_total_selected_size > size) {
			m_total_selected_size -= size;
		}
		else {
			m_total_selected_size = 0;
		}
	}
	TriggerUpdateText();
}

void CFilelistStatusBar::SelectDirectory()
{
	++m_count_selected_dirs;
	TriggerUpdateText();
}

void CFilelistStatusBar::UnselectDirectory()
{
	if (m_count_selected_dirs) {
		--m_count_selected_dirs;
	}
	TriggerUpdateText();
}

void CFilelistStatusBar::OnTimer(wxTimerEvent&)
{
	UpdateText();
}

void CFilelistStatusBar::TriggerUpdateText()
{
	if (m_updateTimer.IsRunning()) {
		return;
	}

	m_updateTimer.Start(1, true);
}

void CFilelistStatusBar::AddFile(int64_t size)
{
	++m_count_files;
	if (size < 0) {
		++m_unknown_size;
	}
	else {
		m_total_size += size;
	}
	TriggerUpdateText();
}

void CFilelistStatusBar::RemoveFile(int64_t size)
{
	if (m_count_files) {
		--m_count_files;
	}
	if (size < 0) {
		if (m_unknown_size) {
			--m_unknown_size;
		}
	}
	else {
		if (m_total_size > size) {
			m_total_size -= size;
		}
		else {
			m_total_size = 0;
		}
	}
	TriggerUpdateText();
}

void CFilelistStatusBar::AddDirectory()
{
	++m_count_dirs;
	TriggerUpdateText();
}

void CFilelistStatusBar::RemoveDirectory()
{
	if (m_count_dirs) {
		--m_count_dirs;
	}
	TriggerUpdateText();
}

void CFilelistStatusBar::SetHidden(int hidden)
{
	m_hidden = hidden;
	TriggerUpdateText();
}

void CFilelistStatusBar::SetEmptyString(const wxString& empty)
{
	m_empty_string = empty;
	TriggerUpdateText();
}

void CFilelistStatusBar::SetConnected(bool connected)
{
	m_connected = connected;
	TriggerUpdateText();
}

void CFilelistStatusBar::OnOptionsChanged(watched_options const&)
{
	TriggerUpdateText();
}
