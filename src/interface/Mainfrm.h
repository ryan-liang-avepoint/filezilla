#ifndef FILEZILLA_INTERFACE_MAINFRM_HEADER
#define FILEZILLA_INTERFACE_MAINFRM_HEADER

#include "statusbar.h"
#include "../include/engine_context.h"
#include "../include/notification.h"
#include "serverdata.h"

#include <wx/timer.h>

#ifndef __WXMAC__
#include <wx/taskbar.h>
#endif

#include "../commonui/updater.h"

#include <list>

class CAsyncRequestQueue;
class CContextControl;
class CertStore;
class CEditHandler;
class CMainFrameStateEventHandler;
class CMenuBar;
class COptions;
class CQueue;
class CQueueView;
class CQuickconnectBar;
class Site;
class CSplitterWindowEx;
class CStatusView;
class CState;
class CToolBar;
class CWindowStateManager;

class CMainFrame final : public wxNavigationEnabled<wxFrame>, public COptionChangeEventHandler
#if FZ_MANUALUPDATECHECK
	, protected CUpdateHandler
#endif
{
	friend class CMainFrameStateEventHandler;
public:
	CMainFrame(COptions& options);
	virtual ~CMainFrame();

	CStatusView* GetStatusView() { return m_pStatusView; }
	CQueueView* GetQueue() { return m_pQueueView; }
	CQuickconnectBar* GetQuickconnectBar() { return m_pQuickconnectBar; }
	COptions& GetOptions() { return options_; }
	CEditHandler* GetEditHandler() { return edit_handler_.get(); }

	// Window size and position as well as pane sizes
	void RememberSplitterPositions();
	bool RestoreSplitterPositions();
	void SetDefaultSplitterPositions();

	void CheckChangedSettings();

	void ConnectNavigationHandler(wxEvtHandler* handler);

	wxStatusBar* GetStatusBar() const { return m_pStatusBar; }

	void ProcessCommandLine();

	void PostInitialize();

	CContextControl* GetContextControl() { return m_pContextControl; }

	bool ConnectToSite(Site & data, Bookmark const& bookmark, CState* pState = 0);

	CFileZillaEngineContext& GetEngineContext() { return m_engineContext; }
	void OnEngineEvent(CFileZillaEngine* engine);

private:
	void UpdateLayout();
	void FixTabOrder();

	bool CloseDialogsAndQuit(wxCloseEvent &event);
	void CreateMenus();
	void CreateQuickconnectBar();
	bool CreateMainToolBar();
	void OpenSiteManager(Site const* site = 0);

	void FocusNextEnabled(std::list<wxWindow*>& windowOrder, std::list<wxWindow*>::iterator iter, bool skipFirst, bool forward);

	COptions & options_;

	CFileZillaEngineContext m_engineContext;

	CStatusBar* m_pStatusBar{};
	CMenuBar* m_pMenuBar{};
	CToolBar* m_pToolBar{};
	CQuickconnectBar* m_pQuickconnectBar{};

	CSplitterWindowEx* m_pTopSplitter{}; // If log position is 0, splits message log from rest of panes
	CSplitterWindowEx* m_pBottomSplitter{}; // Top contains view splitter, bottom queue (or queuelog splitter if in position 1)
	CSplitterWindowEx* m_pQueueLogSplitter{};

	CContextControl* m_pContextControl{};

	CStatusView* m_pStatusView{};
	CQueueView* m_pQueueView{};

#if FZ_MANUALUPDATECHECK
	CUpdater* m_pUpdater{};
	virtual void UpdaterStateChanged( UpdaterState s, build const& v );
	void TriggerUpdateDialog();
	wxTimer update_dialog_timer_;
#endif

	void ShowDirectoryTree(bool local, bool show);

	void ShowDropdownMenu(wxMenu* pMenu, wxToolBar* pToolBar, wxCommandEvent& event);

#ifdef __WXMSW__
	virtual WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);
#endif

	void HandleResize();

	void SetupKeyboardAccelerators();

	void OnOptionsChanged(watched_options const& options);

	// Event handlers
	DECLARE_EVENT_TABLE()
	void OnSize(wxSizeEvent& event);
	void OnMenuHandler(wxCommandEvent& event);
	void OnDisconnect(wxCommandEvent&);
	void OnCancel(wxCommandEvent&);
	void OnClose(wxCloseEvent& event);
	void OnReconnect(wxCommandEvent&);
	void OnRefresh(wxCommandEvent&);
	void OnTimer(wxTimerEvent& event);
	void OnSiteManager(wxCommandEvent&);
	void OnProcessQueue(wxCommandEvent& event);
	void OnMenuEditSettings(wxCommandEvent&);
	void OnToggleToolBar(wxCommandEvent& event);
	void OnToggleLogView(wxCommandEvent&);
	void OnToggleDirectoryTreeView(wxCommandEvent& event);
	void OnToggleQueueView(wxCommandEvent& event);
	void OnMenuHelpAbout(wxCommandEvent&);
	void OnFilter(wxCommandEvent& event);
	void OnFilterRightclicked(wxCommandEvent& event);
#if FZ_MANUALUPDATECHECK
	void OnCheckForUpdates(wxCommandEvent& event);
#endif //FZ_MANUALUPDATECHECK
	void OnSitemanagerDropdown(wxCommandEvent& event);
	void OnNavigationKeyEvent(wxNavigationKeyEvent& event);
	void OnChar(wxKeyEvent& event);
	void OnActivate(wxActivateEvent& event);
	void OnToolbarComparison(wxCommandEvent& event);
	void OnToolbarComparisonDropdown(wxCommandEvent& event);
	void OnDropdownComparisonMode(wxCommandEvent& event);
	void OnDropdownComparisonHide(wxCommandEvent& event);
	void OnSyncBrowse(wxCommandEvent& event);
#ifdef __WXMAC__
	void OnChildFocused(wxChildFocusEvent& event);
#else
	void OnIconize(wxIconizeEvent& event);
	void OnTaskBarClick(wxTaskBarIconEvent&);
#endif
#ifdef __WXGTK__
	void OnTaskBarClick_Delayed(wxCommandEvent& event);
#endif
	void OnSearch(wxCommandEvent& event);
	void OnMenuNewTab(wxCommandEvent& event);
	void OnMenuCloseTab(wxCommandEvent& event);

	bool m_bInitDone{};
	bool m_bQuit{};
	wxEventType m_closeEvent{};
	wxTimer m_closeEventTimer;

	std::unique_ptr<CertStore> cert_store_;
	std::unique_ptr<CAsyncRequestQueue> async_request_queue_;
	CMainFrameStateEventHandler* m_pStateEventHandler{};
	std::unique_ptr<CEditHandler> edit_handler_;

	CWindowStateManager* m_pWindowStateManager{};

	CQueue* m_pQueuePane{};

#ifndef __WXMAC__
	wxTaskBarIcon* m_taskBarIcon{};
#endif
#ifdef __WXGTK__
	// There is a bug in KDE, causing the window to toggle iconized state
	// several times a second after uniconizing it from taskbar icon.
	// Set m_taskbar_is_uniconizing in OnTaskBarClick and unset the
	// next time the pending event processing runs and calls OnTaskBarClick_Delayed.
	// While set, ignore iconize events.
	bool m_taskbar_is_uniconizing{};
#endif

	int m_comparisonToggleAcceleratorId{};

#ifdef __WXMAC__
	int m_lastFocusedChild{-1};
#endif

	wxTimer startupTimer_;
};

#endif
