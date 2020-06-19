#pragma comment(linker, "/MERGE:.data=.text")
#pragma comment(linker, "/MERGE:.rdata=.text")
#pragma comment(linker, "/SECTION:.text,EWR")


#include <Windows.h>
#include <C:\m@x\w32_mac\mysql.h>
#include <stdlib.h>
#include <CommCtrl.h>
#include <WinUser.h>
#include "interface_create.h"
#include "resource.h"
extern int strlength(char *);
HBITMAP bPlusIcon, bDelIcon, bCopyIcon, bSelAllIcon;

char memory_p[10];
const char version[] = "MAC 0.2";
const char groupClass[] = "groupClass";
char textBuf[30];
int i;
const char classDeviceSelect[] = "deviceSelect";
const char className[] = "simple class";

void err_mess(HWND hwnd, char *mess) {
	MessageBox(hwnd, mess, "Ошибка", MB_ICONERROR + MB_OK);
}

char *itoc(int value) {
	char *b_uk;
	int i = 1;
	//format check
	b_uk = memory_p;
	*b_uk = 0;
	if (value < 0) {
		*b_uk++ = '-';
		value *= -1;
	}
	if (value < 0)
		value = 0;

	if (value == 0) {
		*b_uk++ = '0';
		*b_uk = 0;
		return memory_p;
	}


	while (i < value)
		i *= 10;
	if (value != i)
		i /= 10;
	for (i; i; i /= 10) {
		*b_uk++ = value / i + 48;
		value %= i;
	}
	*b_uk = 0;
	return memory_p;
}

int ctoi(char *s) {
	i = *s - 48;
	s++;
	while (*s) {
		i *= 10;
		i += (*s - 48);
		s++;
	}
	return i;
}

int htoi(char *s) {
	int ex = strlength(s), x, y;
	char *p;
	const int ex_v[] = { 1, 16, 256, 4096, 65536, 1048576 };
	//i -accum value, x - index iteration, ex- count of chars, y - offset value
	for (i = 0, x = 0, p = (s + ex - 1); ex; p--, ex--, x++) {
		if ((*p < 58) && (*p > 47))
			y = 48;					//digit
		else if ((*p > 64) && (*p < 71))
			y = 55;					//upper 
		else if ((*p > 96) && (*p < 103))
			y = 87;					//lower
		i += (*p - y) * ex_v[x];
	}
	return i;
}
HWND groupMACCount_create(HWND hParent) {
	HWND hwnd;
	//hwnd = CreateWindowEx(0, groupClass, "Количество адресов", BS_GROUPBOX + WS_CHILD + WS_BORDER + WS_VISIBLE + BS_CENTER, 0, 50, 400, 120, hParent, groupMACCount_count_id, hinst, 0);
	// //groupMACCount_unlimited_HWND = CreateWindowEx(0, "button", "Без ограничений", BS_CENTER + BS_VCENTER + WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON, 30, 30, 170, 24, hwnd, groupMACCount_unlimited_id, hinst, 0);
	//// groupMACCount_limited_HWND = CreateWindowEx(0, "button", "Установить лимит", BS_CENTER + BS_VCENTER + WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON, 30, 58, 170, 24, hwnd, groupMACCount_limited_id, hinst, 0);
	// groupMACCount_count_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "combobox", "1", WS_VISIBLE + WS_CHILD /*+ WS_BORDER */+ CBS_DROPDOWN, 250, 58, 70, 70, hwnd, groupMACCount_count_id, hinst, 0);
	// 
	//// SendMessage(groupMACCount_unlimited_HWND, BM_SETCHECK, 1, 0);
	// SendMessage(groupMACCount_count_HWND, WM_SETTEXT, 0, "1");
	// EnableWindow(groupMACCount_count_HWND, FALSE);
	return hwnd;
}

