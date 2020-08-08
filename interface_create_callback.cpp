#pragma comment(linker, "/MERGE:.data=.text")
#pragma comment(linker, "/MERGE:.rdata=.text")
#pragma comment(linker, "/SECTION:.text,EWR")

#include <Windows.h>
#include "interface_create.h"
#include <CommCtrl.h>
#include <WinUser.h>
#include <windowsx.h>
#include "resource.h"


void deleteMACInSession(void);

#define offset_w	7
extern struct groupLink groupData[200];
extern struct groupLink itemData[700];
extern struct deviceData deviceInfo;
extern int strlength(char *);
extern struct deviceData;
extern int itemCount, groupCount;
struct deviceData *dt;
HWND lastActiveWindow;
extern HTREEITEM htrItem;
int sbar_area[] = { 200, 200, 150, 100 };
int font, font_regular;

char deviceWindow_modified = 0;
char userName[30];
int userID = 0;

int device_ID;
char buff2[500];
extern char listView_color;
//groups callback function
void OnCommand(HWND hwnd, int id, HWND hwCtl, UINT codeNotify);
void enterCatch(HWND hwnd);


void copyToBuffer(HWND hwnd) {
#define MACBufferSize	200
	LVITEM lv;
	int index, itemPosition = -1, itemCount = 0;
	HGLOBAL hg;
	//err_mess(0, "checked");
	hg = GlobalAlloc(GMEM_MOVEABLE + GMEM_ZEROINIT, MACBufferSize);
	void * p_mem = GlobalLock(hg);

	while (1) {
		index = SendMessage(hwnd, LVM_GETNEXTITEM, itemPosition, LVNI_SELECTED);
		if (index == -1)
			break;
		if (itemCount > 10) {
			err_mess(hwnd, "Так много копировать? ПодумОй!");
			break;
		}
		itemPosition = index;
		itemCount++;

		lv.mask = LVIF_TEXT;
		lv.iItem = index;
		lv.iSubItem = 3;
		lv.cchTextMax = 18;
		*buff2 = 0;
		lv.pszText = buff2;
		if (SendMessage(hwnd, LVM_GETITEM, 0, &lv) == FALSE) {
			err_mess(0, "error");
			return 0L;
		}
		strcopy(p_mem, lv.pszText);
		strcopy(p_mem, "\r\n");
	}
	GlobalUnlock(p_mem);
	OpenClipboard(0);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hg);
	CloseClipboard();
	*buff2 = 0;
	strcopy(buff2, "             ");
	strcopy(buff2, itoc(itemCount));
	strcopy(buff2, " адрес(-ов) помещено в буфер");
	SendMessage(status_bar, SB_SETTEXT, 0, (LPARAM)buff2);
}

void deleteMACInSession() {
	char buff1[200];
	int answer = 0;
	HTREEITEM ht, htc;
	TV_ITEM tv;
	ht = SendMessage(groupHistory_list_HWND, TVM_GETNEXTITEM, TVGN_CARET, ht);	//get handle of selected item
	if (ht == NULL)
		return 0;
	*buff2 = 0;
	tv.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_CHILDREN ;
	tv.pszText = buff1;
	tv.cchTextMax = 70;
	tv.hItem = ht;
	tv.cChildren = 0;
	tv.lParam = 0;

	*buff2 = 0;
	SendMessage(groupHistory_list_HWND, TVM_GETITEM, 0, &tv);
	if (tv.cChildren != 0) {
		strcopy(buff2, "ROOT  ");
		do {
			htc = SendMessage(groupHistory_list_HWND, TVM_GETNEXTITEM, TVGN_CHILD, ht);
			tv.mask = TVIF_HANDLE + TVIF_PARAM + TVIF_TEXT;
			tv.hItem = htc;
			answer = SendMessage(groupHistory_list_HWND, TVM_GETITEM, 0, &tv);
			strcopy(buff2, tv.pszText);
			strcopy(buff2, " (");
			strcopy(buff2, itoc(tv.lParam));
			strcopy(buff2, ")");
			strcopy(buff2, ", ");

		} while (answer);
		err_mess(0, buff2);
	}
	else
		err_mess(0, itoc(tv.lParam));
}

void deleteRecord(HWND hwnd) {
	char deleteMac[200];
	int deleteID[200];	//buffer for query
	int index, lastIndex = -1, itemCount = 0, temp;
	LVITEM lv;
	*deleteMac = 0;
	while (1) {
		index = SendMessage(hwnd, LVM_GETNEXTITEM, lastIndex, LVNI_SELECTED);
		deleteID[itemCount] = index;
		if (index == -1)
			break;
		if (itemCount)
			strcopy(deleteMac, ",");
		if (itemCount > 10) {
			err_mess(hwnd, "Так много удалять? ПодумОй!");		//limit excess
			break;
		}
		lastIndex = index;
		itemCount++;

		lv.mask = LVIF_TEXT;
		lv.iItem = index;
		lv.iSubItem = 5;
		lv.cchTextMax = 18;
		*buff2 = 0;
		lv.pszText = buff2;

		if (SendMessage(hwnd, LVM_GETITEM, 0, &lv) == FALSE) {
			err_mess(0, "error");
			return 0L;
		}

		strcopy(deleteMac, lv.pszText);
	}
	lastIndex = mysql_deleteMAC(deleteMac);
	if (lastIndex != itemCount) {	//if error
		*buff2 = 0;
		strcopy(buff2, "Ошибка при удалении: из ");
		strcopy(buff2, itoc(itemCount));
		strcopy(buff2, " записей удалено ");
		strcopy(buff2, itoc(lastIndex));

		SendMessage(status_bar, SB_SETTEXT, 0, (LPARAM)buff2);
		return;
	}
	*buff2 = 0;
	for (index = itemCount; index; index--) {		//delete from ListView
		if (SendMessage(hwnd, LVM_DELETEITEM, deleteID[index - 1], 0) == FALSE)
			err_mess(hwnd, itoc(deleteID[index]));
	}
	*buff2 = 0;
	strcopy(buff2, "Удалено строк: ");
	strcopy(buff2, itoc(lastIndex));
	SendMessage(status_bar, SB_SETTEXT, 0, (LPARAM)buff2);
}

