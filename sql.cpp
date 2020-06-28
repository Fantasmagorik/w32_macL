#include <Windows.h>
#include <C:\m@x\w32_mac\mysql.h>
#include <stdlib.h>
#include <CommCtrl.h>
#include <WinUser.h>
#include "interface_create.h"
//#include <string.h>

//#define groupCount	200
//#define itemCount	1000
#define currentVersion	2
MYSQL conn;
MYSQL *p_conn = 0;
MYSQL_RES* results;
MYSQL_ROW record;
char connectWnd_class[] = "connect class";
char query[41000];
char buff2[500], buff1[300];
int groupCount, itemCount;
//int deviceMap[700];// , tgroupMap[groupCount], tdeviceMap[700];
unsigned int timeout = 2;
struct groupLink groupData[200];
struct groupLink itemData[1000];
struct deviceData deviceInfo;
extern int device_ID;
HTREEITEM htrItem;
struct deviceData *mysql_getDeviceInfo(int);
char listView_color = 0;

void mysql_getDevices(void);
int mysql_getUsers(void);

int strlength(char *str) {
	int i = 0;
	while (*(str + i) != 0)
		i++;
	return i;
}

void strcopy(char *s1, char *s2) {
	char *cur_s1 = s1;
	while (*cur_s1 != 0)
		cur_s1++;
	while (*s2 != 0)
		*cur_s1++ = *s2++;
	*cur_s1 = 0;
}

int strcmp(char *string1, char *string2) {
	while (*string1) {
		if (*string1 != *string2)
			return -1;
		string1++;
		string2++;
	}
	if (*string2)
		return -2; //string 2 longer than source
	return 0;
}
//str

char *utf8_to_ASCII(char *source) {
	char t_buf1[300];
	*buff1 = 0;
	MultiByteToWideChar(CP_UTF8, 0, source, 120, t_buf1, 500);

	WideCharToMultiByte(CP_ACP, 0, t_buf1, 120, buff1, 500, 0, 0);
	return buff1;
}

char *ASCII_to_utf8(char *source) {
	char t_buf1[300];
	*buff1 = 0;
	MultiByteToWideChar(CP_ACP, 0, source, 120, t_buf1, 500);

	WideCharToMultiByte(CP_UTF8, 0, t_buf1, 120, buff1, 500, 0, 0);
	return buff1;
}

int baseMAC_setup(unsigned int startAddress, unsigned int endAddress) {
	//#define macMask "9C:D3:32:"
	const char beginq[] = "INSERT IGNORE INTO mac (mac) VALUES (\"";
	volatile char macString[18] = "9C:D3:32:";// 01:34:67:910:1213:1516
	LVITEM lvstr;
	int i, x, z, count = 0, realCount = 0, offset, desc, limit;

	macString[17] = 0;
	if (mysql_conn())
		return;
	SendMessage(groupStatistic_progress_HWND, PBM_SETRANGE, 0, (LPARAM)MAKELONG(0, ((endAddress - startAddress) / 200) + 1));
	SendMessage(groupStatistic_progress_HWND, PBM_SETSTEP, (WPARAM)1, 0);
	while (startAddress < endAddress) {
		limit = 0;
		*query = 0;
		strcopy(query, beginq);
		do {
			z = 16;
			if (limit)
				strcopy(query, "\"),(\"");

			for (i = 0; i < 6; i++) {
				x = 0xf & (startAddress >> (4 * i));
				offset = (x < 10) ? 48 : 55;
				macString[z--] = (char)(x + offset);
				if (i & 1)
					macString[z--] = ':';
			}
			startAddress++;
			strcopy(query, macString);
			count++; limit++;
		} while ((limit < 200) && (startAddress < endAddress));
		strcopy(query, "\");");
		SendMessage(groupStatistic_progress_HWND, PBM_STEPIT, 0, 0);
		//err_mess(0, query);
		if (mysql_query(&conn, query) != NULL) {
			err_mess(GetActiveWindow(), "Ошибка при попытке разметить новую область");
			return;
		}
		realCount += mysql_affected_rows(&conn);
	}
	*query = 0;
	strcopy(query, "Попытка разметить область на ");
	strcopy(query, itoc(count));
	strcopy(query, ". Реально размечено: ");
	strcopy(query, itoc(realCount));
	err_mess(0, query);
	SendMessage(GetActiveWindow(), PBM_SETPOS, 0, 0);
	return realCount;
}

