#include "MyTree.hpp"

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

#if USE_GENERIC_TREECTRL
BEGIN_EVENT_TABLE(MyTree, wxGenericTreeCtrl)
#else
BEGIN_EVENT_TABLE(MyTree, wxTreeCtrl)
#endif
#if 0
    EVT_TREE_BEGIN_DRAG(TreeTest_Ctrl, MyTreeCtrl::OnBeginDrag)
    EVT_TREE_BEGIN_RDRAG(TreeTest_Ctrl, MyTreeCtrl::OnBeginRDrag)
    EVT_TREE_END_DRAG(TreeTest_Ctrl, MyTreeCtrl::OnEndDrag)
    EVT_TREE_BEGIN_LABEL_EDIT(TreeTest_Ctrl, MyTreeCtrl::OnBeginLabelEdit)
    EVT_TREE_END_LABEL_EDIT(TreeTest_Ctrl, MyTreeCtrl::OnEndLabelEdit)
    EVT_TREE_DELETE_ITEM(TreeTest_Ctrl, MyTreeCtrl::OnDeleteItem)
#if 0       // there are so many of those that logging them causes flicker
    EVT_TREE_GET_INFO(TreeTest_Ctrl, MyTreeCtrl::OnGetInfo)
#endif
    EVT_TREE_SET_INFO(TreeTest_Ctrl, MyTreeCtrl::OnSetInfo)
    EVT_TREE_ITEM_EXPANDED(TreeTest_Ctrl, MyTreeCtrl::OnItemExpanded)
    EVT_TREE_ITEM_EXPANDING(TreeTest_Ctrl, MyTreeCtrl::OnItemExpanding)
    EVT_TREE_ITEM_COLLAPSED(TreeTest_Ctrl, MyTreeCtrl::OnItemCollapsed)
    EVT_TREE_ITEM_COLLAPSING(TreeTest_Ctrl, MyTreeCtrl::OnItemCollapsing)

    EVT_TREE_SEL_CHANGED(TreeTest_Ctrl, MyTreeCtrl::OnSelChanged)
    EVT_TREE_SEL_CHANGING(TreeTest_Ctrl, MyTreeCtrl::OnSelChanging)
    EVT_TREE_KEY_DOWN(TreeTest_Ctrl, MyTreeCtrl::OnTreeKeyDown)
    EVT_TREE_ITEM_ACTIVATED(TreeTest_Ctrl, MyTreeCtrl::OnItemActivated)

    // so many differents ways to handle right mouse button clicks...
    EVT_CONTEXT_MENU(MyTreeCtrl::OnContextMenu)
    // EVT_TREE_ITEM_MENU is the preferred event for creating context menus
    // on a tree control, because it includes the point of the click or item,
    // meaning that no additional placement calculations are required.
    EVT_TREE_ITEM_MENU(TreeTest_Ctrl, MyTreeCtrl::OnItemMenu)
    EVT_TREE_ITEM_RIGHT_CLICK(TreeTest_Ctrl, MyTreeCtrl::OnItemRClick)

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

MyTree::MyTree(wxWindow *parent, const wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
	: wxTreeCtrl(parent, id, pos, size, style)
{
	CreateImageList();

	// Add some items to the tree
	//AddTestItemsToTree(5, 2);
	wxTreeItemId rootId = AddRoot(wxT("The R00t"), -1, -1, NULL);
	SetItemImage(rootId, TreeCtrlIcon_FolderOpened, wxTreeItemIcon_Expanded);
	wxString text;
	text.Printf(wxT("Item #%d"), 666);
	AppendItem(/*GetRootItem()*/ rootId, text /*, MyTreeCtrl::TreeCtrlIcon_File */ );
	text.Printf(wxT("Item #%d"), 1998);
	AppendItem(/*GetRootItem()*/ rootId, text /*, MyTreeCtrl::TreeCtrlIcon_File */ );
//	wxTreeItemId id = AppendItem(idParent, str, image, imageSel, new MyTreeItemData(str));
}


void MyTree::CreateImageList()
{
	//SetImageList(NULL);

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
}