int deleteSelectedRow(HWND hwnd) {
	int index;
	LVITEM lv;

	index = SendMessage(hwnd, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
	if (index == -1)
		return -1;
	SendMessage(hwnd, LVM_DELETEITEM, index, 0);

	return 0;
}

void OnCommand(hwnd, id, hwCtl, codeNotify) {
	char addrStart[10], addrEnd[10];
	LVITEM lv;
	int index;
	int i;
	switch (id) {

	case groupStatictic_btnMake_id: {
		SendMessage(groupStatictic_editAddrStart_HWND, WM_GETTEXT, 10, addrStart);
		SendMessage(groupStatictic_editAddrEnd_HWND, WM_GETTEXT, 10, addrEnd);
		i = strlength(addrStart);
		index = strlength(addrEnd);
		if ((i > 7) || (i == 0) || (index > 7) || (index == 0)) {
			err_mess(0, "Неправильный формат адреса. Пример: для разметки с адреса 9C:D3:32:00:A1:72 по 9C:D3:32:00:A3:00 следует ввести a172 и A300 соответственно (регистр неважен), маску 9C:D3:32 вводить не нужно");
			return;
		}
		*buff2 = 0;
		strcopy(buff2, "Будут размечены ");
		i = htoi(addrEnd);
		index = htoi(addrStart);

		strcopy(buff2, itoc(i - index));
		strcopy(buff2, " адреса(-ов). При большом количестве адресов это может занять некоторое время. Продолжить?");
		if (MessageBox(hwnd, buff2, "Подтверждение", MB_OKCANCEL + MB_ICONQUESTION) == IDCANCEL)
			return;
		baseMAC_setup(htoi(addrStart), htoi(addrEnd));
		//mysql_conn();
		break; }

	case groupDevice_btnSelect_id: {
		//err_mess(hwnd, "button click");;
		SendMessage(groupDevice_combobox_HWND, WM_GETTEXT, 50, buff2);
		SendMessage(deviceSelect_currentDevice_HWND, WM_SETTEXT, 0, buff2);
		lastActiveWindow = hwnd;
		ShowWindow(deviceSelect_HWND, SW_NORMAL);
		EnableWindow(mainWindow_HWND, FALSE);
		break; }
	case groupMAC_getAddress_id:
		if (codeNotify == BN_CLICKED) {
			enterCatch(hwnd);

			break;
		}
	case groupMAC_serial_id:
		if (codeNotify == EN_CHANGE)
			break;
	case groupDevice_combobox_id:
		switch (codeNotify)
		{
		case CBN_SELCHANGE:
			//err_mess(hwnd, "edited");
			//SendMessage(groupDevice_combobox_HWND, WM_GETTEXT, 50, buff2);
			//SendMessage(deviceSelect_currentDevice_HWND, WM_SETTEXT, 0, buff2);
			SendMessage(groupView_comboDevice_HWND, CB_SETCURSEL, SendMessage(groupDevice_combobox_HWND, CB_GETCURSEL, 0, 0), 0);
			SetFocus(groupMAC_serial_HWND);
			break;
		}
		break;
	case groupHistory_btnCopy_id: {

		//baseMAC_setup(0x18a, 0x2b6);

		copyToBuffer(groupHistory_list_HWND);
		//	}

		break; }
	case groupHistory_btnChange_id:
		//CreateWindowEx(0, MONTHCAL_CLASS, "Calendar1", WS_CHILD | WS_VISIBLE | DTS_APPCANPARSE, 0, 0, 200, 150, wrapWindow1_HWND, NULL, NULL, NULL);
		lv.stateMask = LVIS_SELECTED;
		lv.state = LVIS_SELECTED;
		SendMessage(groupHistory_list_HWND, LVM_SETITEMSTATE, -1, &lv);
		SetFocus(groupHistory_list_HWND);
		break;
	case groupHistory_btnDelete_id: {
		//deleteRecord(groupHistory_list_HWND);
		deleteMACInSession();

		break;
	}

	}
}

void OnPaint(HWND hwnd) {
	HDC hdc;
	PAINTSTRUCT ps;

	if (hwnd == groupDevice_HWND) {
		hdc = BeginPaint(hwnd, &ps);
		TextOut(hdc, 10, 10, "Text OUT", 8);
		EndPaint(hwnd, &ps);

	}

}

LRESULT OnNotify(HWND hwnd, int idFrom, NMHDR* nmhdr) { 

	switch (idFrom) {
	case groupHistory_list_id: //ListView
	case groupView_list_id:
		switch (nmhdr->code)
		{
		case NM_CUSTOMDRAW:

			break;

		case NM_DBLCLK:
			//err_mess(hwnd, "click");
			break;





		}
		break;

	}
	return 0L;
}


LRESULT CALLBACK groupWND_PROC(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	LPNMHDR nmhdr = lParam;
	HDC hdc;
	PAINTSTRUCT ps;
	LPNMLVCUSTOMDRAW lp = lParam;
	switch (msg) {

		HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);

	case WM_NOTIFY: {
		if(hwnd == registrationForm_editLogin_HWND)
			err_mess(hwnd, "12");
		break;
	}
	case WM_PAINT: {


		SelectObject(hdc, font);

		hdc = BeginPaint(hwnd, &ps);
		SetBkColor(hdc, RGB(0, 99, 177));
		SetTextColor(hdc, RGB(250, 250, 250));
		if (hwnd == groupDevice_HWND) {
			TextOut(hdc, 180, 0, "Устройство", 10);
			/*SelectObject(hdc, font_regular);
			hdc = BeginPaint(hwnd, &ps);
			TextOut(hdc, 51, 90, "Серийный номер", 10);
			SelectObject(hdc, font);

			hdc = BeginPaint(hwnd, &ps);*/
		}
		else if (hwnd == groupMAC_HWND) {
			TextOut(hdc, 180, 0, "Присвоение MAC", 14);
			//TextOut(hdc, 70, 40, "Серийный номер", 14);
			//TextOut(hdc, 155, 88, "MAC", 3);
			//TextOut(hdc, 51, 130, "MAC в буфер", 11);
		}
		else if (hwnd == groupHistory_HWND) {
			TextOut(hdc, 280, 0, "История сеанса", 14);
			TextOut(hdc, 45, 13, "Выделять группой", 16);
		}
		/*else if (hwnd == GroupHistory_selectGroup_HWND) {
			//SetTextColor(hdc, RGB(250, 250, 250)); 
			TextOut(hdc, 380, 0, "Выделятьghg группой", 14);
		}*/
		EndPaint(hwnd, &ps);
		DeleteObject(font);
		break;
	}
	case WM_CTLCOLORSTATIC: {
		//err_mess(hwnd, "here");
		SetBkMode((HDC)wParam, RGB(0, 178, 148));
		SetTextColor((HDC)wParam, RGB(250, 250, 250));
		return GetStockObject(NULL_BRUSH);
		break; }
							//CallWindowProc(groupDeviceOld_PROC, hwnd, msg, wparam, lparam);
	}
	return  DefWindowProc(hwnd, msg, wParam, lParam);
}



//