int mysql_deleteMAC(char *id) {
	const char del_start[] = "UPDATE mac SET serial = NULL where id in(";
	*query = 0;
	strcopy(query, del_start);
	strcopy(query, id);
	strcopy(query, ")");
	//err_mess(0, query);
	if (mysql_conn())
		return -1;
	if (mysql_query(&conn, query) != 0)
		return -1;
	return (mysql_affected_rows(&conn));

}

struct deviceData *mysql_getDeviceInfo(char *id, char *serial) {
	int MACCount;
	int recordCount;
	struct deviceData *dt = &deviceInfo;

	char q_text_0[] = "select ifnull(p.macCount, 0),  (select count(m.id) from mac as m  where serial = \"", q_text_1[] = "\"), (select g.name from product as g where g.id = p.group_id), p.id, p.name, p.group_id from product as p where p.id = ";
	char ws_text1[] = "select ifnull(p.macCount, 0),  (select count(m.id) from mac as m where m.serial is not null AND m.product = p.id ), (select g.name from product as g where g.id = p.group_id), p.id, p.name, p.group_id from product as p where p.id = ";
	//test
	/*err_mess(GetActiveWindow(), id);
	err_mess(GetActiveWindow(), serial);*/


	if (mysql_conn())
		return 0;
	*query = 0;
	if (strcmp(serial, "0000") == 0) {
		strcopy(query, ws_text1);
		strcopy(query, id);
	}
	else {
		strcopy(query, q_text_0);
		strcopy(query, serial);
		strcopy(query, q_text_1);
		strcopy(query, id);
	}
	//err_mess(GetActiveWindow(), query);
	if (mysql_query(&conn, query)) {
		err_mess(GetActiveWindow(), "GetDevice FAIL");
		return 0;
	}
	results = mysql_store_result(&conn);


	//prepare struct - clear
	*dt->deviceName = 0;
	*dt->groupName = 0;
	dt->id = 0;
	dt->group_id = 0;
	dt->MACCount = 0;
	dt->MACExist = 0;

	if (mysql_num_rows(results)) {

		record = mysql_fetch_row(results);
		MACCount = ctoi(record[0]);
		recordCount = ctoi(record[1]);
		/*fill deviceInfo structure*/ {

			//*dt->deviceName = 0;
			//*dt->groupName = 0;
			strcopy(dt->deviceName, utf8_to_ASCII(record[4]));
			strcopy(dt->groupName, utf8_to_ASCII(record[2]));
			//err_mess(GetActiveWindow(), dt->groupName);
			dt->id = ctoi(record[3]);
			dt->group_id = ctoi(record[5]);
			dt->MACCount = MACCount;
			dt->MACExist = recordCount;
		}
	}
	mysql_free_result(results);
	return dt;
}