HWND groupMAC_create(HWND hParent) {
	HWND hwnd;
	TOOLINFO ti;
	hwnd = CreateWindowEx(0, groupClass, "", BS_GROUPBOX + WS_CHILD /*+ WS_BORDER*/ + WS_VISIBLE + BS_CENTER, 1, 50, 400, 180, hParent, groupMAC_id, hinst, 0);
	CreateWindowEx(WS_EX_TRANSPARENT, "static", "Серийный номер", SS_CENTER + WS_VISIBLE + WS_CHILD, 50, 40, 120, 24, hwnd, groupMAC_sSerial_id, hinst, 0);
	CreateWindowEx(WS_EX_TRANSPARENT, "static", "MAC адрес:", SS_CENTER + WS_VISIBLE + WS_CHILD, 50, 88, 120, 24, hwnd, groupMAC_sMACAddress_id, hinst, 0);
	groupMAC_serial_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "", ES_CENTER + WS_VISIBLE + WS_CHILD /*+ WS_BORDER*/ + ES_WANTRETURN + ES_NUMBER, 200, 40, 130, 24, hwnd, groupMAC_serial_id, hinst, 0);
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = groupMAC_serial_HWND;
	ti.uId = 0;
	ti.lpszText = "Поле для ввода серийного номера";
	GetClientRect(groupMAC_serial_HWND, &ti.rect);
	SendMessage(toolTip_HWND, TTM_ADDTOOL, 0, &ti);
	groupMAC_MACAddress_HWMD = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "", ES_CENTER + WS_VISIBLE + WS_CHILD /*+ WS_BORDER/*+ ES_READONLY*/, 200, 88, 130, 24, hwnd, groupMAC_MACAddress_id, hinst, 0);
	groupMAC_getAddress_HWND = CreateWindowEx(WS_EX_STATICEDGE, "button", "Получить MAC", BS_CENTER + BS_VCENTER + WS_VISIBLE + WS_CHILD /*+ WS_BORDER*/, 220, 130, 110, 24, hwnd, groupMAC_getAddress_id, hinst, 0);
	groupMAC_remember_HWND = CreateWindowEx(0, "button", "Копировать в буфер", BS_CENTER + BS_VCENTER + WS_VISIBLE + WS_CHILD + BS_AUTOCHECKBOX + BS_LEFTTEXT, 30, 130, 180, 24, hwnd, groupMAC_remember_id, hinst, 0);
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = groupMAC_remember_HWND;
	ti.uId = 0;
	ti.lpszText = "Каждая новая последовательность MAC для введенного серийного номера будет копироваться в буфер с символами переноса строки";
	GetClientRect(groupMAC_remember_HWND, &ti.rect);
	SendMessage(toolTip_HWND, TTM_ADDTOOL, 0, &ti);
	SendMessage(groupMAC_remember_HWND, BM_SETCHECK, 1, 0);
	//EnableWindow(groupMAC_getAddress_HWND, FALSE);
	return hwnd;
}

HWND groupDevice_create(HWND hParent) {
	HWND hwnd;
	TOOLINFO ti;
	hwnd = CreateWindowEx(0, groupClass, "Устройство", WS_CHILD /*+ WS_BORDER*/ + WS_VISIBLE, 405, 50, 400, 180, hParent, groupDevice_id, hinst, 0);
	groupDevice_combobox_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "combobox", "", WS_VISIBLE + WS_CHILD /*+ WS_BORDER*/ + WS_VSCROLL + CBS_DROPDOWNLIST/*+ CBS_AUTOHSCROLL + CBS_DISABLENOSCROLL*/, 30, 40, 230, 224, hwnd, (HMENU)groupDevice_combobox_id, hinst, 0);
	groupDevice_btnSelect_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "button", " ... ", BS_CENTER + BS_VCENTER + WS_VISIBLE + WS_CHILD /*+ WS_BORDER*/, 270, 40, 50, 24, hwnd, groupDevice_btnSelect_id, hinst, 0);
	SendMessage(groupDevice_combobox_HWND, WM_SETTEXT, 0, "Выберите устройство");
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = groupDevice_btnSelect_HWND;
	ti.uId = 0;
	ti.lpszText = "Открыть окно для выбора и редактирования всех доступных устройств";
	GetClientRect(groupDevice_btnSelect_HWND, &ti.rect);
	SendMessage(toolTip_HWND, TTM_ADDTOOL, 0, &ti);
	ti.hwnd = groupDevice_combobox_HWND;
	//ti.uId = 1;
	ti.lpszText = "Устройство- владелец серийного номера и MAC адреса";
	GetClientRect(groupDevice_combobox_HWND, &ti.rect);
	SendMessage(toolTip_HWND, TTM_ADDTOOL, 0, &ti);
	return hwnd;
}