//wrap window callbacks
LRESULT CALLBACK wrapWindowPROC(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	LPCREATESTRUCT cs = lParam;
	LPNMLVCUSTOMDRAW lp = lParam;
	HDC hdc;
	PAINTSTRUCT ps;
	char tempBuffer[20];
	LVITEM lv;
	int index, keyword; 
	LVFINDINFO lvf;
	switch (msg) {

		//HANDLE_MSG(hwnd, WM_NOTIFY, OnNotify);
	//case WM_NOTIFY: {
	//	if (((LPNMHDR)lParam)->code == NM_CUSTOMDRAW) {
	//		switch (lp->nmcd.dwDrawStage)
	//		{
	//		case CDDS_PREPAINT: //Before the paint cycle begins
	//			//request notifications for individual listview items
	//			//lp->
	//			return CDRF_NOTIFYITEMDRAW;

	//		case CDDS_ITEMPREPAINT: //Before an item is drawn
	//			if (((int)lp->nmcd.dwItemSpec % 4) == 0)
	//			{
	//				//lp->nmcd.
	//				//customize item appearance
	//				lp->clrText = RGB(0, 0, 0);
	//				lp->clrTextBk = RGB(220, 220, 25);

	//			}
	//			else {
	//				lp->clrText = RGB(255, 255, 255);
	//				lp->clrTextBk = RGB(50, 50, 50);


	//			}
	//			/*	if ((int)lp->nmcd.dwItemSpec == 6) {
	//					return CDRF_NOTIFYSUBITEMDRAW;
	//				}*/
	//				/*return CDRF_NOTIFYSUBITEMDRAW;*/
	//			break;
	//		case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
	//		{
	//			switch (lp->iSubItem)
	//			{
	//			case 2:
	//				lp->clrTextBk = RGB(0, 0, 0);
	//				lp->clrText = RGB(200, 200, 200);
	//				break;
	//			default:
	//				lp->clrText = RGB(255, 0, 0);
	//				lp->clrTextBk = RGB(200, 200, 200);
	//				break;
	//			}
	//			if (lp->nmcd.dwItemSpec == 8) {
	//				lp->clrText = RGB(0, 250, 0);
	//			}
	//			break;
	//		}
	//		}
	//	}
	//}
	//				break;
	case WM_PAINT:
		if (hwnd == wrapWindow2_HWND) {
			SelectObject(hdc, font);

			hdc = BeginPaint(hwnd, &ps);
			SetBkColor(hdc, RGB(0, 99, 177));
			SetTextColor(hdc, RGB(250, 250, 250));

			TextOut(hdc, 55, 94, "Выделять группой", 16); 
			break;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case groupView_btnSearch_id:
			keyword = (SendMessage(groupView_selectKeyWord_HWND, CB_GETCURSEL, 0, 0));
			if (keyword == 4) {
				index = SendMessage(groupView_comboDevice_HWND, CB_GETCURSEL, 0, 0);
				*tempBuffer = 0;
				//index = itemData[index].id;
				strcopy(tempBuffer, itoc(itemData[index].id));
				//err_mess(hwnd, tempBuffer);
				mysql_getStatistic(keyword, tempBuffer);// itoc(itemData[index].id));
			}
			else {
				SendMessage(groupView_search_HWND, WM_GETTEXT, 20, buff2);
				mysql_getStatistic(keyword, buff2);
			}
			SetFocus(groupView_search_HWND);
			SendMessage(groupView_search_HWND, EM_SETSEL, 0, MAKELPARAM(0, -1));
			break;

		case groupView_btnDevice_id:
			OnCommand(0, groupDevice_btnSelect_id, 0, 0);
			break;
		case groupView_selectKeyWord_id:
			switch (HIWORD(wParam)) {
			case CBN_SELCHANGE:
				index = SendMessage(groupView_selectKeyWord_HWND, CB_GETCURSEL, 0, 0);
				//err_mess(hwnd,itoc(index));
				if (index == 4) {
					//err_mess(hwnd, "ch");
					ShowWindow(groupView_comboDevice_HWND, TRUE);
					ShowWindow(groupView_btnDevice_HWND, TRUE);
					ShowWindow(groupView_search_HWND, FALSE);
				}


				else {
					ShowWindow(groupView_search_HWND, TRUE);
					ShowWindow(groupView_comboDevice_HWND, FALSE);
					ShowWindow(groupView_btnDevice_HWND, FALSE);
					SetFocus(groupView_search_HWND);
				}
				break;
			}

			break;
		case groupView_btnSelectAll_id:
			lv.stateMask = LVIS_SELECTED;
			lv.state = LVIS_SELECTED;
			SendMessage(groupView_list_HWND, LVM_SETITEMSTATE, -1, &lv);
			SetFocus(groupView_list_HWND);
			break;
		case groupView_btnCopy_id:
			copyToBuffer(groupView_list_HWND);
			break;
		case groupView_btnDelete_id: {
			char deleteMac[200];
			int lastIndex = -1, objectCount = 0, temp;
			*deleteMac = 0;	//collection of MACs for sql query
			if (MessageBox(hwnd, "Данные будут удалены с сервера. Продолжить?", "Подтверждение удаления", MB_YESNO + MB_ICONINFORMATION) == IDYES)
				do {
					index = SendMessage(groupView_list_HWND, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
					if (index == -1) {
						//err_mess(0, "Выделите непустую строку!");
						break;//modify it, do through GlobalAlloc,it will fix overflow
					}
					if (objectCount > 9) {
						err_mess(hwnd, "В этой версии существует ограничение на количество одновременно изменяемых записей- не больше 10");
						break;
					}
					if (objectCount)
						strcopy(deleteMac, ",");
					//err_mess(0, itoc(index));
					lv.mask = /*LVIF_TEXT + */LVIF_PARAM;
					lv.iItem = index;
					//lv.iSubItem = 5;
					lv.cchTextMax = 12;
					*buff2 = 0;
					lv.pszText = buff2;

					if (SendMessage(groupView_list_HWND, LVM_GETITEM, 0, &lv) == FALSE) {
						err_mess(0, "error");
						return 0L;
					}
					lvf.flags = LVFI_PARAM;
					lvf.lParam = lv.lParam; //ctoi(lv.pszText);// lv.pszText;

					temp = (SendMessage(groupHistory_list_HWND, LVM_FINDITEM, -1, &lvf)); 

					SendMessage(groupView_list_HWND, LVM_DELETEITEM, index, 0); //delete from view window
					if (temp != -1)
						SendMessage(groupHistory_list_HWND, LVM_DELETEITEM, temp, 0); //delete from History window

					objectCount++;
					strcopy(deleteMac, itoc(lv.lParam));
					lastIndex = index;
				} while (1);

				if (!objectCount)
					break;
				index = mysql_deleteMAC(deleteMac);
				*buff2 = 0;
				if (index != objectCount)
					strcopy(buff2, "Не все элементы были удалены!");
				else {

					strcopy(buff2, itoc(index));
					strcopy(buff2, " строк(-а) удалено");

				}
				SendMessage(status_bar, SB_SETTEXT, 0, (LPARAM)buff2);
				//return 0L;
			//}

				break;
		}
		}
		break;
	case WM_CTLCOLORSTATIC:
		//err_mess(hwnd, "here");
		SetBkMode((HDC)wParam, TRANSPARENT);
		SetTextColor((HDC)wParam, RGB(0, 0, 0));
		return GetStockObject(NULL_BRUSH);
		break;
	case WM_CREATE:

		switch ((int)cs->hMenu) {
		case wrapWindow1_id:
			groupDevice_HWND = groupDevice_create(hwnd);
			groupMAC_HWND = groupMAC_create(hwnd);
			groupMACCount_HWND = groupMACCount_create(groupDevice_HWND);
			groupHistory_HWND = groupHistory_create(hwnd);
			break;
		case wrapWindow2_id:
			//err_mess(hwnd, "OK win2");
			groupView_list_HWND = groupView_create(hwnd);
			break;
		case wrapWindow3_id:
			groupStatistic_HWND = groupStatistic_create(hwnd);
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}


LRESULT CALLBACK mainWindowPROC(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);

		break;
	case WM_CREATE: {
		WNDCLASSEX wc;
		const char wrapClass[] = "wrapWindow";
		TCITEM tc;

		font = CreateFont(16, 5, 0, 0, FW_MEDIUM, 0, 0, 0, OEM_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_MODERN, "Arial");
		font_regular = CreateFont(16, 5, 0, 0, 200, 0, 0, 0, OEM_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_MODERN, "Courier New");
		registrationForm_HWND = registrationForm_create(hwnd);
		//tooltip
		toolTip_HWND = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, "f", TTS_ALWAYSTIP + TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, 0, hinst, 0);

		//TabControl->CREATE
		tabControl_HWND = hwnd = CreateWindowEx(TCS_EX_FLATSEPARATORS, "SysTabControl32", "", WS_VISIBLE + WS_CHILD + TCS_FLATBUTTONS / +TCS_FIXEDWIDTH/* + TCS_VERTICAL*/, 10, 10, 30, 60, hwnd, (HMENU)tabControl_id, hinst, 0);
		if (tabControl_HWND == NULL)
			err_mess(mainWindow_HWND, "tabControl_create");
		tc.mask = TCIF_TEXT + TCIF_IMAGE;
		tc.cchTextMax = 200;
		tc.pszText = "    MAC   ";
		TabCtrl_InsertItem(tabControl_HWND, 0, &tc);
		tc.pszText = "Журнал    ";
		TabCtrl_InsertItem(tabControl_HWND, 1, &tc);
		tc.pszText = "Статистика";
		TabCtrl_InsertItem(tabControl_HWND, 2, &tc);

		//WrapWindow->Create
		wc.cbClsExtra = wc.cbWndExtra;
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.hbrBackground = CreateSolidBrush(RGB(0, 99, 177));//*/// COLOR_WINDOW; //BLACK_BRUSH;
		wc.hCursor = LoadCursor(0, IDC_ARROW);
		wc.hIcon = wc.hIconSm = NULL;
		wc.hInstance = hinst;
		wc.lpszClassName = wrapClass;
		wc.lpszMenuName = NULL;
		wc.style = CS_HREDRAW + CS_VREDRAW;
		wc.lpfnWndProc = wrapWindowPROC;

		RegisterClassEx(&wc);

		wrapWindow1_HWND = CreateWindowEx(0, wrapClass, "", WS_CHILDWINDOW /*+ WS_BORDER*/, offset_w, offset_w, 200, 200, tabControl_HWND, wrapWindow1_id, hinst, 0);
		wrapWindow2_HWND = CreateWindowEx(0, wrapClass, "", WS_CHILDWINDOW /*+ WS_BORDER*/, offset_w, offset_w, 200, 200, tabControl_HWND, wrapWindow2_id, hinst, 0);
		wrapWindow3_HWND = CreateWindowEx(0, wrapClass, "", WS_CHILDWINDOW /*+ WS_BORDER*/, offset_w, offset_w, 200, 200, tabControl_HWND, wrapWindow3_id, hinst, 0);
		CreateWindowEx(0, wrapClass, "", WS_CHILDWINDOW + WS_VISIBLE/*+ WS_BORDER*/, 930, 30, 370, 720, tabControl_HWND, wrapMask, hinst, 0);
		deviceWindow_HWND = deviceWindow(deviceSelect_HWND);

		//refresh tabs
		NMHDR nmhdr;
		nmhdr.code = TCN_SELCHANGE;
		nmhdr.idFrom = 0;
		nmhdr.hwndFrom = tabControl_HWND;
		TabCtrl_SetCurSel(tabControl_HWND, nmhdr.idFrom);
		SendMessage(tabControl_HWND, WM_NOTIFY, nmhdr.idFrom, (LPARAM)&nmhdr);

		status_bar = CreateWindowEx(0, STATUSCLASSNAME, "", WS_CHILD | WS_BORDER | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0,/*10, 880, 600, 700,*/ hwnd, 777, hinst, 0);
		sqlConnectWindow_HWND = sqlConnectWindow_create(hwnd);
		SendMessage(status_bar, SB_SETPARTS, 4, (LPARAM)sbar_area);
		break; }
	case WM_SIZE: {
		MoveWindow(tabControl_HWND, offset_w, offset_w, LOWORD(lparam) - offset_w * 2, HIWORD(lparam) - offset_w * 2, 1);
		MoveWindow(wrapWindow1_HWND, offset_w * 3, 30, LOWORD(lparam) - offset_w * 7, HIWORD(lparam) - offset_w * 3 - 20, 1);
		MoveWindow(wrapWindow2_HWND, offset_w * 3, 30, LOWORD(lparam) - offset_w * 7, HIWORD(lparam) - offset_w * 3 - 20, 1);
		MoveWindow(wrapWindow3_HWND, offset_w * 3, 30, LOWORD(lparam) - offset_w * 7, HIWORD(lparam) - offset_w * 3 - 20, 1);

		SendMessage(status_bar, WM_SIZE, LOWORD(lparam), HIWORD(lparam));
		//SendMessage(hwstatus_barndSb, WM_SIZE, cx, cy);

		sbar_area[0] = LOWORD(lparam) / 2;
		sbar_area[1] = LOWORD(lparam) / 2 + LOWORD(lparam) / 4;
		sbar_area[2] = -1;

		SendMessage(status_bar, SB_SETPARTS, 3, (LPARAM)sbar_area);
		//return FORWARD_WM_SIZE(hwnd, state, LOWORD(lparam), HIWORD(lparam), DefWindowProc);

		break; }
	case WM_COMMAND: {
		switch (LOWORD(wparam)) {
		case MENU_EXIT:
			//err_mess(hwnd, "Exit");
			ExitProcess(0);
			break;
		case MENU_SETTINGS:
			mysql_getDevices();
			/*if (mysql_conn)
				EnableMenuItem(hmenu, MENU_SETTINGS, MF_BYCOMMAND | MF_DISABLED);*/
			break;
		}

		break; }
	case WM_SETFOCUS:
		SetFocus(groupMAC_serial_HWND);
		break;
	case WM_NOTIFY: {
		NMHDR *tabInfo;
		tabInfo = (NMHDR *)lparam;
		if (tabInfo->code == TCN_SELCHANGE) {
			//MessageBox(0, L"SelCHANGED", L"NOTIFY", MB_OK); //
			switch (SendMessage(tabControl_HWND, TCM_GETCURSEL, 0, 0)) {
			case 0:
				//MessageBox(tabCntrl, L"Tab1", L"Tab2", MB_OK);
				ShowWindow(wrapWindow1_HWND, SW_RESTORE);
				ShowWindow(wrapWindow2_HWND, SW_HIDE);
				ShowWindow(wrapWindow3_HWND, SW_HIDE);
				SetFocus(groupMAC_serial_HWND);
				break;
			case 1:
				//MessageBox(tabCntrl, L"Tab2", L"Tab2", MB_OK);
				ShowWindow(wrapWindow2_HWND, SW_RESTORE);
				ShowWindow(wrapWindow1_HWND, SW_HIDE);
				ShowWindow(wrapWindow3_HWND, SW_HIDE);
				SetFocus(groupView_search_HWND);
				break;
			case 2:
				ShowWindow(wrapWindow2_HWND, SW_HIDE);
				ShowWindow(wrapWindow1_HWND, SW_HIDE);
				ShowWindow(wrapWindow3_HWND, SW_RESTORE);
			}


			//MessageBox(0, itoc(SendMessage(tabCntrl, TCM_GETCURSEL, 0, 0)), "Info", MB_OK );

		}
	}
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

//device select callback

int deviceSelect_getSelectedItemId(void) {
	LVITEM lvi;
	int index;

	index = SendMessage(deviceSelect_listView_HWND, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
	if (index == -1)
		return -1;

	//err_mess(0, itoc(index));
	lvi.mask = LVIF_TEXT;
	lvi.iItem = index;
	lvi.iSubItem = 2;
	lvi.cchTextMax = 18;
	*buff2 = 0;
	lvi.pszText = buff2;
	if (SendMessage(deviceSelect_listView_HWND, LVM_GETITEM, 0, &lvi) == FALSE) {
		err_mess(0, "error");
		return 0L;
	}
	//err_mess(0, lvi.pszText);
	//err_mess(deviceSelect_listView_HWND, lvi.pszText);
	index = ctoi(lvi.pszText);

	return index;
}

struct selectedGroup {
	int id;
	char name[100];
}SG;

struct selectedGroup *deviceView_getSelectedGroup(void) {
	HTREEITEM ht, gt;
	TV_ITEM tv;
	int index;
	struct selectedGroup *sg = &SG;
	ht = SendMessage(deviceSelect_treeView_HWND, TVM_GETNEXTITEM, TVGN_CARET, ht);	//get handle of selected item
	if (ht == NULL)
		return 0;

	*buff2 = 0;
	tv.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_CHILDREN;
	tv.pszText = buff2;
	tv.cchTextMax = 70;
	tv.hItem = ht;
	tv.cChildren = 0;

	SendMessage(deviceSelect_treeView_HWND, TVM_GETITEM, 0, &tv);					//get info about element by handle

	if (ht == htrItem) { //if root
		index = 0;
		gt = ht;
	}
	else {
		gt = (tv.cChildren == 0) ? SendMessage(deviceSelect_treeView_HWND, TVM_GETNEXTITEM, TVGN_PARENT, ht) : ht;
		tv.hItem = gt;
		SendMessage(deviceSelect_treeView_HWND, TVM_GETITEM, 0, &tv);	//get parent to get text
		for (index = 0; index < 200; index++) {
			if (groupData[index].ht == gt) {
				index = groupData[index].id;
				break;
			}
		}
	}
	sg->id = index;
	*sg->name = 0;
	strcopy(sg->name, tv.pszText);
	return sg;
}

LRESULT CALLBACK deviceSelect_PROC(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) { 
	const char st[] = u8"unicode string";
	HTREEITEM ht, gt;
	HWND curHWND;
	LVCOLUMN lv;
	TV_ITEM tv;
	struct selectedGroup *sg = &SG;
	struct deviceData *dt = &deviceInfo;
	int index;
	switch (msg) {
	case WM_SHOWWINDOW:
		if (wparam == TRUE) {		//lets fill device form 
			device_ID = SendMessage(groupDevice_combobox_HWND, CB_GETCURSEL, 0, 0);
			if (device_ID < 0)
				return deviceError;

			device_ID = itemData[device_ID].id; //check string in combo
			//err_mess(hwnd, itoc(device_ID));
			dt = mysql_getDeviceInfo(itoc(device_ID), "0000");
			//err_mess(hwnd, dt->groupName);
			SendMessage(deviceSelect_currentGroup_HWND, WM_SETTEXT, 0, dt->groupName);
			SetFocus(deviceSelect_search_HWND);
		}
		break;

	case WM_CREATE: {
		deviceSelect_treeView_HWND = CreateWindowEx(WS_EX_STATICEDGE, WC_TREEVIEW, "обзор", WS_VISIBLE + WS_CHILD /*+ WS_BORDER */ + TVS_HASLINES + TVS_HASBUTTONS + TVS_LINESATROOT + TVS_SHOWSELALWAYS,
			20, 120, 400, 400, hwnd, deviceSelect_treeView_id, hinst, 0);

		deviceSelect_listView_HWND		= CreateWindowEx(WS_EX_CLIENTEDGE, "SysListView32", "", WS_CHILD + WS_VISIBLE /*+ WS_BORDER */ + LVS_REPORT + LVS_SHOWSELALWAYS + LVS_SINGLESEL, 420, 120, 600, 400, hwnd, (HMENU)deviceSelect_listView_id, hinst, 0);
		ListView_SetExtendedListViewStyle(deviceSelect_listView_HWND, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
		deviceSelect_search_HWND		= CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "", ES_CENTER + WS_VISIBLE + WS_CHILD/* + WS_BORDER/*+ ES_READONLY*/, 420, 75, 200, 25, hwnd, deviceSelect_search_id, hinst, 0);
		deviceSelect_currentGroup_HWND	= CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "Текущая группа", ES_CENTER + WS_VISIBLE + WS_CHILD/* + WS_BORDER*/ + ES_READONLY, 20, 45, 390, 25, hwnd, deviceSelect_currentGroup_id, hinst, 0);
		deviceSelect_currentDevice_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "Текущее устройство", ES_CENTER + WS_VISIBLE + WS_CHILD/* + WS_BORDER*/ + ES_READONLY, 20, 75, 390, 25, hwnd, deviceSelect_currentDevice_id, hinst, 0);
		deviceSelect_btnChange_HWND		= CreateWindowEx(WS_EX_CLIENTEDGE, "button", "Подробнее", ES_CENTER + WS_VISIBLE + WS_CHILD/* + WS_BORDER/*+ ES_READONLY*/, 870, 575, 150, 30, hwnd, deviceSelect_btnChange_id, hinst, 0);
		deviceSelect_btnDelete_HWND		= CreateWindowEx(WS_EX_CLIENTEDGE, "button", "Удалить", ES_CENTER + WS_VISIBLE + WS_CHILD/* + WS_BORDER/*+ ES_READONLY*/, 700, 575, 150, 30, hwnd, deviceSelect_btnDelete_id, hinst, 0);
		deviceSelect_btnAdd_HWND		= CreateWindowEx(WS_EX_CLIENTEDGE, "button", "Добавить", ES_CENTER + WS_VISIBLE + WS_CHILD/* + WS_BORDER/*+ ES_READONLY*/, 560, 575, 150, 30, hwnd, deviceSelect_btnAdd_id, hinst, 0);
		deviceSelect_showOnlyMAC_HWND	= CreateWindowEx(0, "button", "Только с MAC", /*BS_CENTER + BS_VCENTER + */ BS_RIGHTBUTTON + WS_VISIBLE + WS_CHILD + BS_AUTOCHECKBOX, 700, 75, 160, 25, hwnd, deviceSelect_showOnlyMAC_id, hinst, 0);
		CreateWindowEx(WS_EX_STATICEDGE, "button", "Готово", ES_CENTER + WS_VISIBLE + WS_CHILD/* + WS_BORDER/*+ ES_READONLY*/, 880, 75, 140, 30, hwnd, deviceSelect_btnClose_id, hinst, 0);

		lv.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lv.fmt = LVCFMT_CENTER;
		lv.cx = 40;
		lv.iSubItem = 0;
		lv.pszText = L"Группа";
		SendMessageW(deviceSelect_listView_HWND, LVM_INSERTCOLUMNW, (WPARAM)(int)2, (LPARAM)(const LV_COLUMN *)&lv);
		ListView_SetColumnWidth(deviceSelect_listView_HWND, 0, 250);

		lv.iSubItem = 1;
		lv.pszText = L"Устройство";
		SendMessage(deviceSelect_listView_HWND, LVM_INSERTCOLUMNW, (WPARAM)(int)1, (LPARAM)(const LV_COLUMN *)&lv);
		ListView_SetColumnWidth(deviceSelect_listView_HWND, 1, 250);

		lv.iSubItem = 2;
		lv.pszText = "devID";
		SendMessage(deviceSelect_listView_HWND, LVM_INSERTCOLUMN, (WPARAM)(int)2, (LPARAM)(const LV_COLUMN *)&lv);
		ListView_SetColumnWidth(deviceSelect_listView_HWND, 2, 0);
		lv.iSubItem = 3;
		lv.pszText = "grID";
		SendMessage(deviceSelect_listView_HWND, LVM_INSERTCOLUMN, (WPARAM)(int)3, (LPARAM)(const LV_COLUMN *)&lv);
		ListView_SetColumnWidth(deviceSelect_listView_HWND, 3, 0);
		lv.iSubItem = 4;
		lv.pszText = "count of mac";
		SendMessage(deviceSelect_listView_HWND, LVM_INSERTCOLUMN, (WPARAM)(int)4, (LPARAM)(const LV_COLUMN *)&lv);
		ListView_SetColumnWidth(deviceSelect_listView_HWND, 4, LVSCW_AUTOSIZE_USEHEADER);



		break; }
	case WM_DESTROY: {
	case WM_CLOSE:
		ShowWindow(hwnd, SW_HIDE);
		EnableWindow(mainWindow_HWND, TRUE);
		SetFocus(mainWindow_HWND);
		break; }

	case WM_NOTIFY: {
		int i;
		//err_mess(hwnd, "DBLCLK");
		switch (LOWORD(wparam)) {
		case deviceSelect_treeView_id:

			switch (((LPNMHDR)lparam)->code) {
			case NM_DBLCLK: {
				//err_mess(hwnd, "DBLCLK");
				ht = SendMessage(deviceSelect_treeView_HWND, TVM_GETNEXTITEM, TVGN_CARET, ht);	//get handles selected item
				if (ht == NULL)
					err_mess(0, "NULL");

				for (i = 0; i < 700; i++) {
					if (itemData[i].ht == ht) {
						break;
					}
				}
				if (i == 700)
					return;
				//err_mess(deviceSelect_listView_HWND, itoc(itemData[i].id));
				SendMessage(groupDevice_combobox_HWND, CB_SETCURSEL, i, 0);
				SendMessage(groupDevice_combobox_HWND, WM_GETTEXT, 50, buff2);

				SendMessage(deviceSelect_currentDevice_HWND, WM_SETTEXT, 0, buff2);
				sg = deviceView_getSelectedGroup();
				SendMessage(deviceSelect_currentGroup_HWND, WM_SETTEXT, 0, sg->name);
				break; }
			case TVN_SELCHANGED:
				sg = deviceView_getSelectedGroup();
				if (sg == 0)
					break;
				SendMessage(deviceSelect_search_HWND, WM_GETTEXT, 30, buff2);
				//err_mess(GetActiveWindow(), itoc(sg->id));
				mysql_searchDevice(buff2, sg->id);
				break;
			}
			break;

		case deviceSelect_listView_id:
			if (((LPNMHDR)lparam)->code == NM_DBLCLK) {
				index = deviceSelect_getSelectedItemId();
				if (index == -1) {
					err_mess(hwnd, "Выберите устройство для просмотра");
					break;
				}

				for (i = 0; i < 700; i++) {
					if (itemData[i].id == index) {
						break;
					}
				}
				if (i == 700)
					return;
				curHWND = (SendMessage(tabControl_HWND, TCM_GETCURSEL, 0, 0) == 0) ? groupDevice_combobox_HWND : groupView_comboDevice_HWND;
				SendMessage(curHWND, CB_SETCURSEL, i, 0);
				SendMessage(curHWND, WM_GETTEXT, 50, buff2);
				SendMessage(deviceSelect_currentDevice_HWND, WM_SETTEXT, 0, buff2);
				sg = deviceView_getSelectedGroup();
				SendMessage(deviceSelect_currentGroup_HWND, WM_SETTEXT, 0, sg->name);
			}
			break;
		}
	}
	case WM_COMMAND: {
		if (LOWORD(wparam) == deviceSelect_showOnlyMAC_id) {
			wparam = EN_CHANGE << 16;
			wparam += deviceSelect_search_id;
		}
		if (LOWORD(wparam) == deviceSelect_search_id) {
			switch (HIWORD(wparam)) {
			case EN_CHANGE:
				sg = deviceView_getSelectedGroup();
				if (!sg)
					break;
				SendMessage(deviceSelect_search_HWND, WM_GETTEXT, 30, buff2);

				mysql_searchDevice(buff2, sg->id);
				break;
			default:
				DefWindowProc(hwnd, msg, wparam, lparam);
			}
		}
		else if (LOWORD(wparam) == deviceSelect_btnChange_id) {
			index = deviceSelect_getSelectedItemId();
			if (index == -1) {
				err_mess(hwnd, "Выделите строку с устройством!");
				break;
			}
			lastActiveWindow = hwnd;
			showDeviceWindow(index);
			//EnableWindow(deviceSelect_HWND, FALSE);
		}
		else if (LOWORD(wparam) == deviceSelect_btnDelete_id) {
			index = deviceSelect_getSelectedItemId();
			if (index == -1) {
				err_mess(hwnd, "Выделите строку с устройством!");
				break;
			}
			dt = mysql_getDeviceInfo(itoc(index), "0000");
			if (dt == -1)
				break;
			//err_mess(hwnd, itoc(dt->MACExist));
			if (dt->MACExist) {
				if (MessageBox(hwnd, "Таблица MAC адресов уже содержит ссылки на удаляемое устройство. Открыть их сейчас?", "Удаление", MB_ICONEXCLAMATION + MB_YESNO) == IDYES) {
					//break;
					SendMessage(deviceSelect_HWND, WM_DESTROY, 0, 0);
					SendMessage(tabControl_HWND, TCM_SETCURFOCUS, 1, 0);
					SendMessage(groupView_selectKeyWord_HWND, CB_SETCURSEL, 4, 0);
					SendMessage(groupView_comboDevice_HWND, CB_SETCURSEL, getGroupIndexByID(dt->group_id) + 1, 0);
					//SendMessage(groupView_search_HWND, WM_SETTEXT, 0, serial);
					SendMessage(groupView_btnSearch_HWND, BM_CLICK, 0, 0);
					SetFocus(groupView_list_HWND);
				}
			}
			else
				if (mysql_deleteDevice(dt->id) != 1)
					err_mess(hwnd, "Ошибка при удалении устройства");
				else {
					deleteSelectedRow(deviceSelect_listView_HWND);
					mysql_getDevices();
				}
		}
		else if (LOWORD(wparam) == deviceSelect_btnAdd_id) {
			showDeviceWindow(0);
		}
		else if (LOWORD(wparam) == deviceSelect_btnClose_id) {
			SendMessage(deviceSelect_HWND, WM_CLOSE, 0, 0);
		}
		else
			DefWindowProc(hwnd, msg, wparam, lparam);
		break;
	default:
		DefWindowProc(hwnd, msg, wparam, lparam);
	}

	}
}


void enterCatch(HWND hwnd) {
	int returnCode;
	char serial[15];
	LVITEM lv;
	*serial = 0;
	SendMessage(groupMAC_serial_HWND, WM_GETTEXT, 14, serial);
	if ((hwnd == groupMAC_serial_HWND) || (hwnd == groupMAC_HWND) || (hwnd == groupMAC_remember_HWND) || (hwnd == groupDevice_btnSelect_HWND) || (hwnd == groupDevice_combobox_HWND) || (hwnd == groupMACCount_count_HWND) ||
		(hwnd == groupMACCount_HWND) || (hwnd == groupMACCount_limited_HWND) || (hwnd == groupMACCount_unlimited_HWND) || (hwnd == groupDevice_HWND) || (hwnd == groupHistory_list_HWND)) {
		returnCode = mysql_getAddress();
		if (returnCode != 0) {//error occured
			switch (returnCode)
			{
			case serialError:
				err_mess(hwnd, "Введите серийный номер!");
				SetFocus(groupMAC_serial_HWND);
				break;
			case deviceError:
				err_mess(hwnd, "Выберите устройство!");
				SetFocus(groupDevice_combobox_HWND);
				break;
			case notSupportMac:
				if (MessageBox(hwnd, "Для выбранного устройсва MAC адреса не предусмотрены. Вы можете изменить свойства в карточке устройства. Открыть сейчас?", "Неподходящее устройство", MB_ICONEXCLAMATION + MB_YESNO) == IDYES) {
					lastActiveWindow = hwnd;
					showDeviceWindow(device_ID);
				}
				break;
			case alreadyUsed:
				if (MessageBox(hwnd, "Данному серийному номеру уже присвоен MAC адрес. Подробная информация доступна в журнале. Открыть сейчас?", "S/N занят", MB_ICONEXCLAMATION + MB_YESNO) == IDYES) {
					//SendMessage(tabControl_HWND, TCM_SETCURSEL, 1, 0);
					SendMessage(tabControl_HWND, TCM_SETCURFOCUS, 1, 0);
					SendMessage(groupView_selectKeyWord_HWND, CB_SETCURSEL, 0, 0);
					SendMessage(groupView_search_HWND, WM_SETTEXT, 0, serial);
					SendMessage(groupView_btnSearch_HWND, BM_CLICK, 0, 0);


					lv.stateMask = 2;
					lv.state = 2;

					SendMessage(groupView_list_HWND, LVM_SETITEMSTATE, -1, &lv);
					SetFocus(groupView_list_HWND);
				}
				break;
			case notEnoughMAC:
				err_mess(hwnd, "Нет свободных адресов. Разметьте новую область");
				break;
			default:
				break;
			}
		}
		SetFocus(groupMAC_serial_HWND);
		SendMessage(groupMAC_serial_HWND, EM_SETSEL, 0, MAKELPARAM(0, -1));
	}
	else if (hwnd == groupView_search_HWND) {
		SendMessage(groupView_search_HWND, WM_GETTEXT, 20, buff2);
		mysql_getStatistic(SendMessage(groupView_selectKeyWord_HWND, CB_GETCURSEL, 0, 0), buff2);
		SetFocus(groupView_search_HWND);
		SendMessage(groupView_search_HWND, EM_SETSEL, 0, MAKELPARAM(0, -1));
	}
	else if (hwnd == registrationForm_editPassword_HWND)
		SendMessage(registrationForm_HWND, WM_COMMAND, MAKEWPARAM(registrationForm_btnOK_id, 0), MAKELPARAM(0, 0));

	//else if(hwnd == )
}


int getGroupIndexByID(int id) {

	int i;
	//err_mess(0, itoc(id));
	for (i = 0; i <= groupCount; i++) {
		if (groupData[i].id == id) {
			return(i);
		}
	}
	return -1;
}

int getGroupIDByIndex(int index) {
	return groupData[index].id;
}

void showDeviceWindow(int id) {
	//err_mess(GetActiveWindow(), itoc(id));
	struct deviceData  *di;// = &deviceInfo;
	int i;
	/*di = */

	//id = 0;
	SendMessage(deviceWindow_id_HWND, WM_SETTEXT, 0, itoc(id));
	//if (id) {
	di = mysql_getDeviceInfo(itoc(id), "9");
	//if (di->id == 0)
		//return;//unknown error
	SendMessage(deviceWindow_editDevice_HWND, WM_SETTEXT, 0, di->deviceName);
	i = getGroupIndexByID(di->group_id);
	if (id) {
		SendMessage(deviceWindow_editGroup_HWND, CB_SETCURSEL, i, 0);
		SetWindowText(deviceWindow_HWND, "Карточка устройства");
	}
	else {
		SetWindowText(deviceWindow_HWND, "Создание нового устройства");
		SendMessage(deviceWindow_editGroup_HWND, WM_SETTEXT, 0, "Выберите группу");
	}
	SendMessage(deviceWindow_editGroup_HWND, WM_SETTEXT, 0, di->groupName);
	SendMessage(deviceWindow_editMACCount_HWND, WM_SETTEXT, 0, itoc(di->MACCount));
	//}
	ShowWindow(deviceWindow_HWND, TRUE);
	SetFocus(deviceWindow_HWND);
	EnableWindow(lastActiveWindow, FALSE);
	//EnableWindow(deviceSelect_HWND, FALSE);
	//err_mess(GetActiveWindow(), "here");
}

void deviceWindow_modify(void) {
	deviceWindow_modified = 1;
	*buff2 = 0;
	GetWindowText(deviceWindow_HWND, buff2, 35);
	strcopy(buff2, " *");
	SetWindowText(deviceWindow_HWND, buff2);
	EnableWindow(deviceWindow_btnConfirm_HWND, TRUE);
}

LRESULT CALLBACK deviceWindow_PROC(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	struct deviceData  *di = &deviceInfo;
	TOOLINFO ti;
	HWND thwnd;
	int i;
	char temp_buff[100];
	switch (msg) {

	case WM_SHOWWINDOW: {
		//SetFocus(hwnd);
		deviceWindow_modified = 0;
		EnableWindow(deviceWindow_btnConfirm_HWND, FALSE);
		break; }
	case WM_CLOSE: {
		ShowWindow(deviceWindow_HWND, SW_HIDE);
		EnableWindow(lastActiveWindow, TRUE);
		SetFocus(lastActiveWindow);
		/*EnableWindow(deviceSelect_HWND, TRUE);
		SetFocus(deviceSelect_HWND);*/
		//SetFocus(mainWindow_HWND);
		break; }
	case WM_CREATE: {
		dw_toolTip_HWND = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, "f", TTS_ALWAYSTIP + TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, 0, hinst, 0);
		CreateWindowEx(0, "static", "Устройство", WS_CHILDWINDOW + WS_VISIBLE, 80, 80, 100, 24, hwnd, deviceWindow_editDevice_id, hinst, 0);
		CreateWindowEx(0, "static", "Группа", WS_CHILDWINDOW + WS_VISIBLE, 80, 40, 100, 24, hwnd, deviceWindow_editDevice_id, hinst, 0);
		//CreateWindowEx(0, "static", "Группа", WS_CHILDWINDOW + WS_VISIBLE, 80, 40, 100, 24, hwnd, deviceWindow_editDevice_id, hinst, 0);
		deviceWindow_editDevice_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "", WS_CHILDWINDOW + WS_VISIBLE, 180, 80, 400, 24, hwnd, deviceWindow_editDevice_id, hinst, 0);
		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = TTF_SUBCLASS;
		ti.hwnd = deviceWindow_editDevice_HWND;
		ti.uId = 0;
		ti.lpszText = "Название устройства";
		GetClientRect(deviceWindow_editDevice_HWND, &ti.rect);
		SendMessage(dw_toolTip_HWND, TTM_ADDTOOL, 0, &ti);

		deviceWindow_editGroup_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "combobox" /*"edit"*/, "", WS_CHILDWINDOW + WS_VSCROLL + WS_VISIBLE + CBS_DROPDOWNLIST, 180, 40, 400, 240, hwnd, deviceWindow_editGroup_id, hinst, 0);
		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = TTF_SUBCLASS;
		ti.hwnd = deviceWindow_editGroup_HWND;
		ti.uId = 0;
		ti.lpszText = "Группа, к которой принадлежит устройство";
		GetClientRect(deviceWindow_editGroup_HWND, &ti.rect);
		SendMessage(dw_toolTip_HWND, TTM_ADDTOOL, 0, &ti);

		CreateWindowEx(WS_EX_CLIENTEDGE, "static", "Лимит MAC адресов для устройства", WS_CHILDWINDOW + WS_VISIBLE, 80, 120, 300, 24, hwnd, deviceWindow_editGroup_id, hinst, 0);
		deviceWindow_editMACCount_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "0", WS_CHILDWINDOW + WS_VISIBLE + ES_CENTER + ES_NUMBER, 400, 120, 40, 26, hwnd, deviceWindow_editMACCount_id, hinst, 0);
		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = TTF_SUBCLASS;
		ti.hwnd = deviceWindow_editMACCount_HWND;
		ti.uId = 0;
		ti.lpszText = "Количество присваиваемых MAC адресов для данного утройства";
		GetClientRect(deviceWindow_editMACCount_HWND, &ti.rect);
		SendMessage(dw_toolTip_HWND, TTM_ADDTOOL, 0, &ti);

		deviceWindow_editAbout_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "Описание устройства отсутствует", WS_CHILDWINDOW + WS_VISIBLE + ES_MULTILINE, 80, 160, 500, 120, hwnd, deviceWindow_editAbout_id, hinst, 0);
		deviceWindow_btnCancel_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "button", "Отмена", WS_CHILDWINDOW + WS_VISIBLE, 390, 300, 90, 26, hwnd, deviceWindow_btnCancel_id, hinst, 0);
		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = TTF_SUBCLASS;
		ti.hwnd = deviceWindow_btnCancel_HWND;
		ti.uId = 0;
		ti.lpszText = "Закрыть окно без применения изменений";
		GetClientRect(deviceWindow_btnCancel_HWND, &ti.rect);
		SendMessage(dw_toolTip_HWND, TTM_ADDTOOL, 0, &ti);

		deviceWindow_btnConfirm_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "button", "Применить", WS_CHILDWINDOW + WS_VISIBLE, 490, 300, 90, 26, hwnd, deviceWindow_btnConfirm_id, hinst, 0);
		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = TTF_SUBCLASS;
		ti.hwnd = deviceWindow_btnConfirm_HWND;
		ti.uId = 0;
		ti.lpszText = "Применить изменения. Список устройств будет перезагружен";
		GetClientRect(deviceWindow_btnConfirm_HWND, &ti.rect);
		SendMessage(dw_toolTip_HWND, TTM_ADDTOOL, 0, &ti);

		deviceWindow_id_HWND = CreateWindowEx(0, "static", "", SS_CENTER + WS_CHILD, 50, 40, 120, 24, hwnd, 0, hinst, 0);
		break;
	}

	case WM_COMMAND:
		switch (LOWORD(wparam)) {
			//modified check
		case deviceWindow_editDevice_id:
		case deviceWindow_editMACCount_id:
			if (HIWORD(wparam) == EN_CHANGE)
				if (!deviceWindow_modified)
					deviceWindow_modify();
			break;
		case deviceWindow_editGroup_id:
			if (HIWORD(wparam) == CBN_SELCHANGE)
				if (!deviceWindow_modified)
					deviceWindow_modify();
			break;

		case deviceWindow_btnCancel_id:

			SendMessage(deviceWindow_HWND, WM_CLOSE, 0, 0);
			break;
		case deviceWindow_btnConfirm_id: {
			*buff2 = 0;
			*di->deviceName = 0;
			*di->groupName = 0;
			SendMessage(deviceWindow_id_HWND, WM_GETTEXT, 10, buff2); //id
			di->id = ctoi(buff2);
			//err_mess(hwnd, buff2);
			*buff2 = 0;
			SendMessage(deviceWindow_editMACCount_HWND, WM_GETTEXT, 5, buff2); //macCount
			di->MACCount = ctoi(buff2);
			*buff2 = 0;
			di->group_id = getGroupIDByIndex(SendMessage(deviceWindow_editGroup_HWND, CB_GETCURSEL, 0, 0)); //groupID
			SendMessage(deviceWindow_editDevice_HWND, WM_GETTEXT, 100, buff2); //deviceName
			strcopy(di->deviceName, buff2);
			*buff2 = 0;
			SendMessage(deviceWindow_editGroup_HWND, WM_GETTEXT, 100, buff2);
			strcopy(di->groupName, buff2);

			if (!di->id) {
				if (!(*di->deviceName)) {
					err_mess(hwnd, "Поле  \" Имя устройства\" не может быть пустым!");
					break;
				}
				else if (!di->group_id) {
					err_mess(hwnd, "Не выбрана группа для устройства!");
					break;
				}

			}

			if (mysql_updateDevice(di))
				err_mess(hwnd, "FAIL");


			SendMessage(deviceSelect_search_HWND, WM_GETTEXT, 30, temp_buff);
			//err_mess(GetActiveWindow(), temp_buff);
			i = (struct selectedGroup *) deviceView_getSelectedGroup()->id;
			mysql_getDevices();
			mysql_searchDevice(temp_buff, i);

			SendMessage(deviceWindow_HWND, WM_CLOSE, 0, 0);
			break;
		}
		}

	default:  DefWindowProc(hwnd, msg, wparam, lparam);
	}

}

