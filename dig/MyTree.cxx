#include "MyTree.hpp"
#include <iostream>
#include <string>
#include <PDF.hpp>
#include "PdfDoc.hpp"

#ifdef HAVE_ICONS
#include "icon1.xpm"
#include "icon2.xpm"
#include "icon3.xpm"
#include "icon4.xpm"
#include "icon5.xpm"

enum
{
	TreeCtrlIcon_File,
	TreeCtrlIcon_FileSelected,
	TreeCtrlIcon_Folder,
	TreeCtrlIcon_FolderSelected,
	TreeCtrlIcon_FolderOpened
};

static PdfDoc * theDoc = NULL; // XXX

#endif

enum {
	MyTree_Ctrl = 1000,
};

#if USE_GENERIC_TREECTRL
BEGIN_EVENT_TABLE(MyTree, wxGenericTreeCtrl)
#else
BEGIN_EVENT_TABLE(MyTree, wxTreeCtrl)
#endif
#if 0
    EVT_TREE_BEGIN_DRAG(MyTree_Ctrl, MyTreeCtrl::OnBeginDrag)
    EVT_TREE_BEGIN_RDRAG(MyTree_Ctrl, MyTreeCtrl::OnBeginRDrag)
    EVT_TREE_END_DRAG(MyTree_Ctrl, MyTreeCtrl::OnEndDrag)
    EVT_TREE_BEGIN_LABEL_EDIT(MyTree_Ctrl, MyTreeCtrl::OnBeginLabelEdit)
    EVT_TREE_END_LABEL_EDIT(MyTree_Ctrl, MyTreeCtrl::OnEndLabelEdit)
    EVT_TREE_DELETE_ITEM(MyTree_Ctrl, MyTreeCtrl::OnDeleteItem)
#if 0       // there are so many of those that logging them causes flicker
    EVT_TREE_GET_INFO(MyTree_Ctrl, MyTreeCtrl::OnGetInfo)
#endif
    EVT_TREE_SET_INFO(MyTree_Ctrl, MyTreeCtrl::OnSetInfo)
#endif
    EVT_TREE_ITEM_EXPANDED(MyTree_Ctrl, MyTree::OnItemExpanded)
    EVT_TREE_ITEM_EXPANDING(MyTree_Ctrl, MyTree::OnItemExpanding)
    EVT_TREE_ITEM_COLLAPSED(MyTree_Ctrl, MyTree::OnItemCollapsed)
    EVT_TREE_ITEM_COLLAPSING(MyTree_Ctrl, MyTree::OnItemCollapsing)
    EVT_TREE_SEL_CHANGED(MyTree_Ctrl, MyTree::OnSelChanged)
#if 0
    EVT_TREE_SEL_CHANGING(MyTree_Ctrl, MyTreeCtrl::OnSelChanging)
    EVT_TREE_KEY_DOWN(MyTree_Ctrl, MyTreeCtrl::OnTreeKeyDown)
    EVT_TREE_ITEM_ACTIVATED(MyTree_Ctrl, MyTreeCtrl::OnItemActivated)

    // so many differents ways to handle right mouse button clicks...
    EVT_CONTEXT_MENU(MyTreeCtrl::OnContextMenu)
    // EVT_TREE_ITEM_MENU is the preferred event for creating context menus
    // on a tree control, because it includes the point of the click or item,
    // meaning that no additional placement calculations are required.
    EVT_TREE_ITEM_MENU(MyTree_Ctrl, MyTreeCtrl::OnItemMenu)
    EVT_TREE_ITEM_RIGHT_CLICK(MyTree_Ctrl, MyTreeCtrl::OnItemRClick)

    EVT_RIGHT_DOWN(MyTreeCtrl::OnRMouseDown)
    EVT_RIGHT_UP(MyTreeCtrl::OnRMouseUp)
    EVT_RIGHT_DCLICK(MyTreeCtrl::OnRMouseDClick)
#endif
END_EVENT_TABLE()


#if USE_GENERIC_TREECTRL
IMPLEMENT_DYNAMIC_CLASS(MyTree, wxGenericTreeCtrl)
#else
IMPLEMENT_DYNAMIC_CLASS(MyTree, wxTreeCtrl)
#endif

class MyTreeItemData:public wxTreeItemData
{
	public:
		PDF::OH h;
		MyTreeItemData(PDF::OH handle):h(handle) { }
		std::string dump() const { return h->dump(); }
};

MyTree::MyTree(wxView * view, wxWindow *parent)
	: wxTreeCtrl(parent, MyTree_Ctrl/*id*/, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS)
	, m_view(view)
	, m_details(NULL)
{
	CreateImageList();

	// Add some items to the tree
	//AddTestItemsToTree(5, 2);
	rootId = AddRoot(wxT("The R00t"), -1, -1, NULL);
#if HAVE_ICONS
	SetItemImage(rootId, TreeCtrlIcon_FolderOpened, wxTreeItemIcon_Expanded);
#endif
}

void MyTree::Update()
{
	wxString s;
	PdfDoc * doc = static_cast<PdfDoc*>(m_view->GetDocument());
	doc->GetPrintableName(s);
	SetItemText(rootId, s);
//	SetItemData(NULL);
	wxTreeItemId id = AppendChild(rootId, wxT("Catalog"), doc->get_object( doc->get_root(0) ) );
	AppendChildren(id);
//	SetItemData(rootId, new MyTreeItemData( doc->get_object( doc->get_root(0) ) ));
#if 0
	wxString text;
	text.Printf(wxT("Item #%d"), 666);
	AppendItem(/*GetRootItem()*/ rootId, text /*, MyTreeCtrl::TreeCtrlIcon_File */ );
	text.Printf(wxT("Item #%d"), 1998);
	AppendItem(/*GetRootItem()*/ rootId, text /*, MyTreeCtrl::TreeCtrlIcon_File */ );
#endif
//	wxTreeItemId id = AppendItem(idParent, str, image, imageSel, new MyTreeItemData(str));
}