int mysql_getAddress() {
	int i, x = 0, objectCount, serial, mac_id, bufferItemCount = 0;
	struct deviceData *dt;
	LVITEMW	 lvstr;
	HGLOBAL hg;
	int limitMAC;
	void * p_mem;
	int ifCopy = 0;
	HWND hwnd = GetActiveWindow();
	//form fill check

	{
		device_ID = SendMessage(groupDevice_combobox_HWND, CB_GETCURSEL, 0, 0);
		if (device_ID < 0)
			return deviceError;

		device_ID = itemData[device_ID].id; //check string in combo
		*buff2 = 0;
		SendMessage(groupMAC_serial_HWND, WM_GETTEXT, 14, buff2); //get serial in buff2

		if (strlength(buff2) == 0)
			return serialError;
		if (strcmp("0000", buff2) == 0) {
			err_mess(hwnd, "Значение зарезервировано, проявите фантазию в попытках сломать программу");
			return serialError;
		}
		SendMessage(groupMAC_serial_HWND, WM_SETTEXT, 0, "");	  //clear serial
	}

	if (mysql_conn())
		return;

	lvstr.cchTextMax = 40;
	lvstr.mask = LVIF_TEXT;

	//search dublicate through serial
	//const char serialStart[] = "SELECT date, mac, serial, product.name, product.macCount FROM mac LEFT JOIN product ON product = product.id WHERE serial = \"";
	//const char serialEnd[] = "\"  ORDER BY DATE DESC, mac.id DESC; ";

	//new record callback 
	const char idStart[] = "SELECT date, mac, serial, product.name FROM mac LEFT JOIN product ON product = product.id WHERE mac.id = ";

	//update request
	const char insertStart[] = "UPDATE mac SET serial = \"",
		insertProduct[] = "\", product = ",
		insertEnd[] = ", id = last_insert_id(id) WHERE serial IS NULL ORDER BY id LIMIT 1; ";

	dt = mysql_getDeviceInfo(itoc(device_ID), buff2);

	if (dt->MACCount == 0)
		return notSupportMac;

	if (dt->MACExist != 0)
		return alreadyUsed;

	objectCount = dt->MACCount;


	//buffer?
	if (SendMessage(groupMAC_remember_HWND, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		//err_mess(0, "checked");
		hg = GlobalAlloc(GMEM_MOVEABLE + GMEM_ZEROINIT, objectCount * 19);
		p_mem = GlobalLock(hg);
		ifCopy = 1;
	}

	x = objectCount;
	while (x) {
		*query = 0;
		strcopy(query, insertStart);

		//err_mess(GetActiveWindow(), buff2);
		strcopy(query, buff2);
		strcopy(query, insertProduct);
		//get mac
		strcopy(query, itoc(device_ID)); //
		strcopy(query, insertEnd);
		mysql_query(&conn, query);



		if (mysql_affected_rows(&conn) == 0) {
			err_mess(0, "Похоже свободных MAC адресов не осталось!");
			return notEnoughMAC;
		}
		mac_id = mysql_insert_id(&conn);
		*query = 0;
		strcopy(query, idStart);
		strcopy(query, itoc(mac_id));
		strcopy(query, ";");
		if (mysql_query(&conn, query) == NULL) {
			results = mysql_store_result(&conn);
			if (mysql_num_rows(results) != 1) {
				err_mess(0, "некорректный ответ");
				return queryFail;
			}

			record = mysql_fetch_row(results);
			SendMessage(groupMAC_MACAddress_HWMD, WM_SETTEXT, 0, record[1]);	//show MAC

			*buff1 = 0;
			/*HISTORY filling*/ {
				//ListView_SetBkColor(groupHistory_list_HWND, RGB(20, 100, 50));
				i = SendMessage(groupHistory_list_HWND, LVM_GETITEMCOUNT, 0, 0);		//add string in History
				lvstr.mask = LVIF_TEXT + LVIF_PARAM;
				lvstr.iItem = i;
				lvstr.iSubItem = 0;
				lvstr.pszText = itoc(i + 1);
				lvstr.lParam = mac_id;
				SendMessage(groupHistory_list_HWND, LVM_INSERTITEM, 0, &lvstr);
				lvstr.mask = LVIF_TEXT;
				lvstr.iSubItem = 1;
				//err_mess(0, record[3]);
				lvstr.pszText = record[0];
				SendMessage(groupHistory_list_HWND, LVM_SETITEM, 0, &lvstr);
				lvstr.iSubItem = 2;
				lvstr.pszText = record[2];
				SendMessage(groupHistory_list_HWND, LVM_SETITEM, 0, &lvstr);
				lvstr.iSubItem = 3;
				lvstr.pszText = record[1];

				SendMessage(groupHistory_list_HWND, LVM_SETITEM, 0, &lvstr);

				MultiByteToWideChar(CP_UTF8, 0, record[3], 120, buff1, 500);
				lvstr.iSubItem = 4;
				lvstr.pszText = buff1;
				SendMessage(groupHistory_list_HWND, LVM_SETITEMW, 0, (LPARAM)(LV_ITEM *)&lvstr);
				//test
				lvstr.iSubItem = 5;
				lvstr.pszText = itoc(mac_id);
				SendMessage(groupHistory_list_HWND, LVM_SETITEM, 0, (LPARAM)(LV_ITEM *)&lvstr);
				if (ifCopy) {
					strcopy(p_mem, record[1]); //copy to buffer
					strcopy(p_mem, "\r\n");
					bufferItemCount++;
				}
				x--;
				//objectCount--;
			}
		}
	}
	*buff1 = 0;
	strcopy(buff1, "      ");
	strcopy(buff1, "Получено адресов: ");
	strcopy(buff1, itoc(objectCount));
	if (ifCopy) {
		GlobalUnlock(p_mem);
		OpenClipboard(0);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hg);
		CloseClipboard();
		strcopy(buff1, " . Помещено в буфер");
	}
	SendMessage(status_bar, SB_SETTEXT, 0, (LPARAM)buff1);
	mysql_free_result(results);
	//mysql_close(&conn);
	return 0;
	listView_color ^= listView_color;
	return unknownError;
}