HWND groupHistory_create(HWND hParent) {
	HWND hwnd;
	TOOLINFO ti;
	unsigned int listView_style = 0;
	LVCOLUMN lv;
	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, groupClass, "История сеанса", BS_GROUPBOX + WS_CHILD + WS_BORDER + WS_VISIBLE + BS_CENTER, 30, 250, 880, 360, hParent, groupHistory_id, hinst, 0);
	//ListView as TABLE
	groupHistory_list_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "SysListView32", "", WS_CHILD + WS_VISIBLE /*+ WS_BORDER + LV_VIEW_DETAILS */ + LVS_REPORT + LVS_SHOWSELALWAYS, 5, 40, 700, 325, hwnd, (HMENU)groupHistory_list_id, hinst, 0);
	//ListView_GetExtendedListViewStyle(groupHistory_list_HWND);
	ListView_SetExtendedListViewStyle(groupHistory_list_HWND, ListView_GetExtendedListViewStyle(groupHistory_list_HWND) | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	lv.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lv.fmt = LVCFMT_CENTER;
	lv.cx = 40;

	lv.iSubItem = 0;

	lv.pszText = "№";
	ListView_InsertColumn(groupHistory_list_HWND, 0, &lv);
	//ListView_SetColumnWidth(groupHistory_list_HWND, 0, 30);
	lv.cx = 140;
	lv.iSubItem = 1;
	lv.pszText = "Дата";
	//SendMessage(groupHistory_list_HWND, LVM_INSERTCOLUMNW, (WPARAM)(int)1, (LPARAM)(const LV_COLUMN *)&lv); 
	ListView_InsertColumn(groupHistory_list_HWND, 1, &lv);
	//ListView_SetColumnWidth(groupHistory_list_HWND, 1, 130);
	lv.cx = 120;
	lv.iSubItem = 2;
	lv.pszText = "Серийный номер";
	ListView_InsertColumn(groupHistory_list_HWND, 2, &lv);
	//ListView_SetColumnWidth(groupHistory_list_HWND, 2, 120);
	lv.cx = 140;
	lv.iSubItem = 3;
	lv.pszText = "MAC";
	ListView_InsertColumn(groupHistory_list_HWND, 3, &lv);
	lv.cx = 250;
	lv.iSubItem = 4;
	lv.pszText = "Устройство";
	ListView_InsertColumn(groupHistory_list_HWND, 4, &lv);
	ListView_SetColumnWidth(groupHistory_list_HWND, 4, LVSCW_AUTOSIZE_USEHEADER);
	lv.iSubItem = 5;
	lv.cx = 10;
	lv.pszText = "mac_id";
	ListView_InsertColumn(groupHistory_list_HWND, 5, &lv);
	ListView_SetColumnWidth(groupHistory_list_HWND, 5, 0);

	groupHistory_btnCopy_HWND = CreateWindowEx(WS_EX_STATICEDGE, "button", "Копировать", BS_CENTER + BS_VCENTER + WS_VISIBLE + BS_BITMAP + WS_CHILD/* + WS_BORDER*/, 600, 5, 35, 35, hwnd, groupHistory_btnCopy_id, hinst, 0);
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = groupHistory_btnCopy_HWND;
	ti.uId = 0;
	ti.lpszText = "Поместить выделенные MAC адреса в буфер.";
	GetClientRect(groupHistory_btnCopy_HWND, &ti.rect);
	SendMessage(toolTip_HWND, TTM_ADDTOOL, 0, &ti);

	groupHistory_btnChange_HWND = CreateWindowEx(WS_EX_STATICEDGE, "button", "Изменить", BS_CENTER + BS_VCENTER + BS_PUSHBUTTON + BS_BITMAP + WS_VISIBLE + WS_CHILD/* + WS_BORDER*/, 550, 5, 35, 35, hwnd, groupHistory_btnChange_id, hinst, 0);
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = groupHistory_btnChange_HWND;
	ti.uId = 0;
	ti.lpszText = "Выделить все записи";
	GetClientRect(groupHistory_btnChange_HWND, &ti.rect);
	SendMessage(toolTip_HWND, TTM_ADDTOOL, 0, &ti);

	groupHistory_btnDelete_HWND = CreateWindowEx(WS_EX_STATICEDGE, "button", "Удалить", BS_CENTER + WS_VISIBLE + BS_BITMAP + WS_CHILD /*+ WS_BORDER*/, 650, 5, 35, 35, hwnd, groupHistory_btnDelete_id, hinst, 0);
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = groupHistory_btnDelete_HWND;
	ti.uId = 0;
	ti.lpszText = "Удалить выделенные записи";
	GetClientRect(groupHistory_btnDelete_HWND, &ti.rect);
	SendMessage(toolTip_HWND, TTM_ADDTOOL, 0, &ti);

	GroupHistory_selectGroup_HWND = CreateWindowEx(0, "button", "Выделять группой", /*BS_CENTER + BS_VCENTER + */ BS_RIGHTBUTTON + WS_VISIBLE + WS_CHILD + BS_AUTOCHECKBOX, 30, 10, 160, 25, hwnd, groupHistory_selectGroup_id, hinst, 0);
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = GroupHistory_selectGroup_HWND;
	ti.uId = 0;
	ti.lpszText = "Пока не реализовано";
	GetClientRect(GroupHistory_selectGroup_HWND, &ti.rect);
	SendMessage(toolTip_HWND, TTM_ADDTOOL, 0, &ti);

	SendMessage(groupHistory_btnChange_HWND, BM_SETIMAGE, IMAGE_BITMAP, bSelAllIcon);
	SendMessage(groupHistory_btnDelete_HWND, BM_SETIMAGE, IMAGE_BITMAP, bDelIcon);
	SendMessage(groupHistory_btnCopy_HWND, BM_SETIMAGE, IMAGE_BITMAP, bCopyIcon);
	//EnableWindow(groupHistory_btnChange_HWND, FALSE);
	return hwnd;
}