void MyTree::CreateImageList()
{
#ifdef HAVE_ICONS
	// should correspond to TreeCtrlIcon_xxx enum
	wxBusyCursor wait;
	wxIcon icons[5];
	icons[0] = wxIcon(icon1_xpm);
	icons[1] = wxIcon(icon2_xpm);
	icons[2] = wxIcon(icon3_xpm);
	icons[3] = wxIcon(icon4_xpm);
	icons[4] = wxIcon(icon5_xpm);

	//const int size = 16;
	int size = icons[0].GetWidth();

	// Make an image list containing small icons
	wxImageList *images = new wxImageList(size, size, true);

	for( size_t i = 0; i < WXSIZEOF(icons); i++ ) {
		images->Add(icons[i]);
	}
	AssignImageList(images);
#else
	SetImageList(NULL);
#endif


#if 0 /* custom [+]-like buttons to expand/collapse branches */
	// Make an image list containing small icons
	wxImageList *bimages = new wxImageList(size, size, true);

	// should correspond to TreeCtrlIcon_xxx enum
	icons[0] = wxIcon(icon3_xpm);   // closed
	icons[1] = wxIcon(icon3_xpm);   // closed, selected
	icons[2] = wxIcon(icon5_xpm);   // open
	icons[3] = wxIcon(icon5_xpm);   // open, selected

	for ( size_t i = 0; i < 4; i++ )
	{
		bimages->Add(icons[i]);
	}

	AssignButtonsImageList(bimages);
#else
	SetButtonsImageList(NULL);
#endif
}


void MyTree::OnItemExpanded(wxTreeEvent& event)
{
}

wxTreeItemId MyTree::AppendChild(wxTreeItemId parent, const wxString & name, PDF::OH obj, wxString fullpath)
{
//std::cerr << "AppendChild(" << parent << ", " << name.mb_str() << ", " << obj->type() << "[" << std::hex << (unsigned long)obj.obj() << "]" << ", " << fullpath.mb_str() << ")" << std::endl;
	wxString text;
	wxTreeItemId id;
	fullpath += wxT("/");
	fullpath += name;
	text.Printf(wxT("%s (%s)"), name.c_str(), wxString(obj->type().c_str(), wxConvUTF8).c_str());
	id = AppendItem(parent, text, -1, -1, new MyTreeItemData( obj ));
	return id;
//	AppendChildren(id);
}

void MyTree::AppendChildren(wxTreeItemId id)
{
	MyTreeItemData * d = static_cast<MyTreeItemData*>(GetItemData(id));
	if(!d) return;
	PDF::OH obj = d->h;
	switch(obj.otype()) {
#if 0
		case t_Null:
		case t_Real:
		case t_Integer:
		case t_String:
		case t_Name:
		case t_Keyword:
		case t_Object:
			break;
#endif
//		case PDF::OH::t_Stream: // XXX
		case PDF::OH::t_Dictionary:
			{
				PDF::OH::DictIterator it = obj.begin_dict();
				while(it) {
					AppendChild(id, wxString(it.key().c_str(), wxConvUTF8), it.value());
					++it;
				}
			}
			break;
		case PDF::OH::t_Array:
			{
				unsigned int i;
				for(i = 0; i < obj.size(); i++) {
					wxString name;
					name.Printf(wxT("#%d"), i);
					AppendChild(id, name, obj[i]);
				}
			}
			break;
		case PDF::OH::t_ObjRef:
			{
				PDF::OH tmp = obj;
				tmp.expand();
				AppendChild(id, wxString(obj->dump().c_str(), wxConvUTF8), tmp);
			}
			break;
		default:
			break;
	}
}

void MyTree::OnItemExpanding(wxTreeEvent& event)
{
	wxString text;
	wxTreeItemId id = event.GetItem();
//	MyTreeItemData * d = static_cast<MyTreeItemData*>(GetItemData(id));
//	if(d) std::cout << "OnItemExpanding" << d->dump() << std::endl;
	wxTreeItemIdValue cookie;
	wxTreeItemId child = GetFirstChild(id, cookie);
	while(child.IsOk()) {
		AppendChildren(child);
		child = GetNextChild(id, cookie);
	}
}

void MyTree::OnItemCollapsed(wxTreeEvent& event)
{
}

void MyTree::OnItemCollapsing(wxTreeEvent& event)
{
	wxTreeItemId id = event.GetItem();
//	MyTreeItemData * d = static_cast<MyTreeItemData*>(GetItemData(id));
//	if(d) std::cout << "OnItemCollapsing" << d->dump() << std::endl;
	wxTreeItemIdValue cookie;
	wxTreeItemId child = GetFirstChild(id, cookie);
	while(child.IsOk()) {
		Collapse(child);
		DeleteChildren(child);
		child = GetNextChild(id, cookie);
	}
}

void MyTree::OnSelChanged(wxTreeEvent& event)
{
	wxTreeItemId id = event.GetItem();
	MyTreeItemData * d = static_cast<MyTreeItemData*>(GetItemData(id));
	if(!d) return;
	wxString s(d->h->dump().c_str(), wxConvUTF8);
	m_details->SetValue(s);
}


