#include <Windows.h>
#include <CommCtrl.h>

HINSTANCE hinst;
HWND mainWindow_HWND, tabControl_HWND, wrapWindow1_HWND, wrapWindow2_HWND, wrapWindow3_HWND;
void err_mess(HWND hwnd, char *mess);
char *itoc(int value);
int ctoi(char *s);
enum errorList {
	serialError = 1,
	deviceError,
	notSupportMac,
	alreadyUsed,
	notEnoughMAC,
	queryFail,
	unknownError,
	mysqlLibrary,
	mysqlConnect
};


HWND mainWindow_create(void);
//sHWND tabControl_create(HWND hParent);
LRESULT CALLBACK mainWindowPROC(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK wrapWindowPROC(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK groupWND_PROC(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK deviceSelect_PROC(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK registrationForm_PROC(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

#define tabControl_id	100
#define wrapWindow1_id	110
#define wrapWindow2_id	120
#define wrapWindow3_id	122
#define wrapMask		123
#define sqlConnectWindow_id 125

#define groupMAC_remember_id			129
#define groupMAC_id						130
#define groupMAC_serial_id				131
#define groupMAC_getAddress_id			132
#define groupMAC_MACAddress_id			133
#define groupMAC_sMACAddress_id			134
#define groupMAC_sSerial_id				135

#define groupDevice_id					136
#define groupDevice_combobox_id			137
#define groupDevice_btnSelect_id		138

#define groupMACCount_id				139
#define groupMACCount_limited_id		140
#define groupMACCount_unlimited_id		141
#define groupMACCount_count_id			142

#define groupHistory_id					144
#define groupHistory_list_id			145
#define groupHistory_btnChange_id		146
#define groupHistory_btnDelete_id		147
#define groupHistory_btnCopy_id			148
#define groupHistory_selectGroup_id		149

#define deviceSelect_id					150
#define deviceSelect_treeView_id		151
#define groupView_list_id				153
#define groupView_selectKeyWord_id		154
#define groupView_search_id				155
#define groupView_btnSearch_id			156
#define groupView_btnCopy_id			157
#define groupView_btnDelete_id			158
#define groupView_btnKit_id				159
#define groupView_chkIsGroup_id			160
#define groupView_btnSelectAll_id		161
#define groupView_comboDevice_id		162
#define groupView_btnDevice_id			163

#define deviceSelect_btnClose_id		196
#define deviceSelect_showOnlyMAC_id		197
#define deviceSelect_btnDelete_id		198
#define deviceSelect_btnAdd_id			199
#define deviceSelect_listView_id		200
#define deviceSelect_treeView_id		202
#define deviceSelect_search_id			203
#define deviceSelect_currentGroup_id	204
#define deviceSelect_currentDevice_id	205
#define deviceSelect_btnChange_id		206

#define groupStatictic_editTotal_id		207
#define groupStatictic_editFree_id		208
#define groupStatictic_editBusy_id		209
#define groupStatictic_btnMake_id		210
#define groupStatictic_editAddrStart_id	212
#define groupStatictic_editAddrEnd_id	213
#define groupStatistic_list_id			214
#define groupStatistic_editAbout_id		215

#define deviceWindow_editDevice_id		270
#define deviceWindow_editGroup_id		271
#define deviceWindow_editMACCount_id	272
#define deviceWindow_btnConfirm_id		273
#define deviceWindow_btnCancel_id		274
#define deviceWindow_editAbout_id		275

#define registrationForm_editLogin_id		300
#define registrationForm_editPassword_id	301
#define registrationForm_chkRemember_id		302
#define registrationForm_btnOK_id			303
#define registrationForm_btnCancel_id		304

HMENU hmenu;
int mysql_isConnected;
HWND status_bar;
HWND groupMAC_HWND, groupMAC_serial_HWND, groupMAC_getAddress_HWND, groupMAC_MACAddress_HWMD, groupMAC_remember_HWND;
HWND groupDevice_combobox_HWND, groupDevice_HWND, groupDevice_btnSelect_HWND;
HWND groupMACCount_HWND, groupMACCount_limited_HWND, groupMACCount_unlimited_HWND, groupMACCount_count_HWND;
HWND groupHistory_HWND, groupHistory_list_HWND, groupHistory_btnChange_HWND, groupHistory_btnDelete_HWND, groupHistory_btnCopy_HWND, GroupHistory_selectGroup_HWND;
HWND deviceSelect_HWND, deviceSelect_treeView_HWND, deviceSelect_listView_HWND, deviceSelect_search_HWND, deviceSelect_currentGroup_HWND, deviceSelect_currentDevice_HWND,
deviceSelect_btnChange_HWND, deviceSelect_btnDelete_HWND, deviceSelect_btnAdd_HWND, deviceSelect_showOnlyMAC_HWND;
HWND groupView_list_HWND, groupView_selectKeyWord_HWND, groupView_search_HWND, groupView_btnSearch_HWND, groupView_btnCopy_HWND, groupView_btnDelete_HWND, groupView_btnKit_HWND, groupView_chkIsGroup_HWND,
groupView_btnSelectAll_HWND, groupView_comboDevice_HWND, groupView_btnDevice_HWND;
HWND groupStatistic_HWND, groupStatistic_editTotal_HWND, groupStatistic_editFree_HWND, groupStatistic_editBusy_HWND, groupStatictic_btnMake_HWND,
groupStatictic_editAddrStart_HWND, groupStatictic_editAddrEnd_HWND, groupStatistic_progress_HWND, groupStatistic_list_HWND, groupStatistic_editAbout_HWND;
HWND deviceWindow_HWND, deviceWindow_editDevice_HWND, deviceWindow_editGroup_HWND, deviceWindow_editMACCount_HWND, deviceWindow_btnConfirm_HWND, deviceWindow_editAbout_HWND,
deviceWindow_id_HWND,
deviceWindow_btnCancel_HWND;
HWND sqlConnectWindow_HWND, registrationForm_HWND, registrationForm_editLogin_HWND, registrationForm_editPassword_HWND, registrationForm_chkRemember_HWND, registrationForm_btnEnterHWND, registrationForm_btnCancelHWND;
//HWND groupView_list_HWND
HWND toolTip_HWND, dw_toolTip_HWND;

WNDPROC groupMACOld_PROC, groupDeviceOld_PROC, groupMACCountOld_PROC, groupHistoryOld_PROC;
void try_to_connect(void);
HCURSOR cursorArrow;
int baseMAC_setup(unsigned int startAddress, unsigned int endAddress);

int mysql_conn(void);
int mysql_getAddress(void);

void strcopy(char *s1, char *s2);

struct groupLink {
	HTREEITEM ht;
	int id;
};

struct deviceData {
	int id;
	int group_id;
	char deviceName[100];
	char groupName[100];
	int MACCount;
	int MACExist;
};

struct sqlUser {
	char name[30];
	int privilege;
	int id;
};

struct deviceData *mysql_getDeviceInfo(char *device_ID, char *serial);
void showDeviceWindow(int device_ID);