int mysql_conn() {
	int retryAttempt;
	int limitConnect = 3, answer;
	char status[30];

	/*InvalidateRect(mainWindow_HWND, 0, TRUE);
	UpdateWindow(mainWindow_HWND);*/
	//SendMessage(status_bar, SB_SETTEXT, 2, (LPARAM)"Попытка соединения");
	if (p_conn == 0)	/*full init sequence*/ {
		if (mysql_init(&conn) == 0)
			return mysqlLibrary;	//fail init

		/*try to connect*/

		for (retryAttempt = 1; retryAttempt <= limitConnect; retryAttempt++) { 
			*status = 0;
			/*ShowWindow(sqlConnectWindow_HWND, TRUE);
			EnableWindow(mainWindow_HWND, FALSE);*/
			strcopy(status, "Попытка соединения : ");
			strcopy(status, itoc(retryAttempt));
			strcopy(status, " из ");
			strcopy(status, itoc(limitConnect));
			SendMessage(status_bar, SB_SETTEXT, 2, (LPARAM)status);
			answer = mysql_options(&conn, MYSQL_OPT_CONNECT_TIMEOUT, (unsigned int *)&timeout);
			if (mysql_real_connect(&conn, "95.28.93.38"/*"192.168.8.152"*/, "plc_admin", "97SxFt6U", "test_mac", 3306, NULL, 0) != NULL) {
				break;
			}
		}
		if (retryAttempt > limitConnect) {	//fail connect
			SendMessage(status_bar, SB_SETTEXT, 2, (LPARAM) "Нет соединения");
			/*ShowWindow(sqlConnectWindow_HWND, FALSE);
			EnableWindow(mainWindow_HWND, TRUE);*/
			return mysqlConnect;
		}
		if (mysql_query(&conn, "select minimal_support_version,  last_version from prog;") != NULL) {
			return queryFail;
			/*ShowWindow(sqlConnectWindow_HWND, FALSE);
			EnableWindow(mainWindow_HWND, TRUE);*/
		}
		results = mysql_store_result(&conn);
		record = mysql_fetch_row(results);
		if (currentVersion < ctoi(record[0])) {
			err_mess(GetActiveWindow(), "Версия программы несовместима с версией базы данных! Обновите программу или обратитесь к администратору");
			mysql_close(&conn);
			SendMessage(status_bar, SB_SETTEXT, 2, (LPARAM)"Программа устарела");
			p_conn = 0;
			SendMessage(mainWindow_HWND, WM_DESTROY, 0, 0);
			return -5;
		}
		if (currentVersion != ctoi(record[1]))
			MessageBox(GetActiveWindow(), "Доступна новая версия программы", "Информация", MB_ICONINFORMATION + MB_OK);
		mysql_free_result(results);

		p_conn = &conn;
		//err_mess(GetActiveWindow(), "stable connection");
	}
	// end of full init sequence
	mysql_ping(&conn);
	for (retryAttempt = 3; retryAttempt; retryAttempt--) {
		if (mysql_query(&conn, "SET names 'utf8';") == NULL)	//test if already connect
			break;
	}
	if (retryAttempt < 1) {
		p_conn = 0;
		mysql_conn();
	}
	//return queryFail;

//Here we have a stable connection- lets work
//check version compatibale

	if (mysql_query(&conn, "select count(*) as total, (select count(*) from mac where serial is null) as free from mac;") != NULL)
		return 1;
	results = mysql_store_result(&conn);
	record = mysql_fetch_row(results);
	SendMessage(groupStatistic_editTotal_HWND, WM_SETTEXT, 0, record[0]);
	SendMessage(groupStatistic_editFree_HWND, WM_SETTEXT, 0, record[1]);
	//UpdateWindow(groupStatistic_HWND);
	SendMessage(status_bar, SB_SETTEXT, 2, (LPARAM)"Подключено");
	InvalidateRect(mainWindow_HWND, 0, TRUE);
	UpdateWindow(mainWindow_HWND);
	//UpdateWindow(wrapWindow1_HWND);
	mysql_free_result(results);
	/*ShowWindow(sqlConnectWindow_HWND, FALSE);
	EnableWindow(mainWindow_HWND, TRUE);*/

	return 0;
}

