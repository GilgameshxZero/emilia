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

	onTechSelectorResize();
}
onLoad();

function fetchText(url) {
    return fetch(url).then(function(response) {
		return response.text();
	});
}

function setSelector(id) {
	//update viewer content
	fetchText("tech-blog/" + id + ".html").then(function(html) {
		techViewer.scrollTop = 0;
		techViewer.innerHTML = html;

		onTechSelectorResize();
		onTechViewerResize();
    }).catch(function (error) {
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

	techViewer.style.right = "calc(0% - " + scrollWidth + "px)";
	techViewer.style.paddingRight = "calc(20px + " + scrollWidth + "px)";
}

function onTechSelectorResize() {
	var scrollWidth = techSelector.offsetWidth - techSelector.clientWidth;

	techSelector.style.width = "calc(100% + " + scrollWidth + "px)";
	techSelector.style.paddingRight = scrollWidth + "px";
}