var techSelected = "tech-selector-panel-selected";

var html;
var techSelector;
var techViewer;

var curTechSelect = null;

function onLoad() {
	html = document.getElementsByTagName('html')[0];
	techSelector = document.getElementById("tech-selector-scrollable");
	techViewer = document.getElementById("tech-viewer");

	var fragment = window.location.hash.substr(1);
	var fSplit = fragment.split("&", 2);
	if (fSplit.length >= 2)
		fragment = fSplit[1];
	else
		fragment = "";

	setSelector(fragment == "" ? techSelector.children[0].id : fragment);
	
	for (var a = 0;a < techSelector.children.length;a++) {
		techSelector.children[a].addEventListener("click", onSelectorClick, false);
	}

	new ResizeObserver(onTechViewerResize).observe(techViewer);
	new ResizeObserver(onTechSelectorResize).observe(techSelector);
}
onLoad();

function fetchText(url) {
    return fetch(url).then((response) => (response.text()));
}

function setSelector(id) {
	//update viewer content
	fetchText("tech-blog/" + id + ".html").then((html) => {
		techViewer.scrollTop = 0;
		techViewer.innerHTML = html;
    }).catch((error) => {
        console.warn(error);
    });

	if (curTechSelect != null)
		document.getElementById(curTechSelect).classList.remove(techSelected);
	document.getElementById(id).classList.add(techSelected);
	curTechSelect = id;
}

function onSelectorClick() {
	if (this.classList.contains(techSelected))
		return;

	setSelector(this.id);
}

function onTechViewerResize() {
	var scrollWidth = techViewer.offsetWidth - techViewer.clientWidth;

	//necessary to prevent flickering
	scrollWidth = scrollWidth + scrollWidth % 2;

	techViewer.style.right = "calc(0% - " + scrollWidth + "px)";
	techViewer.style.paddingRight = "calc(20px + " + scrollWidth + "px)";
}

function onTechSelectorResize() {
	var scrollWidth = techSelector.offsetWidth - techSelector.clientWidth;

	//necessary to prevent flickering
	scrollWidth = scrollWidth + scrollWidth % 2;

	techSelector.style.width = "calc(100% + " + scrollWidth + "px)";
	techSelector.style.paddingRight = scrollWidth + "px";
}