HWND groupStatistic_create(HWND hParent) {
	HWND hwnd;
	LVCOLUMN lv;
	hwnd = CreateWindowEx(0, groupClass, "История сеанса", BS_GROUPBOX + WS_CHILD + WS_BORDER + WS_VISIBLE + BS_CENTER, 30, 60, 880, 550, hParent, 0, hinst, 0);
	groupStatictic_editAddrStart_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "", ES_CENTER + WS_VISIBLE + WS_CHILD /*+ WS_BORDER*/ + ES_WANTRETURN, 20, 400, 130, 24, hwnd, groupStatictic_editAddrStart_id, hinst, 0);
	groupStatictic_editAddrEnd_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "", ES_CENTER + WS_VISIBLE + WS_CHILD /*+ WS_BORDER*/ + ES_WANTRETURN, 300, 400, 130, 24, hwnd, groupStatictic_editAddrEnd_id, hinst, 0);
	groupStatictic_btnMake_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "button", "Разметить", ES_CENTER + WS_VISIBLE + WS_CHILD /*+ WS_BORDER*/, 300, 440, 130, 30, hwnd, groupStatictic_btnMake_id, hinst, 0);
	CreateWindowEx(WS_EX_CLIENTEDGE, "static", "Адресов в системе", ES_CENTER + WS_VISIBLE + WS_CHILD /*+ WS_BORDER*/ + ES_READONLY, 20, 200, 200, 24, hwnd, 0, hinst, 0);
	CreateWindowEx(WS_EX_CLIENTEDGE, "static", "Свободно адресов", ES_CENTER + WS_VISIBLE + WS_CHILD /*+ WS_BORDER*/ + ES_READONLY, 20, 240, 200, 24, hwnd, 0, hinst, 0);
	groupStatistic_editTotal_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "", ES_CENTER + WS_VISIBLE + WS_CHILD /*+ WS_BORDER*/ + ES_WANTRETURN, 300, 200, 130, 24, hwnd, groupStatictic_editTotal_id, hinst, 0);
	groupStatistic_editFree_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "", ES_CENTER + WS_VISIBLE + WS_CHILD /*+ WS_BORDER*/ + ES_WANTRETURN, 300, 240, 130, 24, hwnd, groupStatictic_editFree_id, hinst, 0);
	groupStatistic_progress_HWND = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 20, 350, 410, 10, hwnd, (HMENU)0, hinst, NULL);

	groupStatistic_list_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "SysListView32", "", WS_CHILD + WS_VISIBLE/* + WS_BORDER */ + LVS_REPORT /*+ LV_VIEW_DETAILS */ + LVS_SHOWSELALWAYS /*+ LVS_SINGLESEL*/, 20, 20, 410, 120, hwnd, (HMENU)groupView_list_id, hinst, 0);
	ListView_SetExtendedListViewStyle(groupStatistic_list_HWND, (ListView_GetExtendedListViewStyle(groupStatistic_list_HWND) | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES));

	lv.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lv.fmt = LVCFMT_CENTER;
	lv.cx = 40;

	lv.iSubItem = 0;
	lv.cchTextMax = 20;

	lv.iSubItem = 0;
	lv.pszText = "Сеанс";
	SendMessage(groupStatistic_list_HWND, LVM_INSERTCOLUMN, (WPARAM)(int)1, (LPARAM)(const LV_COLUMN *)&lv);
	//ListView_InsertColumn(groupHistory_list_HWND, 1, &lv);
	ListView_SetColumnWidth(groupStatistic_list_HWND, 0, 100);
	lv.iSubItem = 1;
	lv.pszText = "Время";
	ListView_InsertColumn(groupStatistic_list_HWND, 1, &lv);
	ListView_SetColumnWidth(groupStatistic_list_HWND, 1, 80);
	lv.iSubItem = 2;
	lv.pszText = "Присвоено";
	ListView_InsertColumn(groupStatistic_list_HWND, 2, &lv);
	ListView_SetColumnWidth(groupStatistic_list_HWND, 2, 120);

	lv.iSubItem = 3;
	lv.pszText = "Комментарий";
	ListView_InsertColumn(groupStatistic_list_HWND, 3, &lv);
	ListView_SetColumnWidth(groupStatistic_list_HWND, 3, LVSCW_AUTOSIZE_USEHEADER);
	/*
	lv.iSubItem = 5;
	lv.pszText = "mac_id";
	ListView_InsertColumn(hwnd, 5, &lv);
	ListView_SetColumnWidth(hwnd, 5, 0);*/
	return hwnd;
}