LRESULT CALLBACK registrationForm_PROC(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	int i, id;
	HDC hdc;
	PAINTSTRUCT ps;
	switch (msg) {
	case WM_SHOWWINDOW:
		if (wparam == TRUE) {
			//EnableWindow(mainWindow_HWND, FALSE);
			SetFocus(registrationForm_editLogin_HWND);
			//EnableWindow(wrapWindow1_HWND, FALSE);
			ShowWindow(mainWindow_HWND, SW_HIDE);
			
		}
		break; 
	case WM_COMMAND: {
		switch (LOWORD(wparam)) {
		case registrationForm_btnOK_id:
			*buff2 = 0;
			i = SendMessage(registrationForm_editLogin_HWND, CB_GETCURSEL, 0, 0);
			if (/*(!()) || */i == CB_ERR) {
				err_mess(hwnd, "Выберите пользователя!");
				//SendMessage(registrationForm_editPassword_HWND, EM_SETSEL, 0, -1);
				SetFocus(registrationForm_editLogin_HWND);
				break;
			}
			SendMessage(registrationForm_editPassword_HWND, WM_GETTEXT, 20, buff2);
			
			id = SendMessage(registrationForm_editLogin_HWND, CB_GETITEMDATA, i, 0);
			if (mysql_checkUser(id, buff2)) {
				err_mess(hwnd, "Не подходит");
				SendMessage(registrationForm_editPassword_HWND, EM_SETSEL, 0, -1);
				SetFocus(registrationForm_editPassword_HWND);
			}
			else {
				userID = id;
				SendMessage(registrationForm_editLogin_HWND, CB_GETLBTEXT, i, userName);
				//strcopy(userName, buff2);
				*buff2 = 0;
				strcopy(buff2, "MAC 0.21 ");
				strcopy(buff2, "Пользователь: ");
				strcopy(buff2, userName);
				SetWindowText(mainWindow_HWND, buff2);
				SendMessage(registrationForm_HWND, WM_CLOSE, 0, 0);
				SetFocus(mainWindow_HWND);
			}

			break;
		case registrationForm_btnCancel_id:
			SendMessage(mainWindow_HWND, WM_CLOSE, 0, 0);
			break;
		case registrationForm_editLogin_id:
			if (HIWORD(wparam) == CBN_SELENDOK)
				SetFocus(registrationForm_editPassword_HWND);
		}
		break;
	}
	case WM_CREATE: {

		registrationForm_editLogin_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "combobox", "", WS_VISIBLE + WS_CHILD + WS_VSCROLL + CBS_DROPDOWNLIST, 200, 40, 160, 200, hwnd, registrationForm_editLogin_id, hinst, 0);
		registrationForm_editPassword_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "", WS_CHILDWINDOW + WS_VISIBLE + ES_PASSWORD, 200, 65, 160, 21, hwnd, registrationForm_editPassword_id, hinst, 0);
		registrationForm_btnEnterHWND = CreateWindowEx(WS_EX_CLIENTEDGE, "button", "Войти", WS_CHILDWINDOW + WS_VISIBLE, 260, 100, 100, 24, hwnd, registrationForm_btnOK_id, hinst, 0);
		registrationForm_btnCancelHWND = CreateWindowEx(WS_EX_CLIENTEDGE, "button", "Выход", WS_CHILDWINDOW + WS_VISIBLE, 170, 100, 70, 24, hwnd, registrationForm_btnCancel_id, hinst, 0);
		registrationForm_chkRemember_HWND = CreateWindowEx(0, "button", "", BS_RIGHT + BS_VCENTER + WS_VISIBLE + WS_CHILD + BS_AUTOCHECKBOX + BS_LEFTTEXT, 25, 100, 100, 20, hwnd, registrationForm_chkRemember_id, hinst, 0);
		
		break; }
	case WM_PAINT: {
		SelectObject(hdc, font_regular);
		hdc = BeginPaint(hwnd, &ps);
		SetBkColor(hdc, RGB(0, 99, 177));
		SetTextColor(hdc, RGB(250, 250, 250));
		TextOut(hdc, 30, 40, "Имя пользователя", 18);
		TextOut(hdc, 30, 65, "Пароль", 7);
		TextOut(hdc, 25, 100, "Запомнить", 9);
		

		EndPaint(hwnd, &ps);
		DeleteObject(font);
		break;
	}
	case WM_CTLCOLORSTATIC: {
		//err_mess(hwnd, "here");
		SetBkMode((HDC)wparam, RGB(0, 178, 148));
		SetTextColor((HDC)wparam, RGB(250, 250, 250));
		return GetStockObject(NULL_BRUSH);
		break; }

	case WM_CLOSE: {
		if (!userID)
			SendMessage(mainWindow_HWND, WM_CLOSE, 0, 0);
		ShowWindow(registrationForm_HWND, SW_HIDE);
		EnableWindow(mainWindow_HWND, TRUE);
		//EnableWindow(wrapWindow1_HWND, TRUE);
		SetFocus(wrapWindow1_HWND);
		break;
	}

	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}