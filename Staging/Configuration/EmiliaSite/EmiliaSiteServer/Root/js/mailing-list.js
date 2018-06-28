var mListSelected = "mlist-selector-panel-selected";

var html;
var mListSelector;
var mListViewer;

var curMListSelect = null;

function onLoad() {
	html = document.getElementsByTagName('html')[0];
	mListSelector = document.getElementById("mlist-selector-scrollable");
	mListViewer = document.getElementById("mlist-viewer");

	var fragment = window.location.hash.substr(1);
	var fSplit = fragment.split("&", 2);
	if (fSplit.length >= 2)
		fragment = fSplit[1];
	else
		fragment = "";

	setSelector(fragment == "" ? mListSelector.children[0].id : fragment);
	
	for (var a = 0;a < mListSelector.children.length;a++) {
		mListSelector.children[a].addEventListener("click", onSelectorClick, false);
	}

	new ResizeObserver(onMListViewerResize).observe(mListViewer);
	new ResizeObserver(onMListSelectorResize).observe(mListSelector);
}
onLoad();

function fetchText(url) {
    return fetch(url).then((response) => (response.text()));
}

function setSelector(id) {
	//update viewer content
	fetchText("mailing-list/" + id + ".html").then((html) => {
		mListViewer.scrollTop = 0;
		mListViewer.innerHTML = html;
    }).catch((error) => {
        console.warn(error);
    });

	if (curMListSelect != null)
		document.getElementById(curMListSelect).classList.remove(mListSelected);
	document.getElementById(id).classList.add(mListSelected);
	curMListSelect = id;
}

function onSelectorClick() {
	if (this.classList.contains(mListSelected))
		return;

	setSelector(this.id);
}

function onMListViewerResize() {
	var scrollWidth = mListViewer.offsetWidth - mListViewer.clientWidth;

	//necessary to prevent flickering
	scrollWidth = scrollWidth + scrollWidth % 2;

	mListViewer.style.right = "calc(0% - " + scrollWidth + "px)";
	mListViewer.style.paddingRight = "calc(20px + " + scrollWidth + "px)";
}

function onMListSelectorResize() {
	var scrollWidth = mListSelector.offsetWidth - mListSelector.clientWidth;

	//necessary to prevent flickering
	scrollWidth = scrollWidth + scrollWidth % 2;

	mListSelector.style.width = "calc(100% + " + scrollWidth + "px)";
	mListSelector.style.paddingRight = scrollWidth + "px";
}