void mysql_getStatistic(int keyword, char *keyvalue) {
	LVITEMW	 lvstr;
	int i, x, offset;
	char textBuffer[20];
	const char statisticStart[] = "SELECT date, serial, mac, product.name, mac.id FROM  mac LEFT JOIN product ON product = product.id";
	const char q_cond[] = " WHERE ";

	const char statisticSerial[] = "serial = \"";
	const char statisticDevice[] = "product.id = ";
	const char c[] = " ORDER BY date DESC LIMIT 1000;";
	SendMessage(groupView_list_HWND, LVM_DELETEALLITEMS, 0, 0);
	/*err_mess(GetActiveWindow(), itoc(keyword));
	err_mess(GetActiveWindow(), keyvalue);*/

	*query = 0;
	if (mysql_conn())
		return;

	strcopy(query, statisticStart);
	switch (keyword)
	{
	case 0:
		strcopy(query, q_cond);
		strcopy(query, statisticSerial);
		strcopy(query, keyvalue);
		strcopy(query, "\"");
		break;
	case 1:
	case 2:
	case 3:
		err_mess(wrapWindow2_HWND, "not ready yet");
		return;
	case 4:
		strcopy(query, q_cond);
		strcopy(query, statisticDevice);
		strcopy(query, keyvalue);
		strcopy(query, " AND mac.serial IS NOT NULL");
		break;
	default:
		strcopy(query, " WHERE serial IS NOT NULL ");
		break;
	}
	//err_mess(GetActiveWindow(), query);
	strcopy(query, c);
	if (mysql_query(&conn, query)) {
		// SELECT @i: = @i+1 AS No, date, mac, serial FROM (SELECT @i:=0) AS z, mac  ORDER BY date DESC LIMIT 1000
		//SELECT @i: =  date, mac, serial FROM (SELECT @i:=0) AS z, mac
		err_mess(wrapWindow2_HWND, "Ошибка при попытке загрузить журнал");
		return;
	}
	results = mysql_store_result(&conn);
	i = mysql_num_rows(results);
	if (i == 0) {
		SendMessage(status_bar, SB_SETTEXT, 0, (LPARAM)"Ничего не найдено!");
		//SetFocus(groupView_search_HWND);
		//SendMessage(groupView_search_HWND, EM_SETSEL, 0, MAKELPARAM(0, -1));
		return;
	}
	*buff2 = 0;
	strcopy(buff2, "Найдено строк: ");
	strcopy(buff2, itoc(i));
	SendMessage(status_bar, SB_SETTEXT, 0, (LPARAM)buff2);


	i = 0;
	*textBuffer = 0;
	while (record = mysql_fetch_row(results)) {
		lvstr.iItem = i;
		lvstr.cchTextMax = 50;
		lvstr.mask = LVIF_TEXT;
		lvstr.iSubItem = 0;
		lvstr.pszText = itoc(i + 1);
		SendMessage(groupView_list_HWND, LVM_INSERTITEM, 0, (LPARAM)(const LV_ITEM *)&lvstr);
		lvstr.iSubItem = 1;
		lvstr.pszText = record[0];
		SendMessage(groupView_list_HWND, LVM_SETITEM, 0, (LPARAM)(const LV_ITEM *)&lvstr);
		lvstr.iSubItem = 2;
		lvstr.pszText = record[1];
		//if(strcmp)
		SendMessage(groupView_list_HWND, LVM_SETITEM, 0, (LPARAM)(LV_ITEM *)&lvstr);
		lvstr.iSubItem = 3;
		lvstr.pszText = record[2];
		SendMessage(groupView_list_HWND, LVM_SETITEM, 0, (LPARAM)(LV_ITEM *)&lvstr);
		MultiByteToWideChar(CP_UTF8, 0, record[3], 120, buff2, 500);
		lvstr.iSubItem = 4;
		lvstr.pszText = buff2;
		SendMessage(groupView_list_HWND, LVM_SETITEMW, 0, (LPARAM)(LV_ITEM *)&lvstr);

		lvstr.iSubItem = 5;
		lvstr.pszText = record[4];
		SendMessage(groupView_list_HWND, LVM_SETITEM, 0, (LPARAM)(LV_ITEM *)&lvstr);
		i++;
	}
	//ListView_EnsureVisible(groupView_list_HWND, i-1, FALSE);
	mysql_free_result(results);
}

