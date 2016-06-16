#ifndef MYTREE_HPP_INCLUDED
#define MYTREE_HPP_INCLUDED
#include "wx/treectrl.h"
#include "wx/docview.h"
#include <PDF.hpp>
#include <wx/textctrl.h>

struct MyTreeEventsHandler
{
	virtual void SelectedNothing()=0;
	virtual void SelectedObject(PDF::OH h, PDF::OH parent)=0;
};

class MyTree: public wxTreeCtrl
{
	private:
		wxTreeItemId rootId, catalogId;
		wxView * m_view;
		MyTreeEventsHandler * m_handler;
	protected:
		wxTreeItemId AppendChild(wxTreeItemId parent, const wxString & name, PDF::OH obj, wxString fullpath = wxT(""));
		void AppendChildren(wxTreeItemId item);
	public:
#if 0
		enum
		{
			TreeCtrlIcon_File,
			TreeCtrlIcon_FileSelected,
			TreeCtrlIcon_Folder,
			TreeCtrlIcon_FolderSelected,
			TreeCtrlIcon_FolderOpened
		};
#endif
		MyTree():m_view(NULL),m_handler(NULL),catalogId(0) { }
		MyTree(wxView * view, wxWindow *parent, MyTreeEventsHandler *handler = NULL);
	//	MyTree(wxWindow *parent, const wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS);
		virtual ~MyTree(){};

		void Update();

#if 0
		void OnBeginDrag(wxTreeEvent& event);
		void OnBeginRDrag(wxTreeEvent& event);
		void OnEndDrag(wxTreeEvent& event);
		void OnBeginLabelEdit(wxTreeEvent& event);
		void OnEndLabelEdit(wxTreeEvent& event);
		void OnDeleteItem(wxTreeEvent& event);
		void OnContextMenu(wxContextMenuEvent& event);
		void OnItemMenu(wxTreeEvent& event);
		void OnGetInfo(wxTreeEvent& event);
		void OnSetInfo(wxTreeEvent& event);
#endif
		void OnItemExpanded(wxTreeEvent& event);
		void OnItemExpanding(wxTreeEvent& event);
		void OnItemCollapsed(wxTreeEvent& event);
		void OnItemCollapsing(wxTreeEvent& event);
		void OnSelChanged(wxTreeEvent& event);
#if 0
		void OnSelChanging(wxTreeEvent& event);
		void OnTreeKeyDown(wxTreeEvent& event);
		void OnItemActivated(wxTreeEvent& event);
		void OnItemRClick(wxTreeEvent& event);

		void OnRMouseDown(wxMouseEvent& event);
		void OnRMouseUp(wxMouseEvent& event);
		void OnRMouseDClick(wxMouseEvent& event);

		void GetItemsRecursively(const wxTreeItemId& idParent,
				wxTreeItemIdValue cookie = 0);

#endif
		void CreateImageList();
#if 0
		void CreateButtonsImageList(int size = 11);

		void AddTestItemsToTree(size_t numChildren, size_t depth);

		void DoSortChildren(const wxTreeItemId& item, bool reverse = false)
		{ m_reverseSort = reverse; wxTreeCtrl::SortChildren(item); }
		void DoEnsureVisible() { if (m_lastItem.IsOk()) EnsureVisible(m_lastItem); }

		void DoToggleIcon(const wxTreeItemId& item);

		void ShowMenu(wxTreeItemId id, const wxPoint& pt);

		int ImageSize(void) const { return m_imageSize; }

		void SetLastItem(wxTreeItemId id) { m_lastItem = id; }
#endif

	protected:
#if 0
		virtual int OnCompareItems(const wxTreeItemId& i1, const wxTreeItemId& i2);

		// is this the test item which we use in several event handlers?
		bool IsTestItem(const wxTreeItemId& item)
		{
			// the test item is the first child folder
			return GetItemParent(item) == GetRootItem() && !GetPrevSibling(item);
		}
	private:
		void AddItemsRecursively(const wxTreeItemId& idParent,
				size_t nChildren,
				size_t depth,
				size_t folder);

		int          m_imageSize;               // current size of images
		bool         m_reverseSort;             // flag for OnCompareItems
		wxTreeItemId m_lastItem,                // for OnEnsureVisible()
								 m_draggedItem;             // item being dragged right now
#endif

		// NB: due to an ugly wxMSW hack you _must_ use DECLARE_DYNAMIC_CLASS()
		//     if you want your overloaded OnCompareItems() to be called.
		//     OTOH, if you don't want it you may omit the next line - this will
		//     make default (alphabetical) sorting much faster under wxMSW.
		DECLARE_DYNAMIC_CLASS(MyTreeCtrl)
		DECLARE_EVENT_TABLE()
};



#endif /* MYTREE_HPP_INCLUDED */

