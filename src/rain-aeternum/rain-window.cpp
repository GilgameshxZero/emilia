#include "rain-window.h"

namespace Rain {
	const LPCTSTR RainWindow::NULLCLASSNAME = "";
	int RainWindow::class_id;
	std::unordered_map<HWND, RainWindow *> RainWindow::objmap;

	RainWindow::RainWindow() {
		hwnd = NULL;
		classname = NULL;
	}
	RainWindow::~RainWindow() {
		if (hwnd) {
			int k = DestroyWindow(hwnd);
			if (k == 0) {
				int p = GetLastError();
				k += p;
			}
			hwnd = NULL;
		}
		if (classname) {
			delete[] classname;
			classname = NULL;
		}
	}
	int RainWindow::create(
		std::unordered_map<UINT, MSGFC> *msgm,
		MSGFC		*intfc,
		UINT		style,
		int			cbClsExtra,
		int			cbWndExtra,
		HINSTANCE	hInstance,
		HICON		hIcon,
		HCURSOR		hCursor,
		HBRUSH		hbrBackground,
		LPCTSTR		lpszMenuName,
		HICON		hIconSm,
		DWORD		dwExStyle,
		LPCTSTR		lpWindowName,
		DWORD		dwStyle,
		int			x,
		int			y,
		int			nWidth,
		int			nHeight,
		HWND		hWndParent,
		HMENU		hMenu,
		LPCTSTR		lpszClassName) {
		this->msgm = msgm;
		this->intfc = intfc;

		if (lpszClassName == NULLCLASSNAME) {
			static LPCTSTR prefix = _T("Rain::Mono5::RainWindow ");
			static const size_t prelen = _tcslen(prefix);
			LPTSTR format = new TCHAR[prelen + 3]; //2 for the "%d", and 1 for the "\0"
			int idlen = static_cast<int>(Rain::tToStr(class_id).length()); //length of class_id
			classname = new TCHAR[idlen + prelen + 1]; //1 for the "\0", memory freed later
			_tcscpy_s(format, prelen + 3, prefix);
			_tcscat_s(format, prelen + 3, _T("%d"));
			_stprintf_s(classname, idlen + prelen + 1, format, class_id);
			lpszClassName = classname;
			class_id++;
			delete[] format;
		} else {
			size_t prelen = _tcslen(lpszClassName);
			classname = new TCHAR[prelen + 1];
			_tcscpy_s(classname, prelen + 1, lpszClassName);
		}

		WNDCLASSEX wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = style;
		wcex.lpfnWndProc = rainWindowProc;
		wcex.cbClsExtra = cbClsExtra;
		wcex.cbWndExtra = cbWndExtra;
		wcex.hInstance = hInstance;
		wcex.hIcon = hIcon;
		wcex.hCursor = hCursor;
		wcex.hbrBackground = hbrBackground;
		wcex.lpszMenuName = lpszMenuName;
		wcex.lpszClassName = lpszClassName;
		wcex.hIconSm = hIconSm;

		if (!RegisterClassEx(&wcex)) return GetLastError();

		hwnd = CreateWindowEx(
			dwExStyle,
			lpszClassName,
			lpWindowName,
			dwStyle,
			x, y, nWidth, nHeight,
			hWndParent,
			hMenu,
			hInstance,
			this); //pass pointer to this class, so that we can access message funcs

		if (hwnd == NULL) return GetLastError();

		objmap.insert(std::make_pair(hwnd, this));

		return 0;
	}
	LPCTSTR RainWindow::getWndClassName() {
		return classname;
	}

	RainWindow *RainWindow::getWndObj(HWND hwndid) {
		auto it = objmap.find(hwndid);
		if (it == objmap.end())
			return NULL;
		return it->second;
	}
	WPARAM RainWindow::enterMessageLoop() {
		MSG msg;
		BOOL bRet;

		while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
			if (bRet == -1)
				return -1; //serious error
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		return msg.wParam;
	}

	LRESULT CALLBACK rainWindowProc(
		HWND   hwnd,
		UINT   uMsg,
		WPARAM wParam,
		LPARAM lParam) {
		UNALIGNED RainWindow *wndobj;

		if (uMsg == WM_CREATE || uMsg == WM_NCCREATE)
			wndobj = reinterpret_cast<UNALIGNED RainWindow *>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams);
		else
			wndobj = RainWindow::getWndObj(hwnd);

		if (wndobj == NULL)
			return DefWindowProc(hwnd, uMsg, wParam, lParam);

		if (wndobj->intfc != NULL) {
			LRESULT rt = (*(wndobj->intfc)) (hwnd, uMsg, wParam, lParam);
			if (rt != 0)
				return rt;
		}

		auto it = wndobj->msgm->find(uMsg);
		if (it != wndobj->msgm->end())
			return it->second(hwnd, uMsg, wParam, lParam);
		else
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}