void mysql_getDevices(void) {
# define UNICODE
	TV_INSERTSTRUCT tvi;
	HTREEITEM htrRoot, elements[700];
	int i, x, offset;
	char textBuffer[20];
	LVITEMW	 lvstr;
	if (mysql_conn())
		return;

	if (mysql_query(&conn, "SELECT id, name FROM product WHERE group_id = 0 ORDER BY name;") != NULL) {
		err_mess(0, "Ошибка при загрузке списка устройств");
		return;
	}
	results = mysql_store_result(&conn);
	//err_mess(0, itoc(mysql_num_rows(results)));
	i = 0;
	SendMessage(deviceSelect_treeView_HWND, TVM_DELETEITEM, 0, NULL);//clear tree
	SendMessage(deviceWindow_editGroup_HWND, CB_RESETCONTENT, 0, 0);//clear group combo deviceWINDOW
	SendMessage(groupDevice_combobox_HWND, CB_RESETCONTENT, 0, 0);
	SendMessage(groupView_comboDevice_HWND, CB_RESETCONTENT, 0, 0);
	SendMessage(deviceSelect_listView_HWND, LVM_DELETEALLITEMS, 0, 0);
	//err_mess(0, "pause");
	tvi.hInsertAfter = TVI_FIRST;
	tvi.hParent = TVI_ROOT;
	tvi.item.mask = TVIF_TEXT;
	tvi.item.cchTextMax = 40;
	tvi.item.pszText = L"Все";
	htrItem = SendMessage(deviceSelect_treeView_HWND, TVM_INSERTITEMW, 0, &tvi);

	while (record = mysql_fetch_row(results)) {
		//groupMap[i] = ctoi(record[0]);
		tvi.hInsertAfter = TVI_LAST;
		tvi.hParent = htrItem;

		MultiByteToWideChar(CP_UTF8, 0, record[1], 120, buff2, 500);
		tvi.item.pszText = buff2;

		groupData[i].ht = SendMessage(deviceSelect_treeView_HWND, TVM_INSERTITEMW, 0, &tvi);
		groupData[i].id = ctoi(record[0]);

		SendMessage(deviceWindow_editGroup_HWND, CB_ADDSTRING, 0, (LPARAM)utf8_to_ASCII(record[1]));
		i++;
	}
	groupCount = i;
	tvi.hInsertAfter = TVI_LAST;

	mysql_free_result(results);

	//records				       groups.name, SUBSTRING(product.name, 1, 40), product.id, group_id FROM product LEFT JOIN groups ON group_id = groups.id ORDER BY group_id
	if (mysql_query(&conn, "SELECT id, SUBSTRING(name, 1, 40), group_id FROM product WHERE group_id != 0 ORDER BY group_id;") == 0) {
		results = mysql_store_result(&conn);
		i = 0;
		*query = 0;
		strcopy(query, "Обнаружено ");
		strcopy(query, itoc(mysql_num_rows(results)));
		strcopy(query, " устройств");
		//err_mess(0, query);
		*query = 0;
		while (record = mysql_fetch_row(results)) {		//result using
		//for(i=0; i<9;i++){

			MultiByteToWideChar(CP_UTF8, 0, record[1], 120, buff2, 500);
			SendMessageW(groupDevice_combobox_HWND, CB_ADDSTRING, 0, (LPARAM)buff2);
			SendMessageW(groupView_comboDevice_HWND, CB_ADDSTRING, 0, (LPARAM)buff2);
			//deviceMap[i] = ctoi(record[2]);

			lvstr.iSubItem = 1; //lvstr.
			MultiByteToWideChar(CP_UTF8, 0, record[1], 120, buff2, 500);
			lvstr.pszText = (buff2);
			SendMessage(deviceSelect_listView_HWND, LVM_SETITEMW, 0, (LPARAM)(const LV_ITEM *)&lvstr);

			lvstr.iSubItem = 2;
			lvstr.pszText = record[0];
			SendMessage(deviceSelect_listView_HWND, LVM_SETITEM, 0, (LPARAM)(const LV_ITEM *)&lvstr);

			//

			//tree view
			//
			//tvi.hInsertAfter = TVI_FIRST;
			offset = ctoi(record[2]);
			for (x = 0; x < groupCount && groupData[x].id != offset; x++); //try to find  appropriate group
			if (x == groupCount)
				tvi.hParent = htrItem;
			else
				tvi.hParent = groupData[x].ht;

			tvi.item.mask = TVIF_TEXT;
			tvi.item.cchTextMax = 140;
			MultiByteToWideChar(CP_UTF8, 0, record[1], 120, buff2, 500);
			tvi.item.pszText = buff2;
			//save htree?
			itemData[i].ht = SendMessage(deviceSelect_treeView_HWND, TVM_INSERTITEMW, 0, &tvi);
			itemData[i].id = ctoi(record[0]);

			i++;
		}
		itemCount = i;
		SendMessage(deviceSelect_treeView_HWND, TVM_EXPAND, TVE_EXPAND, htrItem);
		SendMessage(deviceSelect_treeView_HWND, TVM_SELECTITEM, TVGN_CARET, htrItem);
	}
	else
		MessageBox(0, "query fail", "Info", MB_ICONINFORMATION);
	/*ShowWindow(sqlConnectWindow_HWND, FALSE);
	EnableWindow(mainWindow_HWND, TRUE);*/

}