HWND groupView_create(HWND hParent) {
	HWND hwnd;
	LVCOLUMN lv;
	TOOLINFO ti;
	//ListView as TABLE

	groupView_selectKeyWord_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "combobox", "", WS_VISIBLE + WS_CHILD +/* WS_BORDER +*/ WS_VSCROLL + CBS_DROPDOWNLIST /*+ CBS_AUTOHSCROLL + CBS_DISABLENOSCROLL*/, 30, 50, 220, 140, hParent, (HMENU)groupView_selectKeyWord_id, hinst, 0);
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = groupView_selectKeyWord_HWND;
	ti.uId = 0;
	ti.lpszText = "Категория поиска";
	GetClientRect(groupView_selectKeyWord_HWND, &ti.rect);
	SendMessage(toolTip_HWND, TTM_ADDTOOL, 0, &ti);

	groupView_search_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "", ES_CENTER + WS_VISIBLE + WS_CHILD/* + WS_BORDER/*+ ES_READONLY*/, 300, 50, 230, 25, hParent, groupView_search_id, hinst, 0);
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = groupView_search_HWND;
	ti.uId = 0;
	ti.lpszText = "строка поиска";
	GetClientRect(groupView_search_HWND, &ti.rect);
	SendMessage(toolTip_HWND, TTM_ADDTOOL, 0, &ti);

	groupView_btnSearch_HWND = CreateWindowEx(WS_EX_STATICEDGE, "button", "Поиск", BS_CENTER + BS_VCENTER + WS_VISIBLE + WS_CHILD /*+ WS_BORDER*/, 630, 50, 120, 24, hParent, groupView_btnSearch_id, hinst, 0);
	groupView_btnCopy_HWND = CreateWindowEx(WS_EX_STATICEDGE, "button", "В буфер", BS_CENTER + BS_BITMAP + WS_VISIBLE + WS_CHILD /*+ WS_BORDER*/, 650, 110, 35, 35, hParent, groupView_btnCopy_id, hinst, 0);
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = groupView_btnCopy_HWND;
	ti.uId = 0;
	ti.lpszText = "Поместить выделенные MAC адреса в буфер.";
	GetClientRect(groupView_btnCopy_HWND, &ti.rect);
	SendMessage(toolTip_HWND, TTM_ADDTOOL, 0, &ti);

	//groupView_btnKit_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "button", "Скомплектовать", BS_CENTER + BS_VCENTER + WS_VISIBLE + WS_CHILD /*+ WS_BORDER*/, 780, 320, 120, 24, hParent, groupView_btnKit_id, hinst, 0);
	groupView_btnDelete_HWND = CreateWindowEx(WS_EX_STATICEDGE, "button", "Удалить", BS_CENTER + BS_BITMAP + WS_VISIBLE + WS_CHILD /*+ WS_BORDER*/, 700, 110, 35, 35, hParent, groupView_btnDelete_id, hinst, 0);
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = groupView_btnDelete_HWND;
	ti.uId = 0;
	ti.lpszText = "Удалить выделенные записи";
	GetClientRect(groupView_btnDelete_HWND, &ti.rect);
	SendMessage(toolTip_HWND, TTM_ADDTOOL, 0, &ti);

	groupView_chkIsGroup_HWND = CreateWindowEx(0, "button", "Выделять группой", BS_CENTER + BS_VCENTER + WS_VISIBLE + WS_CHILD + BS_AUTOCHECKBOX, 30, 90, 220, 25, hParent, groupView_chkIsGroup_id, hinst, 0);
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = groupView_chkIsGroup_HWND;
	ti.uId = 0;
	ti.lpszText = "Пока не реализовано";
	GetClientRect(groupView_chkIsGroup_HWND, &ti.rect);
	SendMessage(toolTip_HWND, TTM_ADDTOOL, 0, &ti);

	groupView_btnSelectAll_HWND = CreateWindowEx(WS_EX_STATICEDGE, "button", "Выбрать все", BS_CENTER + BS_BITMAP + WS_VISIBLE + WS_CHILD, 600, 110, 35, 35, hParent, groupView_btnSelectAll_id, hinst, 0);
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = groupView_btnSelectAll_HWND;
	ti.uId = 0;
	ti.lpszText = "Выделить все записи";
	GetClientRect(groupView_btnSelectAll_HWND, &ti.rect);
	SendMessage(toolTip_HWND, TTM_ADDTOOL, 0, &ti);

	//EnableWindow(groupView_btnKit_HWND, FALSE);
	groupView_comboDevice_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "combobox", "", +WS_CHILD /*+ WS_BORDER*/ + WS_VSCROLL + CBS_DROPDOWN/*+ CBS_AUTOHSCROLL + CBS_DISABLENOSCROLL*/, 300, 50, 230, 140, hParent, (HMENU)groupView_comboDevice_id, hinst, 0);
	groupView_btnDevice_HWND = CreateWindowEx(WS_EX_CLIENTEDGE, "button", " ... ", BS_CENTER + BS_VCENTER + WS_CHILD /*+ WS_BORDER*/, 550, 50, 50, 24, hParent, groupView_btnDevice_id, hinst, 0);
	SendMessage(groupView_btnDelete_HWND, BM_SETIMAGE, IMAGE_BITMAP, bDelIcon);
	SendMessage(groupView_btnCopy_HWND, BM_SETIMAGE, IMAGE_BITMAP, bCopyIcon);
	SendMessage(groupView_btnSelectAll_HWND, BM_SETIMAGE, IMAGE_BITMAP, bSelAllIcon);

	SendMessage(groupDevice_combobox_HWND, WM_SETTEXT, 0, "Выберите устройство");

	SendMessage(groupView_selectKeyWord_HWND, CB_ADDSTRING, 0, "Серийный номер");
	SendMessage(groupView_selectKeyWord_HWND, CB_ADDSTRING, 0, "MAC адрес");
	SendMessage(groupView_selectKeyWord_HWND, CB_ADDSTRING, 0, "Дата");
	SendMessage(groupView_selectKeyWord_HWND, CB_ADDSTRING, 0, "Пользователь");
	SendMessage(groupView_selectKeyWord_HWND, CB_ADDSTRING, 0, "Устройство");
	SendMessage(groupView_selectKeyWord_HWND, CB_ADDSTRING, 0, "Показать последние 1000 записей");
	SendMessage(groupView_selectKeyWord_HWND, CB_SETCURSEL, 0, 0);

	hwnd = CreateWindowEx(WS_EX_STATICEDGE, "SysListView32", "", WS_CHILD + WS_VISIBLE/* + WS_BORDER */ + LVS_REPORT /*+ LV_VIEW_DETAILS */ + LVS_SHOWSELALWAYS /*+ LVS_SINGLESEL*/, 30, 150, 720, 450, hParent, (HMENU)groupView_list_id, hinst, 0);
	ListView_SetExtendedListViewStyle(hwnd, (ListView_GetExtendedListViewStyle(hwnd) | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES));

	lv.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lv.fmt = LVCFMT_CENTER;
	lv.cx = 40;

	lv.iSubItem = 0;
	lv.cchTextMax = 20;
	lv.pszText = "№";
	ListView_InsertColumn(hwnd, 0, &lv);
	ListView_SetColumnWidth(hwnd, 0, 30);

	lv.iSubItem = 1;
	lv.pszText = L"Дата";
	SendMessage(hwnd, LVM_INSERTCOLUMNW, (WPARAM)(int)1, (LPARAM)(const LV_COLUMN *)&lv);
	//ListView_InsertColumn(groupHistory_list_HWND, 1, &lv);
	ListView_SetColumnWidth(hwnd, 1, 130);
	lv.iSubItem = 2;
	lv.pszText = "Серийный номер";
	ListView_InsertColumn(hwnd, 2, &lv);
	ListView_SetColumnWidth(hwnd, 2, 120);
	lv.iSubItem = 3;
	lv.pszText = "MAC";
	ListView_InsertColumn(hwnd, 3, &lv);
	ListView_SetColumnWidth(hwnd, 3, 140);

	lv.iSubItem = 4;
	lv.pszText = "Устройство";
	ListView_InsertColumn(hwnd, 4, &lv);
	ListView_SetColumnWidth(hwnd, 4, LVSCW_AUTOSIZE_USEHEADER);

	lv.iSubItem = 5;
	lv.pszText = "mac_id";
	ListView_InsertColumn(hwnd, 5, &lv);
	ListView_SetColumnWidth(hwnd, 5, 0);

	return hwnd;
}

