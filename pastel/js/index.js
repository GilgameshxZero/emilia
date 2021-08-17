let navSelected = "nav-selected";
let defaultNav = "home";
let divider = "divider";

let navBar;
let mainDiv;

let curNavSelect = null;

window.onload = function () {
	navBar = document.getElementById("navigator");
	mainDiv = document.getElementById("main");

	var fragment = window.location.hash.substr(1);
	fragment = fragment.split("&", 2)[0];

	setNavBar(fragment == "" ? defaultNav : fragment);

	for (var a = 0; a < navBar.children.length; a++) {
		navBar.children[a].addEventListener("click", onNavClick, false);
	}
}

function fetchText(url) {
	return fetch(url).then(function (response) {
		return response.text();
	});
}

function loadJS(url, implementationCode, location) {
	let elems = document.getElementsByTagName("body")[0].getElementsByTagName("script");
	for (let a = 0; a < elems.length; a++) {
		elems[a].remove();
	}
	let scriptTag = document.createElement("script");
	document.getElementsByTagName("body")[0].appendChild(scriptTag);
	scriptTag.src = url;

	scriptTag.onload = implementationCode;
	scriptTag.onreadystatechange = implementationCode;

	location.appendChild(scriptTag);
}

function isWebkit() {
	let isChrome = /Chrome/.test(navigator.userAgent) && /Google Inc/.test(navigator.vendor);
	let isSafari = /Safari/.test(navigator.userAgent) && /Apple Computer/.test(navigator.vendor);

	return isChrome || isSafari;
}

function setNavBar(id) {
	//update main content
	mainDiv.style.transitionDuration = "0s";
	mainDiv.style.opacity = 0;
	fetchText(id + ".html").then(function (html) {
		mainDiv.style.transitionDuration = "0.5s";
		mainDiv.innerHTML = html;
	
		loadJS("js/" + id + ".js", function () {
			//do nothing
		}, document.body);
	
		//wait for js to set display as block
	}).catch(function (error) {
		console.warn(error);
	});

	//change navbar style
	if (curNavSelect != null)
		document.getElementById(curNavSelect).classList.remove(navSelected);
	document.getElementById(id).classList.add(navSelected);
	document.getElementById(divider).className = document.getElementById(id).className;
	curNavSelect = id;

	let fragment = window.location.hash.substr(1);
	let fSplit = fragment.split("&", 2);
	if (fSplit.length >= 2)
		parent.location.hash = id + "&" + fSplit[1];
	else
		parent.location.hash = id;
}

function onNavClick(event) {
	if (this.classList.contains(navSelected))
		return;

	setNavBar(this.id);
}