void mysql_searchDevice(char *dev, int group) {
	LV_ITEMW lvstr;
	int i;
	*query = 0;
	//err_mess(GetActiveWindow(), dev);
	strcopy(query, "select pr.id, pr.name, (select gr.name from product as gr where gr.id = pr.group_id), ifnull(pr.macCount, 0) from product as pr  where pr.group_id != 0 && pr.name LIKE \"%");

	//WideCharToMultiByte(CP_UTF8, 0, dev, -1, buff2, 120, 0, 0);
	//rr_mess(GetActiveWindow(), dev);
	/*strcopy(query, dev);*/
	strcopy(query, ASCII_to_utf8(dev));
	if (group != 0) {
		strcopy(query, "%\" && group_id = ");
		strcopy(query, itoc(group));
	}
	else
		strcopy(query, "%\" ");
	if (SendMessage(deviceSelect_showOnlyMAC_HWND, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		strcopy(query, " && pr.MACCount > 0 ");
	}
	strcopy(query, " ORDER BY pr.group_id, pr.name;");
	if (mysql_conn())
		return;
	//err_mess(GetActiveWindow(), query);
	if (mysql_query(&conn, query)) {
		err_mess(0, "fail");
		return;
	}
	results = mysql_store_result(&conn);

	i = 0;
	ListView_DeleteAllItems(deviceSelect_listView_HWND);
	while (record = mysql_fetch_row(results)) {
		lvstr.iItem = i;
		lvstr.cchTextMax = 50;
		lvstr.mask = LVIF_TEXT;
		MultiByteToWideChar(CP_UTF8, 0, record[2], 120, buff2, 500);
		lvstr.iSubItem = 0;
		lvstr.pszText = buff2;
		SendMessage(deviceSelect_listView_HWND, LVM_INSERTITEMW, 0, (LPARAM)(LV_ITEM *)&lvstr);

		lvstr.iSubItem = 1;
		MultiByteToWideChar(CP_UTF8, 0, record[1], 120, buff2, 500);
		lvstr.pszText = (buff2);
		SendMessage(deviceSelect_listView_HWND, LVM_SETITEMW, 0, (LPARAM)(const LV_ITEM *)&lvstr);

		lvstr.iSubItem = 2;
		lvstr.pszText = record[0];
		SendMessage(deviceSelect_listView_HWND, LVM_SETITEM, 0, (LPARAM)(const LV_ITEM *)&lvstr);

		lvstr.iSubItem = 4;
		lvstr.pszText = record[3];
		SendMessage(deviceSelect_listView_HWND, LVM_SETITEM, 0, (LPARAM)(const LV_ITEM *)&lvstr);

		i++;
	}
	mysql_free_result(results);
}

int mysql_updateDevice(struct deviceData *dt) {
	char upd_name[] = "update product set name = \"", upd_group[] = "\", group_id = ", upd_macCount[] = ", macCount = ";
	char ins_query[] = "insert into product(name, group_id, macCount) values(\"";
	*query = 0;
	if (dt->id) {
		strcopy(query, upd_name);
		strcopy(query, ASCII_to_utf8(dt->deviceName));
		strcopy(query, upd_group);
		strcopy(query, itoc(dt->group_id));
		strcopy(query, upd_macCount);
		strcopy(query, itoc(dt->MACCount));
		strcopy(query, " where id = ");
		strcopy(query, itoc(dt->id));
	}
	else {	// new device
		strcopy(query, ins_query);
		strcopy(query, ASCII_to_utf8(dt->deviceName));
		strcopy(query, "\", ");
		strcopy(query, itoc(dt->group_id));
		strcopy(query, ", ");
		strcopy(query, itoc(dt->MACCount));
		strcopy(query, ") ");
	}
	//err_mess(0, query);
	if (mysql_conn())
		return;
	if (mysql_query(&conn, query) != 0)
		return 1;
	if (mysql_affected_rows(&conn) != 1)
		return 2;
	return 0;

}

int mysql_deleteDevice(int id) {
	*query = 0;
	strcopy(query, "delete from product where id = ");
	strcopy(query, itoc(id));
	//err_mess(0, query);
	if (mysql_conn())
		return;
	//err_mess(GetActiveWindow(), query);
	if (mysql_query(&conn, query))
		return 0;
	return (mysql_affected_rows(&conn));
}
//struct deviceData *mysql_getDeviceInfo


int mysql_getUsers() {
	int i = 0;
	//char u_query[] = "select name, id, accesslevel, birthday from users;";
	if (mysql_conn())
		return;
	if (mysql_query(&conn, "select name, id, accesslevel, birthday from users;") != 0)
		return 1;
	SendMessage(registrationForm_editLogin_HWND, CB_RESETCONTENT, 0, 0);
	results = mysql_store_result(&conn);
	while (record = mysql_fetch_row(results)) {
		SendMessage(registrationForm_editLogin_HWND, CB_ADDSTRING, 0, utf8_to_ASCII(record[0]));
		SendMessage(registrationForm_editLogin_HWND, CB_SETITEMDATA, i, ctoi(record[1]));
		i++;
	}
	mysql_free_result(results);
	return i;
}

int mysql_checkUser(int id, char *pass) {
	struct sqlUser *curUser;
	if (mysql_conn())
		return;
	*query = 0;
	strcopy(query, "select accesslevel, password, md5(\"");
	strcopy(query, pass);
	strcopy(query, "\") from users where id = ");
	strcopy(query, itoc(id));
	//err_mess(0, query);
	if (mysql_query(&conn, query) != 0)
		return 1;
	results = mysql_store_result(&conn);

	record = mysql_fetch_row(results);
	*query = 0;
	*buff2 = 0;
	strcopy(query, record[1]);
	//err_mess(0, "here");
	strcopy(buff2, record[2]);

	mysql_free_result(results);

	return strcmp(query, buff2);
}