void EntryPoint(void) {

	hinst = GetModuleHandle(0);
	mysql_isConnected = 0;
	main();
	ExitProcess(0);
}


HWND mainWindow_create() {

	WNDCLASSEX wc;
	//const char className[] = "simple class";
	wc.cbClsExtra = wc.cbWndExtra = NULL;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hbrBackground = /*BLACK_BRUSH;// /*COLOR_WINDOW;// */CreateSolidBrush(RGB(164, 176, 176));
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = LoadIcon(hinst, ID_ICONMain);
	wc.hIconSm = LoadIcon(0, ID_ICONMainSm);
	wc.hInstance = hinst;
	wc.lpfnWndProc = mainWindowPROC;
	wc.lpszClassName = className;
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW + CS_VREDRAW;

	RegisterClassEx(&wc);
	return CreateWindowEx(WS_EX_CONTROLPARENT, wc.lpszClassName, version, /*WS_OVERLAPPEDWINDOW + */WS_VISIBLE + WS_SYSMENU + WS_MINIMIZEBOX + WS_DISABLED, CW_USEDEFAULT, CW_USEDEFAULT, 850, 750, 0, hmenu, hinst, 0);
}
//TreeView_InsertItem(0, 0);

int main(void) {
	MSG msg, *p;
	WNDCLASSEX wc;	//child window proto 
	struct deviceData deviceInfo;

	bPlusIcon = LoadImage(hinst, MAKEINTRESOURCE(IDB_PLUS), IMAGE_BITMAP, 0, 0, 0);
	bDelIcon = LoadImage(hinst, MAKEINTRESOURCE(IDB_DELETE), IMAGE_BITMAP, 0, 0, 0);
	bCopyIcon = LoadImage(hinst, MAKEINTRESOURCE(IDB_COPY), IMAGE_BITMAP, 0, 0, 0);
	bSelAllIcon = LoadImage(hinst, MAKEINTRESOURCE(IDB_SELALL), IMAGE_BITMAP, 0, 0, 0);

	wc.cbClsExtra = wc.cbWndExtra = NULL;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hbrBackground = /*BLACK_BRUSH;//*/ COLOR_WINDOW;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = wc.hIconSm = NULL;
	wc.hInstance = hinst;
	wc.lpszClassName = groupClass;
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW + CS_VREDRAW;
	wc.lpfnWndProc = groupWND_PROC;
	RegisterClassEx(&wc);//child window proto END
	hmenu = LoadMenu(hinst, MAKEINTRESOURCE(ID_MENU));

	mainWindow_HWND = mainWindow_create();
	if (mainWindow_HWND == NULL)
		err_mess(0, "cant create window");
	deviceSelect_HWND = CreateDeviceSelectWindow();
	mysql_getDevices();
	mysql_getUsers();
	while (GetMessage(&msg, 0, 0, 0) > 0) {
		p = &msg;
		if ((p->message == WM_KEYDOWN) && (p->wParam == 13)) {//VK_TAB add here handler to change focus through child windows

			/*SendMessage(groupMAC_serial_HWND, WM_GETTEXT, 10, textBuf);
			SendMessage(groupMAC_serial_HWND, WM_CLEAR, 0, 0);
			SendMessage(groupMAC_MACAddress_HWMD, EM_SETREADONLY, 0, 0);
			SendMessage(groupMAC_MACAddress_HWMD, WM_SETTEXT, 0, textBuf);
			SetFocus(groupMAC_MACAddress_HWMD);
			SendMessage(groupMAC_MACAddress_HWMD, EM_SETSEL, 0, MAKELPARAM(0, 10));
			SendMessage(groupMAC_MACAddress_HWMD, EM_SETREADONLY, 1, 0);*/
			//err_mess(p->hwnd, itoc(p->hwnd));
			enterCatch(p->hwnd);
			//mysql_getAddress();

			//err_mess(0, textBuf);
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	ExitProcess(0);
	//InitCommonControls();
	return 0;
}

int CreateDeviceSelectWindow(void) {
	const char classDeviceSelect[] = "deviceSelect";
	WNDCLASSEX wc;
	wc.cbClsExtra = wc.cbWndExtra = NULL;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hbrBackground = COLOR_WINDOW;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hIconSm = wc.hIcon;
	wc.hInstance = hinst;
	wc.lpfnWndProc = deviceSelect_PROC;
	wc.lpszClassName = classDeviceSelect;
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW + CS_VREDRAW;
	RegisterClassEx(&wc);
	return (HWND)CreateWindowEx(0, classDeviceSelect, "Обзор устройств", WS_CAPTION + WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 1100, 700, 0, 0, hinst, 0);
}


HWND deviceWindow(HWND Parent) {
	HWND hwnd;
	WNDCLASSEX wc;
	extern LRESULT CALLBACK deviceWindow_PROC(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	wc.cbClsExtra = wc.cbWndExtra = NULL;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hbrBackground = COLOR_WINDOW;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hIconSm = wc.hIcon;
	wc.hInstance = hinst;
	wc.lpfnWndProc = deviceWindow_PROC;
	wc.lpszClassName = "deviceWindow";
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW + CS_VREDRAW;
	RegisterClassEx(&wc);
	hwnd = CreateWindowEx(0, wc.lpszClassName, "Карточка устройства", WS_SYSMENU, 600, 400, 700, 400, Parent, 0, hinst, 0);
	return hwnd;
}

HWND sqlConnectWindow_create(HWND hwnd) {
	//WNDCLASSEX wc;
	//wc.cbClsExtra = wc.cbWndExtra = NULL;
	//wc.cbSize = sizeof(WNDCLASSEX);
	//wc.hbrBackground = /*BLACK_BRUSH;//*/ COLOR_WINDOW;
	//wc.hCursor = LoadCursor(0, IDC_ARROW);
	//wc.hIcon = wc.hIconSm = NULL;
	//wc.hInstance = hinst;
	//wc.lpszClassName = "sql_conn_window";
	//wc.lpszMenuName = NULL;
	//wc.style = CS_HREDRAW + CS_VREDRAW;
	//wc.lpfnWndProc = groupWND_PROC;
	//RegisterClassEx(&wc);//child window proto END
	////hmenu = LoadMenu(hinst, MAKEINTRESOURCE(ID_MENU));

	//return CreateWindowEx(WS_EX_TOPMOST, wc.lpszClassName, "", /*WS_CHILDWINDOW + WS_VISIBLE*/, CW_USEDEFAULT, CW_USEDEFAULT, 500, 350, 0, 0, hinst, 0);
}

HWND registrationForm_create(HWND hwnd) {
	WNDCLASSEX wc;
	wc.cbClsExtra = wc.cbWndExtra = NULL;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hbrBackground = /*BLACK_BRUSH;//*/ COLOR_WINDOW;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = wc.hIconSm = NULL;
	wc.hInstance = hinst;
	wc.lpszClassName = "sql_conn_window";
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW + CS_VREDRAW;
	wc.lpfnWndProc = registrationForm_PROC;
	RegisterClassEx(&wc);//child window proto END
	//hmenu = LoadMenu(hinst, MAKEINTRESOURCE(ID_MENU));

	return CreateWindowEx(WS_EX_TOPMOST, wc.lpszClassName, "Авторизация", /*WS_CHILDWINDOW + */WS_VISIBLE + WS_CAPTION, CW_USEDEFAULT, CW_USEDEFAULT, 400, 250, 0, 0, hinst, 0);
}